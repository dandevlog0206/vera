#include "open_type.h"

#include "../../include/vera/core/exception.h"
#include "../../include/vera/util/static_vector.h"
#include "../parse.h"
#include <algorithm>

#define MAX_CONTOUR_COUNT 128

#define CHECK(expression)                          \
	do {                                               \
		OTFResult result = expression;                 \
		if (result.result() != OTFResultType::Success) \
			return result;                             \
	} while (0)

VERA_NAMESPACE_BEGIN

typedef float (*OTFVectorParser)(const uint8_t*, uint32_t&);

static bool has_flag(OTFGlyphFlagBits a, OTFGlyphFlagBits b)
{
	return static_cast<bool>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

static bool has_flag(OTFComponentGlyphFlagBits a, OTFComponentGlyphFlagBits b)
{
	return static_cast<bool>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

static OTFEncodingID parse_encoding_id(const uint8_t* data, uint32_t& offset, OTFPlatformID platform_id)
{
	uint16_t encoding_id     = parse_u16_be(data, offset);
	uint16_t platform_offset = static_cast<uint16_t>(OTFEncodingID::__Platform_Offset__);
	uint16_t encoding_offset = platform_offset * static_cast<uint16_t>(platform_id);

	return static_cast<OTFEncodingID>(encoding_id + encoding_offset);
}

static OTFLanguageID parse_language_id(const uint8_t* data, uint32_t& offset, OTFPlatformID platform_id)
{
	uint16_t language_id     = parse_u16_be(data, offset);
	uint16_t platform_offset = static_cast<uint16_t>(OTFLanguageID::__Platform_Offset__);
	uint16_t language_offset = platform_offset * static_cast<uint16_t>(platform_id);
	return static_cast<OTFLanguageID>(language_id + language_offset);
}

static void decode_utf16be_to_utf8(std::string& str_buf)
{
	size_t new_buf_size = str_buf.size() / 2;

	for (size_t i = 0; i < new_buf_size; i++)
		str_buf[i] = str_buf[2 * i + 1];

	str_buf.resize(new_buf_size);
}

static float fixed_to_float(OTFFixed value)
{
	int16_t  integer  = static_cast<int16_t>((value >> 16) & 0xFFFF);
	uint16_t fraction = static_cast<uint16_t>(value & 0xFFFF);
	return static_cast<float>(integer) + static_cast<float>(fraction) / 65536.0f;
}

static OTFFixed float_to_fixed(float value)
{
	int16_t  integer  = static_cast<int16_t>(value);
	uint16_t fraction = static_cast<uint16_t>((value - static_cast<float>(integer)) * 65536.0f);
	return (static_cast<OTFFixed>(integer) << 16) | static_cast<OTFFixed>(fraction);
}

static float f2dot14_to_float(OTFF2DOT14 value)
{
	int16_t  integer  = static_cast<int16_t>((value >> 14) & 0x3);
	uint16_t fraction = static_cast<uint16_t>(value & 0x3FFF);
	return static_cast<float>(integer) + static_cast<float>(fraction) / 16384.0f;
}

static OTFF2DOT14 float_to_f2dot14(float value)
{
	int16_t  integer  = static_cast<int16_t>(value);
	uint16_t fraction = static_cast<uint16_t>((value - static_cast<float>(integer)) * 16384.0f);
	return (static_cast<OTFF2DOT14>(integer) << 14) | static_cast<OTFF2DOT14>(fraction);
}

static uint16_t get_version_major(OTFVersion version)
{
	return static_cast<uint16_t>((version >> 8) & 0xFF);
}

static uint16_t get_version_minor(OTFVersion version)
{
	return static_cast<uint16_t>(version & 0xFF);
}

static OTFVersion make_version(uint16_t major, uint16_t minor)
{
	return static_cast<OTFVersion>(major) << 8 |
		   static_cast<OTFVersion>(minor);
}

static uint32_t parse_ttc_header(TTCHeader& header, const uint8_t* data, uint32_t offset)
{
	header.magic        = parse_enum_be<OTFTTCTag>(data, offset);
	header.majorVersion = parse_u16_be(data, offset);
	header.minorVersion = parse_u16_be(data, offset);
	header.numFonts     = parse_u32_be(data, offset);
	return offset;
}

static uint32_t parse_ttf_header(TTFHeader& header, const uint8_t* data, uint32_t offset)
{
	header.sfntVersion   = parse_enum_be<OTFSFNTVersion>(data, offset);
	header.numTables     = parse_u16_be(data, offset);
	header.searchRange   = parse_u16_be(data, offset);
	header.entrySelector = parse_u16_be(data, offset);
	header.rangeShift    = parse_u16_be(data, offset);
	return offset;
}

static uint32_t parse_table_record(OTFTableRecord& record, const uint8_t* data, uint32_t offset)
{
	record.tableTag = parse_enum_be<OTFTableTag>(data, offset);
	record.checkSum = parse_u32_be(data, offset);
	record.offset   = parse_u32_be(data, offset);
	record.length   = parse_u32_be(data, offset);
	return offset;
}

static uint32_t parse_cmap_table_header(OTFCMAPTableHeader& header, const uint8_t* data, uint32_t offset)
{
	header.version      = parse_u16_be(data, offset);
	header.numSubtables = parse_u16_be(data, offset);
	return offset;
}

static uint32_t parse_encoding_record(OTFEncodingRecord& record, const uint8_t* data, uint32_t offset)
{
	record.platformID     = parse_enum_be<OTFPlatformID>(data, offset);
	record.encodingID     = parse_encoding_id(data, offset, record.platformID);
	record.subtableOffset = parse_u32_be(data, offset);
	return offset;
}

static uint32_t parse_sequential_map_group(OTFSequentialMapGroup& group, const uint8_t* data, uint32_t offset)
{
	group.startCharCode = parse_u32_be(data, offset);
	group.endCharCode   = parse_u32_be(data, offset);
	group.startGlyphID  = parse_u32_be(data, offset);
	return offset;
}

static uint32_t parse_long_hor_metric(OTFLongHorMetric& metric, const uint8_t* data, uint32_t offset)
{
	metric.advanceWidth    = parse_u16_be(data, offset);
	metric.leftSideBearing = parse_i16_be(data, offset);
	return offset;
}

static uint32_t parse_name_table_header(OTFNAMETableHeader& header, const uint8_t* data, uint32_t offset)
{
	header.version       = parse_u16_be(data, offset);
	header.count         = parse_u16_be(data, offset);
	header.storageOffset = parse_u16_be(data, offset);
	return offset;
}

static uint32_t parse_name_record(OTFNameRecord& record, const uint8_t* data, uint32_t offset)
{
	record.platformID   = parse_enum_be<OTFPlatformID>(data, offset);
	record.encodingID   = parse_encoding_id(data, offset, record.platformID);
	record.languageID   = parse_language_id(data, offset, record.platformID);
	record.nameID       = parse_enum_be<OTFNameID>(data, offset);
	record.length       = parse_u16_be(data, offset);
	record.stringOffset = parse_u16_be(data, offset);
	return offset;
}

static uint32_t parse_post_table_header(OTFPOSTTableHeader& header, const uint8_t* data, uint32_t offset)
{
	header.version            = parse_u32_be(data, offset);
	header.italicAngle        = parse_u32_be(data, offset);
	header.underlinePosition  = parse_i16_be(data, offset);
	header.underlineThickness = parse_i16_be(data, offset);
	header.isFixedPitch       = parse_u32_be(data, offset);
	header.minMemType42       = parse_u32_be(data, offset);
	header.maxMemType42       = parse_u32_be(data, offset);
	header.minMemType1        = parse_u32_be(data, offset);
	header.maxMemType1        = parse_u32_be(data, offset);
	return offset;
}

static uint32_t parse_glyph_header(OTFGlyphHeader& header, const uint8_t* data, uint32_t offset)
{
	header.numberOfContours = parse_i16_be(data, offset);
	header.xMin             = parse_i16_be(data, offset);
	header.yMin             = parse_i16_be(data, offset);
	header.xMax             = parse_i16_be(data, offset);
	header.yMax             = parse_i16_be(data, offset);
	return offset;
}

static bool supported_cmap_encoding_format4(OTFEncodingID encoding_id) {
	switch (encoding_id) {
	case OTFEncodingID::Unicode_1_0_Semantics:
	case OTFEncodingID::Unicode_1_1_Semantics:
	case OTFEncodingID::Unicode_ISO_10646:
	case OTFEncodingID::Unicode_BMP:
	case OTFEncodingID::Unicode_Full_Repertoire:
		return true;
	case OTFEncodingID::Windows_Unicode_BMP:
		return true;
	default:
		return false;
	}
}

static bool compare_name_entry(const OTFNameEntry& a, const OTFNameEntry& b) {
	if (a.platformID != b.platformID)
		return a.platformID < b.platformID;
	if (a.encodingID != b.encodingID)
		return a.encodingID < b.encodingID;
	return a.languageID < b.languageID;
}

static const char* get_macintosh_glyph_names(OTFUint16 glyph_name_idx)
{
	static const char* macintosh_glyph_names[258] = {
		".notdef",
		".null",
		"nonmarkingreturn",
		"space",
		"exclam",
		"quotedbl",
		"numbersign",
		"dollar",
		"percent",
		"ampersand",
		"quotesingle",
		"parenleft",
		"parenright",
		"asterisk",
		"plus",
		"comma",
		"hyphen",
		"period",
		"slash",
		"zero",
		"one",
		"two",
		"three",
		"four",
		"five",
		"six",
		"seven",
		"eight",
		"nine",
		"colon",
		"semicolon",
		"less",
		"equal",
		"greater",
		"question",
		"at",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"bracketleft",
		"backslash",
		"bracketright",
		"asciicircum",
		"underscore",
		"grave",
		"a",
		"b",
		"c",
		"d",
		"e",
		"f",
		"g",
		"h",
		"i",
		"j",
		"k",
		"l",
		"m",
		"n",
		"o",
		"p",
		"q",
		"r",
		"s",
		"t",
		"u",
		"v",
		"w",
		"x",
		"y",
		"z",
		"braceleft",
		"bar",
		"braceright",
		"asciitilde",
		"Adieresis",
		"Aring",
		"Ccedilla",
		"Eacute",
		"Ntilde",
		"Odieresis",
		"Udieresis",
		"aacute",
		"agrave",
		"acircumflex",
		"adieresis",
		"atilde",
		"aring",
		"ccedilla",
		"eacute",
		"egrave",
		"ecircumflex",
		"edieresis",
		"iacute",
		"igrave",
		"icircumflex",
		"idieresis",
		"ntilde",
		"oacute",
		"ograve",
		"ocircumflex",
		"odieresis",
		"otilde",
		"uacute",
		"ugrave",
		"ucircumflex",
		"udieresis",
		"dagger",
		"degree",
		"cent",
		"sterling",
		"section",
		"bullet",
		"paragraph",
		"germandbls",
		"registered",
		"copyright",
		"trademark",
		"acute",
		"dieresis",
		"notequal",
		"AE",
		"Oslash",
		"infinity",
		"plusminus",
		"lessequal",
		"greaterequal",
		"yen",
		"mu",
		"partialdiff",
		"summation",
		"product",
		"pi",
		"integral",
		"ordfeminine",
		"ordmasculine",
		"Omega",
		"ae",
		"oslash",
		"questiondown",
		"exclamdown",
		"logicalnot",
		"radical",
		"florin",
		"approxequal",
		"Delta",
		"guillemotleft",
		"guillemotright",
		"ellipsis",
		"nonbreakingspace",
		"Agrave",
		"Atilde",
		"Otilde",
		"OE",
		"oe",
		"endash",
		"emdash",
		"quotedblleft",
		"quotedblright",
		"quoteleft",
		"quoteright",
		"divide",
		"lozenge",
		"ydieresis",
		"Ydieresis",
		"fraction",
		"currency",
		"guilsinglleft",
		"guilsinglright",
		"fi",
		"fl",
		"daggerdbl",
		"periodcentered",
		"quotesinglbase",
		"quotedblbase",
		"perthousand",
		"Acircumflex",
		"Ecircumflex",
		"Aacute",
		"Edieresis",
		"Egrave",
		"Iacute",
		"Icircumflex",
		"Idieresis",
		"Igrave",
		"Oacute",
		"Ocircumflex",
		"apple",
		"Ograve",
		"Uacute",
		"Ucircumflex",
		"Ugrave",
		"dotlessi",
		"circumflex",
		"tilde"
		"macron",
		"breve",
		"dotaccent",
		"ring",
		"cedilla",
		"hungarumlaut",
		"ogonek",
		"caron",
		"Lslash",
		"lslash",
		"Scaron",
		"scaron",
		"Zcaron",
		"zcaron",
		"brokenbar",
		"Eth",
		"eth",
		"Yacute",
		"yacute",
		"Thorn",
		"thorn",
		"minus",
		"multiply",
		"onesuperior",
		"twosuperior",
		"threesuperior",
		"onehalf",
		"onequarter",
		"threequarters",
		"franc",
		"Gbreve",
		"gbreve",
		"Idotaccent",
		"Scedilla",
		"scedilla",
		"Cacute",
		"cacute",
		"Ccaron",
		"ccaron",
		"dcroat"
	};

	return macintosh_glyph_names[glyph_name_idx];
}

std::vector<uint32_t> OpenTypeImpl::getTTCFontOffsets(const uint8_t* data, const size_t size)
{
	if (size < sizeof(TTCHeader))
		throw Exception("data size too small for TTC header");

	TTCHeader header;
	uint32_t  offset = 0;
	
	offset = parse_ttc_header(header, data, 0);

	if (header.magic != OTFTTCTag::Value)
		throw Exception("invalid TTC header magic");
	if (size < sizeof(TTCHeader) + header.numFonts * sizeof(OTFUint32))
		throw Exception("data size too small for TTC font offsets");

	std::vector<uint32_t> result(header.numFonts);

	for (size_t i = 0; i < header.numFonts; ++i)
		result[i] = parse_u32_be(data, offset);

	return result;
}

OpenTypeImpl::OpenTypeImpl(const uint8_t* data, size_t size, uint32_t offset)
{
	const uint8_t* ttc_data = data;

	data += offset;
	size -= offset;

	if (size < sizeof(TTFHeader))
		throw Exception("data size too small for TTF header");

	TTFHeader      header;
	OTFTableRecord record;

	offset = parse_ttf_header(header, data, 0);

	if (size < sizeof(TTFHeader) + header.numTables * sizeof(OTFTableRecord))
		throw Exception("data size too small for TTF table records");

	for (uint16_t i = 0; i < header.numTables; ++i) {
		offset = parse_table_record(record, data, offset);
		tableMap[record.tableTag] = record;
	}

	if (OTFResult result = parseTable(ttc_data, size); result != OTFResultType::Success) {
		postScript.glyphNames.clear();
		tableMap.clear();
		charToGlyphMap.clear();
		glyphs.clear();
		nameEntries.clear();
		languageTags.clear();
		glyphOffsets.clear();
		glyphBinary.clear();
		controlValues.clear();
		programData.clear();

		throw Exception("failed to parse OpenType tables: " + std::string(result.what()));
	}

	unitsPerEM = static_cast<float>(this->header.unitsPerEm);
}

OpenTypeImpl::~OpenTypeImpl()
{
	// nothing to do
}

std::string_view OpenTypeImpl::getName() const VERA_NOEXCEPT
{
	for (const auto& entry : nameEntries)
#ifdef _WIN32
		if (entry.platformID == OTFPlatformID::Windows)
			return entry.names[static_cast<size_t>(OTFNameID::FullFontName)];
#else
	VERA_ASSERT_MSG(false, "not implemented");
#endif // _WIN32

	return {};
}

void OpenTypeImpl::loadAllGlyphs()
{
	for (GlyphID glyph_id = 0; glyph_id < maxProfile.numGlyphs; ++glyph_id)
		if (OTFResult result = loadGlyph(glyph_id); result != OTFResultType::Success)
			throw Exception("failed to load glyph ID {}: {}", glyph_id, result.what());
}

void OpenTypeImpl::loadGlyphRange(const basic_range<GlyphID>& range)
{
	if (maxProfile.numGlyphs < range.last())
		throw Exception("glyph ID range out of bounds");

	for (GlyphID glyph_id : range)
		if (OTFResult result = loadGlyph(glyph_id); result != OTFResultType::Success)
			throw Exception("failed to load glyph ID {}: {}", glyph_id, result.what());
}

void OpenTypeImpl::loadCodeRange(const CodeRange& range)
{
	if (range == CodeRange(UnicodeRange::ALL))

	if (range.end() > 0x10FFFF)
		throw Exception("codepoint range out of bounds");

	for (char32_t codepoint : range) {
		auto it = charToGlyphMap.find(codepoint);

		if (it == charToGlyphMap.cend())
			throw Exception("codepoint not found in character map");
		
		if (OTFResult result = loadGlyph(it->second); result != OTFResultType::Success)
			throw Exception("failed to load glyph for codepoint U+{:04X}: {}",
				static_cast<uint32_t>(codepoint), result.what());
	}
}

uint32_t OpenTypeImpl::getGlyphCount() const VERA_NOEXCEPT
{
	return maxProfile.numGlyphs;
}

GlyphID OpenTypeImpl::getGlyphID(char32_t codepoint) const
{
	auto it = charToGlyphMap.find(codepoint);
	
	if (it == charToGlyphMap.cend())
		throw Exception("codepoint not found in character map");
	
	return it->second;
}

const Glyph& OpenTypeImpl::findGlyph(GlyphID glyph_id) const
{
	if (glyph_id >= maxProfile.numGlyphs)
		throw Exception("glyph ID out of bounds");

	auto it = glyphs.find(glyph_id);

	if (it == glyphs.cend())
		throw Exception("glyph not loaded");

	return it->second;
}

const Glyph& OpenTypeImpl::getGlyph(GlyphID glyph_id)
{
	if (glyph_id >= maxProfile.numGlyphs)
		throw Exception("glyph ID out of bounds");

	auto it = glyphs.find(glyph_id);

	if (it == glyphs.cend()) {
		auto result = loadGlyph(glyph_id);

		if (result != OTFResultType::Success)
			throw Exception("failed to load glyph: " + std::string(result.what()));
		
		return glyphs.find(glyph_id)->second;
	}

	return it->second;
}

const Glyph& OpenTypeImpl::findGlyphByCodepoint(char32_t codepoint) const
{
	return findGlyph(getGlyphID(codepoint));
}

const Glyph& OpenTypeImpl::getGlyphByCodepoint(char32_t codepoint)
{
	return getGlyph(getGlyphID(codepoint));
}

OTFResult OpenTypeImpl::loadGlyph(uint32_t glyph_id)
{
	auto glyph_it = glyphs.find(glyph_id);
	if (glyph_it != glyphs.cend())
		return OTFResultType::Success;

	glyph_it = glyphs.emplace(glyph_id, Glyph{}).first;

	OTFGlyphHeader glyph_header;
	uint32_t       glyph_offset = glyphOffsets[glyph_id];

	glyph_offset = parse_glyph_header(glyph_header, glyphBinary.data(), glyph_offset);

	if (glyph_header.numberOfContours >= 0) {
		CHECK(parseSimpleGlyph(
			glyph_it->second,
			glyph_header,
			glyph_offset));
	} else {
		CHECK(parseCompositeGlyph(
			glyph_it->second,
			glyph_header,
			glyph_offset));
	}

	glyph_it->second.glyphID = glyph_id;
	glyph_it->second.aabb    = AABB2D{
		static_cast<float>(glyph_header.xMin),
		static_cast<float>(glyph_header.yMin),
		static_cast<float>(glyph_header.xMax),
		static_cast<float>(glyph_header.yMax)
	};

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseSimpleGlyph(Glyph& glyph, const OTFGlyphHeader& glyph_header, uint32_t offset)
{
	static_vector<OTFUint16, MAX_CONTOUR_COUNT> end_indicies;

	uint32_t       num_contours = glyph_header.numberOfContours;
	const uint8_t* data         = glyphBinary.data();

	glyph.contours.resize(num_contours);
	end_indicies.resize(num_contours);
	parse_array_be(end_indicies.data(), num_contours, data, offset);

	OTFUint16 inst_length = parse_u16_be(data, offset);

	glyph.instructions = array_view<uint8_t>(data + offset, inst_length);
	offset            += inst_length;

	uint32_t point_count    = static_cast<uint32_t>(end_indicies.back() + 1);
	uint32_t x_point_offset = offset;
	uint32_t y_point_offset = 0;
	uint32_t x_point_size   = 0;

	while (0 < point_count) {
		OTFGlyphFlagBits flags = parse_enum_be<OTFGlyphFlagBits>(data, x_point_offset);

		if (has_flag(flags, OTFGlyphFlagBits::RepeatFlag)) {
			uint32_t repeat_count = static_cast<uint32_t>(parse_u8_be(data, x_point_offset) + 1);

			if (point_count < repeat_count)
				return { OTFResultType::InvalidFormat, "glyph data is corrupted" };

			point_count -= repeat_count;

			if (has_flag(flags, OTFGlyphFlagBits::XShortVector))
				x_point_size += repeat_count;
			else if (!has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive))
				x_point_size += repeat_count * sizeof(OTFInt16);
		} else {
			point_count -= 1;

			if (has_flag(flags, OTFGlyphFlagBits::XShortVector))
				x_point_size += 1;
			else if (!has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive))
				x_point_size += sizeof(OTFInt16);
		}
	}

	y_point_offset = x_point_offset + x_point_size;
	point_count    = static_cast<uint32_t>(end_indicies.back() + 1);

	OTFGlyphFlagBits flags;
	float2           curr_point;
	float2           prev_point;
	float            delta_x;
	float            delta_y;

	auto     curr_path    = glyph.contours.begin();
	auto     curr_end     = end_indicies.begin();
	uint32_t point_comp   = 0;
	uint32_t repeat_count = 0;
	float    curr_x       = 0.f;
	float    curr_y       = 0.f;
	bool     on_curve     = false;

	for (uint32_t i = 0; i < point_count; ++i) {
		if (repeat_count == 0) {
			flags = parse_enum_be<OTFGlyphFlagBits>(data, offset);

			if (has_flag(flags, OTFGlyphFlagBits::RepeatFlag))
				repeat_count = parse_u8_be(data, offset) + 1;
			else
				repeat_count = 1;

			on_curve = has_flag(flags, OTFGlyphFlagBits::OnCurvePoint);
		}

		if (has_flag(flags, OTFGlyphFlagBits::XShortVector)) {
			delta_x = static_cast<float>(static_cast<int32_t>(parse_u16_be(data, x_point_offset)));
			delta_x = has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive) ? delta_x : -delta_x;
		} else {
			if (has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive))
				delta_x = 0.f;
			else
				delta_x = static_cast<float>(static_cast<int32_t>(parse_u32_be(data, x_point_offset)));
		}

		if (has_flag(flags, OTFGlyphFlagBits::YShortVector)) {
			float delta_y = static_cast<float>(static_cast<int32_t>(parse_u16_be(data, y_point_offset)));
			delta_y = has_flag(flags, OTFGlyphFlagBits::YIsSameOrPositive) ? delta_y : -delta_y;
		} else {
			if (has_flag(flags, OTFGlyphFlagBits::YIsSameOrPositive))
				delta_y = 0.f;
			else
				delta_y = static_cast<float>(static_cast<int32_t>(parse_u32_be(data, y_point_offset)));
		}

		if (delta_x != 0.f || delta_y != 0.f) {
			curr_x += delta_x;
			curr_y += delta_y;
			curr_path->emplace_back(float2(curr_x, curr_y), on_curve);
		}

		if (i == *curr_end) {
			point_comp = 0;
			++curr_path;
			++curr_end;
		}

		--repeat_count;
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseCompositeGlyph(Glyph& glyph, const OTFGlyphHeader& glyph_header, uint32_t offset)
{
	OTFComponentGlyphFlagBits flags;
	OTFGlyphID                glyph_id;

	float transform[6] = { 1.f, 0.f, 0.f, 1.f, 0.f, 0.f };
	float arg1;
	float arg2;

	const uint8_t* data = glyphBinary.data();

	do {
		flags    = parse_enum_be<OTFComponentGlyphFlagBits>(data, offset);
		glyph_id = parse_u16_be(data, offset);

		if (has_flag(flags, OTFComponentGlyphFlagBits::Arg1And2AreWords)) {
			if (has_flag(flags, OTFComponentGlyphFlagBits::ArgsAreXYValues)) {
				arg1 = static_cast<float>(parse_i16_be(data, offset));
				arg2 = static_cast<float>(parse_i16_be(data, offset));
			} else {
				arg1 = static_cast<float>(parse_u16_be(data, offset));
				arg2 = static_cast<float>(parse_u16_be(data, offset));
			}
		} else {
			if (has_flag(flags, OTFComponentGlyphFlagBits::ArgsAreXYValues)) {
				arg1 = static_cast<float>(parse_i8_be(data, offset));
				arg2 = static_cast<float>(parse_i8_be(data, offset));
			} else {
				arg1 = static_cast<float>(parse_u8_be(data, offset));
				arg2 = static_cast<float>(parse_u8_be(data, offset));
			}
		}

		if (has_flag(flags, OTFComponentGlyphFlagBits::WeHaveAScale)) {
			float scale = f2dot14_to_float(parse_i16_be(data, offset));

			transform[0] = scale;
			transform[1] = 0.f;
			transform[2] = 0.f;
			transform[3] = scale;
		} else if (has_flag(flags, OTFComponentGlyphFlagBits::WeHaveAnXAndYScale)) {
			float x_scale = f2dot14_to_float(parse_i16_be(data, offset));
			float y_scale = f2dot14_to_float(parse_i16_be(data, offset));

			transform[0] = x_scale;
			transform[1] = 0.f;
			transform[2] = 0.f;
			transform[3] = y_scale;
		} else if (has_flag(flags, OTFComponentGlyphFlagBits::WeHaveATwoByTwo)) {
			transform[0] = f2dot14_to_float(parse_i16_be(data, offset));
			transform[1] = f2dot14_to_float(parse_i16_be(data, offset));
			transform[2] = f2dot14_to_float(parse_i16_be(data, offset));
			transform[3] = f2dot14_to_float(parse_i16_be(data, offset));
		}

		if (has_flag(flags, OTFComponentGlyphFlagBits::ArgsAreXYValues)) {
			transform[4] = arg1;
			transform[5] = arg2;

			if (has_flag(flags, OTFComponentGlyphFlagBits::ScaledComponentOffset)) {
				transform[4] *= transform[0];
				transform[5] *= transform[3];
			}
		}

		if (has_flag(flags, OTFComponentGlyphFlagBits::WeHaveInstructions)) {
			OTFUint16 inst_length = parse_u16_be(data, offset);

			glyph.instructions = array_view<uint8_t>(data + offset, inst_length);
			offset            += inst_length;
		}

		auto it = glyphs.find(glyph_id);

		if (it == glyphs.end()) {
			CHECK(loadGlyph(glyph_id));
			it = glyphs.find(glyph_id);
		}

		const Glyph& composite_glyph = it->second;

		if (composite_glyph.contours.empty())
			return { OTFResultType::InvalidFormat, "invalid composite glyph component format" };

		// TODO: optimize transform

		glyph.contours.reserve(glyph.contours.size() + composite_glyph.contours.size());

		for (const auto& contour : composite_glyph.contours) {
			auto& curr_path = glyph.contours.emplace_back();

			curr_path.reserve(contour.size());

			for (const auto& p : contour) {
				float x = transform[0] * p.position.x + transform[1] * p.position.y + transform[4];
				float y = transform[2] * p.position.x + transform[3] * p.position.y + transform[5];

				curr_path.emplace_back(float2{ x, y }, p.onCurve);
			}
		}
	} while (has_flag(flags, OTFComponentGlyphFlagBits::MoreComponents));

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseTable(const uint8_t* data, const size_t size)
{
	if (auto it = tableMap.find(OTFTableTag::HEAD); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'head' table" };

		CHECK(parseHeadTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'head' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::MAXP); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'maxp' table" };

		CHECK(parseMaxpTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'maxp' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::CMAP); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'cmap' table" };

		CHECK(parseCmapTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'cmap' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::HHEA); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'hhea' table" };

		CHECK(parseHheaTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'hhea' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::HMTX); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'hmtx' table" };

		CHECK(parseHmtxTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'hmtx' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::NAME); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'name' table" };

		CHECK(parseNameTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'name' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::OS2); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'os/2' table" };

		CHECK(parseOs2Table(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'os/2' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::POST); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'post' table" };

		CHECK(parsePostTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'post' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::CVT); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'cvt' table" };
		if (it->second.length % sizeof(OTFUint16) != 0)
			return { OTFResultType::InvalidSize, "invalid 'cvt' table size" };

		CHECK(parseCvtTable(data + it->second.offset, it->second.length));
	}

	if (auto it = tableMap.find(OTFTableTag::FPGM); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'fpgm' table" };

		CHECK(parseFpgmTable(data + it->second.offset, it->second.length));
	}

	if (auto it = tableMap.find(OTFTableTag::LOCA); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'loca' table" };

		CHECK(parseLocaTable(data + it->second.offset, it->second.length));
	} else {
		return { OTFResultType::MissingTable, "missing 'loca' table" };
	}

	if (auto it = tableMap.find(OTFTableTag::GLYF); it != tableMap.cend()) {
		if (size < it->second.offset + it->second.length)
			return { OTFResultType::InvalidSize, "data size too small for 'glyf' table" };

		uint32_t offset = it->second.offset;
		uint32_t length = it->second.length;

		glyphBinary.assign(data + offset, data + offset + length);
	} else {
		return { OTFResultType::MissingTable, "missing 'glyf' table" };
	}
}

OTFResult OpenTypeImpl::parseHeadTable(const uint8_t* data, const size_t size)
{
	uint32_t offset = 0;

	header.version            = parse_u32_be(data, offset);
	header.fontRevision       = parse_u32_be(data, offset);
	header.checkSumAdjustment = parse_u32_be(data, offset);
	header.magicNumber        = parse_u32_be(data, offset);
	header.flags              = parse_u16_be(data, offset);
	header.unitsPerEm         = parse_u16_be(data, offset);
	header.created            = parse_u64_be(data, offset);
	header.modified           = parse_u64_be(data, offset);
	header.xMin               = parse_i16_be(data, offset);
	header.yMin               = parse_i16_be(data, offset);
	header.xMax               = parse_i16_be(data, offset);
	header.yMax               = parse_i16_be(data, offset);
	header.macStyle           = parse_u16_be(data, offset);
	header.lowestRecPPEM      = parse_u16_be(data, offset);
	header.fontDirectionHint  = parse_i16_be(data, offset);
	header.indexToLocFormat   = parse_enum_be<OTFIndexFormat>(data, offset);
	header.glyphDataFormat    = parse_i16_be(data, offset);

	if (header.magicNumber != 0x5f0f3cf5)
		return { OTFResultType::InvalidMagic, "invalid 'head' table magic number" };

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseMaxpTable(const uint8_t* data, const size_t size)
{
	uint32_t offset = 0;

	maxProfile.version               = parse_u32_be(data, offset);
	maxProfile.numGlyphs             = parse_u16_be(data, offset);

	if (maxProfile.version == make_version(0, 5))
		return OTFResultType::Success;

	maxProfile.maxPoints             = parse_u16_be(data, offset);
	maxProfile.maxContours           = parse_u16_be(data, offset);
	maxProfile.maxCompositePoints    = parse_u16_be(data, offset);
	maxProfile.maxCompositeContours  = parse_u16_be(data, offset);
	maxProfile.maxZones              = parse_u16_be(data, offset);
	maxProfile.maxTwilightPoints     = parse_u16_be(data, offset);
	maxProfile.maxStorage            = parse_u16_be(data, offset);
	maxProfile.maxFunctionDefs       = parse_u16_be(data, offset);
	maxProfile.maxInstructionDefs    = parse_u16_be(data, offset);
	maxProfile.maxStackElements      = parse_u16_be(data, offset);
	maxProfile.maxSizeOfInstructions = parse_u16_be(data, offset);
	maxProfile.maxComponentElements  = parse_u16_be(data, offset);
	maxProfile.maxComponentDepth     = parse_u16_be(data, offset);

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseCmapTable(const uint8_t* data, const size_t size)
{
	OTFCMAPTableHeader header;
	OTFEncodingRecord  encoding_record;
	
	uint32_t encoding_offset = parse_cmap_table_header(header, data, 0);

	for (uint16_t i = 0; i < header.numSubtables; ++i) {
		encoding_offset = parse_encoding_record(encoding_record, data, encoding_offset);

		uint32_t       subtable_offset = encoding_record.subtableOffset;
		OTFCMAPFormat  format          = parse_enum_be<OTFCMAPFormat>(data, subtable_offset);
		const uint8_t* subtable_data   = data + subtable_offset;
		size_t         subtable_size   = size - subtable_offset;

		// TODO: check size of subtable

		switch (format) {
		case OTFCMAPFormat::ByteEncodingTable:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::HighByteMappingThroughTable:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::SegmentMappingToDeltaValues:
			CHECK(parseCmapFormat4(subtable_data, subtable_size, encoding_record));
			break;
		case OTFCMAPFormat::TrimmedTableMapping:
			CHECK(parseCmapFormat6(subtable_data, subtable_size, encoding_record));
			break;
		case OTFCMAPFormat::Mixed16And32BitMapping:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::TrimmedArray:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::SegmentedCoverage:
			CHECK(parseCmapFormat12(subtable_data, subtable_size, encoding_record));
			break;
		case OTFCMAPFormat::ManyToOneRangeMappings:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::UnicodeVariationSequences:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		default:
			return { OTFResultType::InvalidFormat, "invalid cmap subtable format" };
		}
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseHheaTable(const uint8_t* data, const size_t size)
{
	uint32_t offset = 0;

	horizontalHeader.majorVersion        = parse_u16_be(data, offset);
	horizontalHeader.minorVersion        = parse_u16_be(data, offset);
	horizontalHeader.ascender            = parse_i16_be(data, offset);
	horizontalHeader.descender           = parse_i16_be(data, offset);
	horizontalHeader.lineGap             = parse_i16_be(data, offset);
	horizontalHeader.advanceWidthMax     = parse_u16_be(data, offset);
	horizontalHeader.minLeftSideBearing  = parse_i16_be(data, offset);
	horizontalHeader.minRightSideBearing = parse_i16_be(data, offset);
	horizontalHeader.xMaxExtent          = parse_i16_be(data, offset);
	horizontalHeader.caretSlopeRise      = parse_i16_be(data, offset);
	horizontalHeader.caretSlopeRun       = parse_i16_be(data, offset);
	horizontalHeader.caretOffset         = parse_i16_be(data, offset);
	horizontalHeader.reserved0           = parse_i16_be(data, offset);
	horizontalHeader.reserved1           = parse_i16_be(data, offset);
	horizontalHeader.reserved2           = parse_i16_be(data, offset);
	horizontalHeader.reserved3           = parse_i16_be(data, offset);
	horizontalHeader.metricDataFormat    = parse_i16_be(data, offset);
	horizontalHeader.numberOfHMetrics    = parse_u16_be(data, offset);

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseHmtxTable(const uint8_t* data, const size_t size)
{
	OTFLongHorMetric metric;
	OTFFWORD         bearing;
	uint32_t         offset      = 0;
	uint16_t         num_metrics = horizontalHeader.numberOfHMetrics;
	uint16_t         num_glyphs  = maxProfile.numGlyphs;
	
	for (uint16_t i = 0; i < num_metrics; ++i) {
		offset = parse_long_hor_metric(metric, data, offset);
		horizontalMetrics.longHorMetrics.push_back(metric);
	}

	for (uint16_t i = num_metrics; i < num_glyphs; ++i) {
		bearing = parse_i16_be(data, offset);
		horizontalMetrics.leftSideBearings.push_back(bearing);
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseNameTable(const uint8_t* data, const size_t size)
{
	OTFNAMETableHeader header;
	OTFNameRecord      name_record;
	std::string        str_buf;
	uint32_t           offset = 0;

	offset = parse_name_table_header(header, data, offset);

	for (uint16_t i = 0; i < header.count; ++i) {
		offset = parse_name_record(name_record, data, offset);

		if (name_record.nameID >= OTFNameID::__Count__)
			return { OTFResultType::InvalidID, "invalid name id" };

		uint32_t str_offset = header.storageOffset + name_record.stringOffset;

		str_buf.resize(name_record.length);
		memcpy(str_buf.data(), data + str_offset, str_buf.size());

		if (str_buf[0] == '\0')
			decode_utf16be_to_utf8(str_buf);

		OTFNameEntry* name_entry = findNameEntry(
			name_record.platformID,
			name_record.encodingID,
			name_record.languageID);

		if (name_entry == nullptr) {
			OTFNameEntry& new_entry = nameEntries.emplace_back();
			new_entry.platformID = name_record.platformID;
			new_entry.encodingID = name_record.encodingID;
			new_entry.languageID = name_record.languageID;
			new_entry.names.resize(static_cast<size_t>(OTFNameID::__Count__));
			new_entry.names[static_cast<size_t>(name_record.nameID)] = str_buf;
		} else {
			name_entry->names[static_cast<size_t>(name_record.nameID)] = str_buf;
		}
	}

	if (header.version == 1) {
		OTFLangTagRecord lang_tag;
		OTFUint16 lang_tag_count = parse_u16_be(data, offset);

		for (uint16_t i = 0; i < lang_tag_count; ++i) {
			lang_tag.length        = parse_u16_be(data, offset);
			lang_tag.langTagOffset = parse_u16_be(data, offset);

			uint32_t str_offset = header.storageOffset + lang_tag.langTagOffset;

			str_buf.resize(lang_tag.length);
			memcpy(str_buf.data(), data + str_offset, str_buf.size());

			decode_utf16be_to_utf8(str_buf);

			languageTags.push_back(str_buf);
		}
	}

	std::sort(VERA_SPAN(nameEntries), compare_name_entry);

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseOs2Table(const uint8_t* data, const size_t size)
{
	uint32_t offset = 0;

	os2Metrics.version             = parse_u16_be(data, offset);
	os2Metrics.xAvgCharWidth       = parse_i16_be(data, offset);
	os2Metrics.usWeightClass       = parse_u16_be(data, offset);
	os2Metrics.usWidthClass        = parse_u16_be(data, offset);
	os2Metrics.fsType              = parse_u16_be(data, offset);
	os2Metrics.ySubscriptXSize     = parse_i16_be(data, offset);
	os2Metrics.ySubscriptYSize     = parse_i16_be(data, offset);
	os2Metrics.ySubscriptXOffset   = parse_i16_be(data, offset);
	os2Metrics.ySubscriptYOffset   = parse_i16_be(data, offset);
	os2Metrics.ySuperscriptXSize   = parse_i16_be(data, offset);
	os2Metrics.ySuperscriptYSize   = parse_i16_be(data, offset);
	os2Metrics.ySuperscriptXOffset = parse_i16_be(data, offset);
	os2Metrics.ySuperscriptYOffset = parse_i16_be(data, offset);
	os2Metrics.yStrikeoutSize      = parse_i16_be(data, offset);
	os2Metrics.yStrikeoutPosition  = parse_i16_be(data, offset);
	os2Metrics.sFamilyClass        = parse_i16_be(data, offset);
	parse_array_be(os2Metrics.panose, 10, data, offset);
	os2Metrics.ulUnicodeRange1     = parse_u32_be(data, offset);
	os2Metrics.ulUnicodeRange2     = parse_u32_be(data, offset);
	os2Metrics.ulUnicodeRange3     = parse_u32_be(data, offset);
	os2Metrics.ulUnicodeRange4     = parse_u32_be(data, offset);
	parse_array_be(os2Metrics.achVendID, 4, data, offset);
	os2Metrics.fsSelection         = parse_u16_be(data, offset);
	os2Metrics.usFirstCharIndex    = parse_u16_be(data, offset);
	os2Metrics.usLastCharIndex     = parse_u16_be(data, offset);
	os2Metrics.sTypoAscender       = parse_i16_be(data, offset);
	os2Metrics.sTypoDescender      = parse_i16_be(data, offset);
	os2Metrics.sTypoLineGap        = parse_i16_be(data, offset);
	os2Metrics.usWinAscent         = parse_u16_be(data, offset);
	os2Metrics.usWinDescent        = parse_u16_be(data, offset);
	
	if (os2Metrics.version >= 1) {
		os2Metrics.ulCodePageRange1 = parse_u32_be(data, offset);
		os2Metrics.ulCodePageRange2 = parse_u32_be(data, offset);
	}
	
	if (os2Metrics.version >= 2) {
		os2Metrics.sxHeight      = parse_i16_be(data, offset);
		os2Metrics.sCapHeight    = parse_i16_be(data, offset);
		os2Metrics.usDefaultChar = parse_u16_be(data, offset);
		os2Metrics.usBreakChar   = parse_u16_be(data, offset);
		os2Metrics.usMaxContext  = parse_u16_be(data, offset);
	}
	
	if (os2Metrics.version >= 5) {
		os2Metrics.usLowerOpticalPointSize = parse_u16_be(data, offset);
		os2Metrics.usUpperOpticalPointSize = parse_u16_be(data, offset);
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parsePostTable(const uint8_t* data, const size_t size)
{
	uint32_t offset = 0;
	auto&    header = reinterpret_cast<OTFPOSTTableHeader&>(postScript);

	offset = parse_post_table_header(header, data, offset);

	if (get_version_major(header.version) < 2)
		return OTFResultType::Success;

	std::vector<std::string> glyph_names;
	OTFUint16                num_glyphs    = parse_u16_be(data, offset);
	uint16_t                 string_count  = getGlyphNameCount(num_glyphs, data, offset);
	uint32_t                 string_offset = offset + num_glyphs * sizeof(OTFUint16);

	glyph_names.resize(string_count);
	
	for (uint16_t i = 0; i < string_count; ++i) {
		uint8_t        string_length = parse_u8_be(data, string_offset);
		const uint8_t* data_ptr      = &data[string_offset];

		glyph_names[i].assign(reinterpret_cast<const char*>(data_ptr), string_length);
		string_offset += string_length;
	}

	for (uint16_t i = 0; i < num_glyphs; ++i) {
		OTFUint16 glyph_name_idx = parse_u16_be(data, offset);
		
		if (glyph_name_idx < 258) {
			const char* glyph_name = get_macintosh_glyph_names(glyph_name_idx);
			postScript.glyphNames.push_back(glyph_name);
		} else {
			std::string& glyph_name = glyph_names[glyph_name_idx - 258];
			postScript.glyphNames.push_back(glyph_name);
		}
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseCvtTable(const uint8_t* data, const size_t size)
{
	controlValues.resize(size / sizeof(OTFUint16));

	for (uint32_t offset = 0; auto& value : controlValues)
		value = parse_u16_be(data, offset);

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseFpgmTable(const uint8_t* data, const size_t size)
{
	programData.resize(size);
	memcpy(programData.data(), data, size);

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseLocaTable(const uint8_t* data, const size_t size)
{
	uint32_t offset = 0;
	uint32_t count  = maxProfile.numGlyphs + 1;

	glyphOffsets.resize(count);

	if (header.indexToLocFormat == OTFIndexFormat::ShortOffsets) {
		if (size < count * sizeof(OTFUint16))
			return { OTFResultType::InvalidSize, "data size too small for 'loca' table" };

		for (uint32_t i = 0; i < count; ++i)
			glyphOffsets[i] = parse_u16_be(data, offset) * 2;
	} else if (header.indexToLocFormat == OTFIndexFormat::LongOffsets) {
		if (size < count * sizeof(OTFUint32))
			return { OTFResultType::InvalidSize, "data size too small for 'loca' table" };

		for (uint32_t i = 0; i < count; ++i)
			glyphOffsets[i] = parse_u32_be(data, offset);
	} else {
		return { OTFResultType::InvalidFormat, "invalid 'loca' table index format" };
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseCmapFormat4(const uint8_t* data, const size_t size, const OTFEncodingRecord& encoding_record)
{
	uint32_t      offset         = 0;
	OTFUint16     length         = parse_u16_be(data, offset);
	OTFLanguageID language       = parse_language_id(data, offset, encoding_record.platformID);
	OTFUint16     seg_count      = parse_u16_be(data, offset) / 2;
/*  OTFUint16     search_range   = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
/*  OTFUint16     entry_selector = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
/*  OTFUint16     range_shift    = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
	
	uint32_t seg_count_size      = seg_count * sizeof(OTFUint16);
	uint32_t start_code_offset   = offset + seg_count_size + sizeof(uint16_t);
	uint32_t end_code_offset     = offset;
	uint32_t id_delta_offset     = start_code_offset + seg_count_size;
	uint32_t id_range_off_offset = id_delta_offset + seg_count_size;

	if (!supported_cmap_encoding_format4(encoding_record.encodingID))
		return { OTFResultType::Unsupported, "unsupported cmap format4 encoding" };

	for (uint16_t i = 0; i < seg_count; ++i) {
		char32_t start_code      = static_cast<char32_t>(parse_u16_be(data, start_code_offset));
		char32_t end_code        = static_cast<char32_t>(parse_u16_be(data, end_code_offset));
		uint32_t array_offset    = id_range_off_offset;
		uint32_t id_range_offset = static_cast<uint32_t>(parse_u16_be(data, id_range_off_offset));
		int16_t  id_delta        = parse_i16_be(data, id_delta_offset);

		if (id_range_offset == 0) {
			for (char32_t i = start_code; i <= end_code; ++i) {
				auto glyph_idx = static_cast<OTFUint16>(i + id_delta);
				charToGlyphMap[i] = glyph_idx;
			}
		} else {
			for (char32_t i = start_code; i <= end_code; ++i) {
				uint32_t  idx_offset = id_range_offset + 2 * (i - start_code) + array_offset;
				OTFUint16 glyph_idx  = parse_u16_be(data, idx_offset);

				if (glyph_idx != 0)
					glyph_idx += id_delta;

				charToGlyphMap[i] = glyph_idx;
			}
		}
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseCmapFormat6(const uint8_t* data, const size_t size, const OTFEncodingRecord& encoding_record)
{
	uint32_t      offset      = 0;
	OTFLanguageID language    = parse_language_id(data, offset, encoding_record.platformID);
	OTFUint16     first_code  = parse_u16_be(data, offset);
	OTFUint16     entry_count = parse_u16_be(data, offset);

	for (uint16_t i = 0; i < entry_count; ++i) {
		char32_t  code     = static_cast<char32_t>(first_code + i);
		OTFUint16 glyph_id = parse_u16_be(data, offset);

		charToGlyphMap[code] = glyph_id;
	}

	return OTFResultType::Success;
}

OTFResult OpenTypeImpl::parseCmapFormat12(const uint8_t* data, const size_t size, const OTFEncodingRecord& encoding_record)
{
	OTFSequentialMapGroup map_group;

	uint32_t offset      = 0;
/*  OTFUint16 reserved   = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
	OTFUint32 length     = parse_u32_be(data, offset);
	OTFUint32 language   = parse_u32_be(data, offset);
	OTFUint32 num_groups = parse_u32_be(data, offset);

	for (uint32_t i = 0; i < num_groups; ++i) {
		offset = parse_sequential_map_group(map_group, data, offset);
		
		uint32_t code_count      = map_group.endCharCode - map_group.startCharCode + 1;
		uint32_t first_glyph_idx = map_group.startGlyphID;
		uint32_t last_glyph_idx  = first_glyph_idx + code_count;
		char32_t code            = static_cast<char32_t>(map_group.startCharCode);

		for (uint32_t i = first_glyph_idx; i < last_glyph_idx; ++i, ++ code)
			charToGlyphMap[code] = i;
	}

	return OTFResultType::Success;
}

OTFNameEntry* OpenTypeImpl::findNameEntry(OTFPlatformID platform_id, OTFEncodingID encoding_id, OTFLanguageID language_id)
{
	auto cmp_entry = OTFNameEntry{ platform_id, encoding_id, language_id, {} };
	auto it        = std::lower_bound(VERA_SPAN(nameEntries), cmp_entry, compare_name_entry);

	return
		it != nameEntries.end()&&
		it->platformID == platform_id &&
		it->encodingID == encoding_id &&
		it->languageID == language_id ?
		&(*it) : nullptr;
}

uint16_t OpenTypeImpl::getGlyphNameCount(OTFUint16 num_glyphs, const uint8_t* data, uint32_t offset)
{
	OTFUint16 max_count = 0;

	for (uint16_t i = 0; i < num_glyphs; ++i) {
		OTFUint16 glyph_name_idx = parse_u16_be(data, offset);
		max_count = std::max<OTFUint16>(max_count, glyph_name_idx + 1);
	}

	return max_count;
}

VERA_NAMESPACE_END
