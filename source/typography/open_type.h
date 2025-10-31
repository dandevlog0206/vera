#define OPEN_TYPE_IMPLEMENTAION

#pragma once

#include "../../include/vera/typography/glyph.h"
#include "../../include/vera/util/result_message.h"
#include "../../include/vera/util/ranged_set.h"
#include "../parse.h"
#include <unordered_map>
#include <map>
#include <algorithm>
#include <string>

#define OTF_CHECK(expression)                          \
	do {                                               \
		OTFResult result = expression;                 \
		if (result.result() != OTFResultType::Success) \
			return result;                             \
	} while (0)

VERA_NAMESPACE_BEGIN

typedef int8_t  OTFInt8;
typedef uint8_t OTFUint8;
typedef int16_t  OTFInt16;
typedef uint16_t OTFUint16;
typedef int32_t OTFInt32;
typedef uint32_t OTFUint32;
typedef uint32_t OTFFixed;
typedef int16_t OTFFWORD;
typedef uint16_t OTFUFWORD;
typedef uint16_t OTFF2DOT14;
typedef uint64_t OTFLONGDATETIME;
typedef int8_t OTFOffset8;
typedef int16_t OTFOffset16;
typedef int32_t OTFOffset32;
typedef uint32_t OTFVersion;
typedef uint16_t OTFGlyphID;

typedef float (*OTFVectorParser)(const uint8_t*, uint32_t&);

enum class OTFResultType
{
	Success,
	MissingTable,
	Unsupported,
	InvalidID,
	InvalidMagic,
	InvalidSize,
	InvalidIndex,
	InvalidRange,
	InvalidFormat,
	MissingCodepoint,
	MissingGlyph,
	OutOfBounds
};

typedef ResultMessage<OTFResultType> OTFResult;

enum class OTFSFNTVersion : uint32_t
{
	TrueType = 0x00010000,
	CFF      = 0x4F54544F, // 'OTTO'
};

enum class OTFTTCTag : uint32_t
{
	Value = 0x74746366 // 'ttcf'
};

enum class OTFTableTag : uint32_t
{
	// Required tables
	HEAD = 0x68656164, // 'head'
	MAXP = 0x6D617870, // 'maxp'
	CMAP = 0x636D6170, // 'cmap'
	HHEA = 0x68686561, // 'hhea'
	HMTX = 0x686D7478, // 'hmtx'
	NAME = 0x6E616D65, // 'name'
	OS2  = 0x4F532F32, // 'OS/2'
	POST = 0x706F7374, // 'post'

	// Tables Related to TrueType Outlines
	CVT  = 0x63767420, // 'cvt '
	FPGM = 0x6670676D, // 'fpgm'
	LOCA = 0x6C6F6361, // 'loca'
	GLYF = 0x676C7966, // 'glyf'
	PREP = 0x70726570, // 'prep'
	GASP = 0x67617370, // 'gasp'

	// Table Related to SVG Outlines
	CFF  = 0x43464620, // 'CFF '
	CFF2 = 0x43464632, // 'CFF2'
	VORG = 0x564F5247, // 'VORG'

	// Table related to SVG outlines
	SVG  = 0x53564720, // 'SVG '
	
	// Tables Related to Bitmap Glyphs
	EBDT = 0x45424454, // 'EBDT'
	EBLC = 0x45424C43, // 'EBLC'
	EBSC = 0x45425343, // 'EBSC'
	CBDT = 0x43424454, // 'CBDT'
	CBLC = 0x43424C43, // 'CBLC'
	SBIX = 0x73626978, // 'sbix'

	// Advanced Typographic Tables
	BASE = 0x42415345, // 'BASE'
	GDEF = 0x47444546, // 'GDEF'
	GPOS = 0x47504F53, // 'GPOS'
	GSUB = 0x47535542, // 'GSUB'
	JSTF = 0x4A535446, // 'JSTF'
	MATH = 0x4D415448, // 'MATH'

	// Tables used for OpenType Font Variations
	AVAR = 0x61766172, // 'avar'
	CVAR = 0x63766172, // 'cvar'
	FVAR = 0x66766172, // 'fvar'
	GVAR = 0x67766172, // 'gvar'
	HVAR = 0x48564152, // 'HVAR'
	MVAR = 0x4D564152, // 'MVAR'
	STAT = 0x53544154, // 'STAT'
	VVAR = 0x56564152, // 'VVAR'

	// Tables Related to Color Fonts
	COLR = 0x434F4C52, // 'COLR'
	CPAL = 0x4350414C, // 'CPAL'
//  CBDT = 0x43424454, // 'CBDT'
//  CBLC = 0x43424C43, // 'CBLC'
//  SBIX = 0x73626978, // 'sbix'
//  SVG  = 0x53564720, // 'SVG '

	// Other OpenType Tables
	DSIG = 0x44534947, // 'DSIG'
	HDMX = 0x68646D78, // 'hdmx'
	KERN = 0x6B65726E, // 'kern'
	LTSH = 0x4C545348, // 'LTSH'
	MERG = 0x4D455247, // 'MERG'
	META = 0x6D657461, // 'meta'
	PCLT = 0x50434C54, // 'PCLT'
	VDMX = 0x56444D58, // 'VDMX'
	VHEA = 0x76686561, // 'vhea'
	VMTX = 0x766D7478  // 'vmtx'
};

enum class OTFIndexFormat : uint16_t
{
	ShortOffsets = 0,
	LongOffsets  = 1
};

enum class OTFPlatformID : uint16_t
{
	Unicode       = 0,
	Macintosh     = 1,
	ISO           = 2,
	Windows       = 3,
	Custom        = 4
};

enum class OTFEncodingID : uint16_t
{
	// Unicode
	Unicode_1_0_Semantics           = 0,
	Unicode_1_1_Semantics           = 1,
	Unicode_ISO_10646               = 2,
	Unicode_BMP                     = 3,
	Unicode_Full_Repertoire         = 4,

	// Macintosh
	Macintosh_Roman                 = 256,
	Macintosh_Japanese              = 257,
	Macintosh_TraditionalChinese    = 258,
	Macintosh_Korean                = 259,
	Macintosh_Arabic                = 260,
	Macintosh_Hebrew                = 261,
	Macintosh_Greek                 = 262,
	Macintosh_Russian               = 263,
	Macintosh_RSymbol               = 264,
	Macintosh_Devanagari            = 265,
	Macintosh_Gurmukhi              = 266,
	Macintosh_Gujarati              = 267,
	Macintosh_Oriya                 = 268,
	Macintosh_Bengali               = 269,
	Macintosh_Tamil                 = 270,
	Macintosh_Telugu                = 271,
	Macintosh_Kannada               = 272,
	Macintosh_Malayalam             = 273,
	Macintosh_Sinhalese             = 274,
	Macintosh_Burmese               = 275,
	Macintosh_Khmer                 = 276,
	Macintosh_Thai                  = 277,
	Macintosh_Laotian               = 278,
	Macintosh_Georgian              = 279,
	Macintosh_Armenian              = 280,
	Macintosh_SimplifiedChinese     = 281,
	Macintosh_Tibetan               = 282,
	Macintosh_Mongolian             = 283,
	Macintosh_Geez                  = 284,
	Macintosh_Slavic                = 285,
	Macintosh_Vietnamese            = 286,
	Macintosh_Sindhi                = 287,
	Macintosh_Uninterpreted         = 288,

	// ISO
	ISO_ASCII                       = 512,
	ISO_10646                       = 513,
	ISO_8859_1                      = 514,

	// Windows
	Windows_Symbol                  = 768,
	Windows_Unicode_BMP             = 769,
	Windows_ShiftJIS                = 770,
	Windows_PRC                     = 771,
	Windows_Big5                    = 772,
	Windows_Wansung                 = 773,
	Windows_Johab                   = 774,
	Windows_Reserved0               = 775,
	Windows_Reserved1               = 775,
	Windows_Reserved2               = 775,
	Windows_Uniocde_Full_Repertoire = 776,

	__Platform_Offset__             = 256
};

enum class OTFLanguageID : uint16_t
{
	// No language id for Unicode

	// Macintosh
	Macintosh_English               = 256,
	Macintosh_French                = 257,
	Macintosh_German                = 258,
	Macintosh_Italian               = 259,
	Macintosh_Dutch                 = 260,
	Macintosh_Swedish               = 261,
	Macintosh_Spanish               = 262,
	Macintosh_Danish                = 263,
	Macintosh_Portuguese            = 264,
	Macintosh_Norwegian             = 265,
	Macintosh_Hebrew                = 266,
	Macintosh_Japanese              = 267,
	Macintosh_Arabic                = 268,
	Macintosh_Finnish               = 269,
	Macintosh_Greek                 = 270,
	Macintosh_Icelandic             = 271,
	Macintosh_Maltese               = 272,
	Macintosh_Turkish               = 273,
	Macintosh_Croatian              = 274,
	Macintosh_Chinese_Traditional   = 275,
	Macintosh_Urdu                  = 276,
	Macintosh_Hindi                 = 277,
	Macintosh_Thai                  = 278,
	Macintosh_Korean                = 279,
	Macintosh_Lithuanian            = 280,
	Macintosh_Polish                = 281,
	Macintosh_Hungarian             = 282,
	Macintosh_Estonian              = 283,
	Macintosh_Latvian               = 284,
	Macintosh_Sami                  = 285,
	Macintosh_Faroese               = 286,
	Macintosh_Farsi_Persian         = 287,
	Macintosh_Russian               = 288,
	Macintosh_Chinese_Simplified    = 289,
	Macintosh_Flemish               = 290,
	Macintosh_Irish_Gaelic          = 291,
	Macintosh_Albanian              = 292,
	Macintosh_Romanian              = 293,
	Macintosh_Czech                 = 294,
	Macintosh_Slovak                = 295,
	Macintosh_Slovenian             = 296,
	Macintosh_Yiddish               = 297,
	Macintosh_Serbian               = 298,
	Macintosh_Macedonian            = 299,
	Macintosh_Bulgarian             = 300,
	Macintosh_Ukrainian             = 301,
	Macintosh_Byelorussian          = 302,
	Macintosh_Uzbek                 = 303,
	Macintosh_Kazakh                = 304,
	Macintosh_Azerbaijani_Cyrillic  = 305,
	Macintosh_Azerbaijani_Arabic    = 306,
	Macintosh_Armenian              = 307,
	Macintosh_Georgian              = 308,
	Macintosh_Moldavian             = 309,
	Macintosh_Kirghiz               = 310,
	Macintosh_Tajiki                = 311,
	Macintosh_Turkmen               = 312,
	Macintosh_Mongolian_Mongolian   = 313,
	Macintosh_Mongolian_Cyrillic    = 314,
	Macintosh_Pashto                = 315,
	Macintosh_Kurdish               = 316,
	Macintosh_Kashmiri              = 317,
	Macintosh_Sindhi                = 318,
	Macintosh_Tibetan               = 319,
	Macintosh_Nepali                = 320,
	Macintosh_Sanskrit              = 321,
	Macintosh_Marathi               = 322,
	Macintosh_Bengali               = 323,
	Macintosh_Assamese              = 324,
	Macintosh_Gujarati              = 325,
	Macintosh_Punjabi               = 326,
	Macintosh_Oriya                 = 327,
	Macintosh_Malayalam             = 328,
	Macintosh_Kannada               = 329,
	Macintosh_Tamil                 = 330,
	Macintosh_Telugu                = 331,
	Macintosh_Sinhalese             = 332,
	Macintosh_Burmese               = 333,
	Macintosh_Khmer                 = 334,
	Macintosh_Lao                   = 335,
	Macintosh_Vietnamese            = 336,
	Macintosh_Indonesian            = 337,
	Macintosh_Tagalog               = 338,
	Macintosh_Malay_Roman           = 339,
	Macintosh_Malay_Arabic          = 340,
	Macintosh_Amharic               = 341,
	Macintosh_Tigrinya              = 342,
	Macintosh_Galla                 = 343,
	Macintosh_Somali                = 344,
	Macintosh_Swahili               = 345,
	Macintosh_Kinyarwanda_Ruanda    = 346,
	Macintosh_Rundi                 = 347,
	Macintosh_Nyanja_Chewa          = 348,
	Macintosh_Malagasy              = 349,
	Macintosh_Esperanto             = 350,
	Macintosh_Welsh                 = 384,
	Macintosh_Basque                = 385,
	Macintosh_Catalan               = 386,
	Macintosh_Latin                 = 387,
	Macintosh_Quechua               = 388,
	Macintosh_Guarani               = 389,
	Macintosh_Aymara                = 390,
	Macintosh_Tatar                 = 391,
	Macintosh_Uighur                = 392,
	Macintosh_Dzongkha              = 393,
	Macintosh_Javanese_Roman        = 394,
	Macintosh_Sundanese_Roman       = 395,
	Macintosh_Galician              = 396,
	Macintosh_Afrikaans             = 397,
	Macintosh_Breton                = 398,
	Macintosh_Inuktitut             = 399,
	Macintosh_Scottish_Gaelic       = 400,
	Macintosh_Manx_Gaelic           = 401,
	Macintosh_Irish_Gaelic_DotAbove = 402,
	Macintosh_Tongan                = 403,
	Macintosh_Greek_Polytonic       = 404,
	Macintosh_Greenlandic           = 405,
	Macintosh_Azerbaijani_Roman     = 406,

	// no language ID for ISO

	// no language ID for Windows

	__Platform_Offset__             = 256
};

enum class OTFCMAPFormat : uint16_t
{
	ByteEncodingTable           = 0,
	HighByteMappingThroughTable = 2,
	SegmentMappingToDeltaValues = 4,
	TrimmedTableMapping         = 6,
	Mixed16And32BitMapping      = 8,
	TrimmedArray                = 10,
	SegmentedCoverage           = 12,
	ManyToOneRangeMappings      = 13,
	UnicodeVariationSequences   = 14
};

enum class OTFNameID : uint16_t
{
	Copyright                      = 0,
	FontFamily                     = 1,
	FontSubfamily                  = 2,
	UniqueFontIdentifier           = 3,
	FullFontName                   = 4,
	VersionString                  = 5,
	PostScriptName                 = 6,
	Trademark                      = 7,
	ManufacturerName               = 8,
	Designer                       = 9,
	Description                    = 10,
	VendorURL                      = 11,
	DesignerURL                    = 12,
	LicenseDescription             = 13,
	LicenseInfoURL                 = 14,
	Reserved                       = 15,
	TypographicFamily              = 16,
	TypographicSubfamily           = 17,
	CompatibleFull                 = 18,
	SampleText                     = 19,
	PostScriptCIDFindfontName      = 20,
	WWSFamilyName                  = 21,
	WWSSubfamilyName               = 22,
	LightBackgroundPalette         = 23,
	DarkBackgroundPalette          = 24,
	VariationsPostScriptNamePrefix = 25,
	__Count__                      = 26,
};

enum class OTFGlyphFlagBits : uint8_t
{
	OnCurvePoint       = 0x01,
	XShortVector       = 0x02,
	YShortVector       = 0x04,
	RepeatFlag         = 0x08,
	XIsSameOrPositive  = 0x10,
	YIsSameOrPositive  = 0x20,
	OverlapSimple      = 0x40,
	Reserved           = 0x80
};

enum class OTFComponentGlyphFlagBits : uint16_t
{
	Arg1And2AreWords        = 0x0001,
	ArgsAreXYValues         = 0x0002,
	RoundXYToGrid           = 0x0004,
	WeHaveAScale            = 0x0008,
	MoreComponents          = 0x0020,
	WeHaveAnXAndYScale      = 0x0040,
	WeHaveATwoByTwo         = 0x0080,
	WeHaveInstructions      = 0x0100,
	UseMyMetrics            = 0x0200,
	OverlapCompound         = 0x0400,
	ScaledComponentOffset   = 0x0800,
	UnscaledComponentOffset = 0x1000
};

struct TTCHeader
{
	OTFTTCTag magic;
	OTFUint16 majorVersion;
	OTFUint16 minorVersion;
	OTFUint32 numFonts;
};

struct TTFHeader
{
	OTFSFNTVersion sfntVersion;
	OTFUint16      numTables;
	OTFUint16      searchRange;
	OTFUint16      entrySelector;
	OTFUint16      rangeShift;
};

struct OTFTableDirectoryHeader
{
	OTFSFNTVersion sfntVersion;
	OTFUint16      numTables;
	OTFUint16      searchRange;
	OTFUint16      entrySelector;
	OTFUint16      rangeShift;
};

struct OTFTableRecord
{
	OTFTableTag tableTag;
	OTFUint32   checkSum;
	OTFUint32   offset;
	OTFUint32   length;
};

// HEAD: Font Header Table

struct OTFHEADTable
{
	OTFVersion      version;
	OTFFixed        fontRevision;
	OTFUint32       checkSumAdjustment;
	OTFUint32       magicNumber;
	OTFUint16       flags;
	OTFFWORD        unitsPerEm;
	OTFLONGDATETIME created;
	OTFLONGDATETIME modified;
	OTFInt16        xMin;
	OTFInt16        yMin;
	OTFInt16        xMax;
	OTFInt16        yMax;
	OTFUint16       macStyle;
	OTFUint16       lowestRecPPEM;
	OTFInt16        fontDirectionHint;
	OTFIndexFormat  indexToLocFormat;
	OTFInt16        glyphDataFormat;
};

// MAXP: Maximum Profile Table

struct OTFMAXPTable
{
	OTFVersion version;
	OTFUint16  numGlyphs;
	OTFUint16  maxPoints;
	OTFUint16  maxContours;
	OTFUint16  maxCompositePoints;
	OTFUint16  maxCompositeContours;
	OTFUint16  maxZones;
	OTFUint16  maxTwilightPoints;
	OTFUint16  maxStorage;
	OTFUint16  maxFunctionDefs;
	OTFUint16  maxInstructionDefs;
	OTFUint16  maxStackElements;
	OTFUint16  maxSizeOfInstructions;
	OTFUint16  maxComponentElements;
	OTFUint16  maxComponentDepth;
};

// CMAP: Character Map Table

struct OTFCMAPTableHeader
{
	OTFUint16 version;
	OTFUint16 numSubtables;
};

struct OTFEncodingRecord
{
	OTFPlatformID platformID;
	OTFEncodingID encodingID;
	OTFOffset32   subtableOffset;
};

struct OTFSequentialMapGroup
{
	OTFUint32 startCharCode;
	OTFUint32 endCharCode;
	OTFUint32 startGlyphID;
};

class OTFCMAPTable
{
public:
	OTFUint16                                version        = {};
	OTFUint16                                numSubtables   = {};
	std::unordered_map<char32_t, OTFGlyphID> charToGlyphMap = {};
};

// HHEA: Horizontal Header Table

struct OTFHHEATable
{
	OTFUint16 majorVersion;
	OTFUint16 minorVersion;
	OTFFWORD  ascender;
	OTFFWORD  descender;
	OTFFWORD  lineGap;
	OTFUFWORD advanceWidthMax;
	OTFFWORD  minLeftSideBearing;
	OTFFWORD  minRightSideBearing;
	OTFFWORD  xMaxExtent;
	OTFInt16  caretSlopeRise;
	OTFInt16  caretSlopeRun;
	OTFInt16  caretOffset;
	OTFInt16  reserved0;
	OTFInt16  reserved1;
	OTFInt16  reserved2;
	OTFInt16  reserved3;
	OTFInt16  metricDataFormat;
	OTFUint16 numberOfHMetrics;
};

// HMTX: Horizontal Metrics Table

struct OTFLongHorMetric
{
	OTFUFWORD advanceWidth;
	OTFFWORD  leftSideBearing;
};

class OTFHMTXTable
{
public:
	std::vector<OTFLongHorMetric> longHorMetrics   = {};
	std::vector<OTFFWORD>         leftSideBearings = {};
};

// NAME: Naming Table

struct OTFNAMETableHeader
{
	OTFUint16 version;
	OTFUint16 count;
	OTFUint16 storageOffset;
};

struct OTFNameRecord
{
	OTFPlatformID platformID;
	OTFEncodingID encodingID;
	OTFLanguageID languageID;
	OTFNameID     nameID;
	OTFUint16     length;
	OTFUint16     stringOffset;
};

struct OTFLangTagRecord
{
	OTFUint16 length;
	OTFUint16 langTagOffset;
};

class OTFNameEntry
{
public:
	OTFPlatformID            platformID = {};
	OTFEncodingID            encodingID = {};
	OTFLanguageID            languageID = {};
	std::vector<std::string> names      = {};
};

class OTFNAMETable
{
public:
	OTFUint16                 version       = {};
	OTFUint16                 count         = {};
	OTFUint16                 storageOffset = {};
	std::vector<OTFNameEntry> names         = {};
	std::vector<std::string>  languageTags  = {};
};

// OS2: OS/2 and Windows Metrics Table

struct OTFOS2Table
{
	OTFUint16 version;
	OTFInt16  xAvgCharWidth;
	OTFUint16 usWeightClass;
	OTFUint16 usWidthClass;
	OTFUint16 fsType;
	OTFInt16  ySubscriptXSize;
	OTFInt16  ySubscriptYSize;
	OTFInt16  ySubscriptXOffset;
	OTFInt16  ySubscriptYOffset;
	OTFInt16  ySuperscriptXSize;
	OTFInt16  ySuperscriptYSize;
	OTFInt16  ySuperscriptXOffset;
	OTFInt16  ySuperscriptYOffset;
	OTFInt16  yStrikeoutSize;
	OTFInt16  yStrikeoutPosition;
	OTFInt16  sFamilyClass;
	OTFUint8  panose[10];
	OTFUint32 ulUnicodeRange1;
	OTFUint32 ulUnicodeRange2;
	OTFUint32 ulUnicodeRange3;
	OTFUint32 ulUnicodeRange4;
	OTFUint8  achVendID[4];
	OTFUint16 fsSelection;
	OTFUint16 usFirstCharIndex;
	OTFUint16 usLastCharIndex;
	OTFInt16  sTypoAscender;
	OTFInt16  sTypoDescender;
	OTFInt16  sTypoLineGap;
	OTFUint16 usWinAscent;
	OTFUint16 usWinDescent;
	OTFUint32 ulCodePageRange1;
	OTFUint32 ulCodePageRange2;
	OTFInt16  sxHeight;
	OTFInt16  sCapHeight;
	OTFUint16 usDefaultChar;
	OTFUint16 usBreakChar;
	OTFUint16 usMaxContext;
	OTFUint16 usLowerOpticalPointSize;
	OTFUint16 usUpperOpticalPointSize;
};

// POST: Post Script Table

struct OTFPOSTTableHeader
{
	OTFVersion version;
	OTFFixed   italicAngle;
	OTFFWORD   underlinePosition;
	OTFFWORD   underlineThickness;
	OTFUint32  isFixedPitch;
	OTFUint32  minMemType42;
	OTFUint32  maxMemType42;
	OTFUint32  minMemType1;
	OTFUint32  maxMemType1;
};

class OTFPOSTTable
{
public:
	OTFVersion               version;
	OTFFixed                 italicAngle;
	OTFFWORD                 underlinePosition;
	OTFFWORD                 underlineThickness;
	OTFUint32                isFixedPitch;
	OTFUint32                minMemType42;
	OTFUint32                maxMemType42;
	OTFUint32                minMemType1;
	OTFUint32                maxMemType1;
	std::vector<std::string> glyphNames;
};

// glyf: Glyph Data Table

struct OTFGlyphHeader
{
	OTFInt16 numberOfContours;
	OTFInt16 xMin;
	OTFInt16 yMin;
	OTFInt16 xMax;
	OTFInt16 yMax;
};

class OTFFont
{
public:
	using TableMap = std::unordered_map<OTFTableTag, OTFTableRecord>;
	using GlyphMap = std::map<OTFGlyphID, Glyph>;

	TableMap              tableMap          = {};
	OTFHEADTable          head              = {};
	OTFMAXPTable          maxProfile        = {};
	OTFCMAPTable          characterMap      = {};
	OTFHHEATable          horizontalHeader  = {};
	OTFHMTXTable          horizontalMetrics = {};
	OTFNAMETable          nameTable         = {};
	OTFOS2Table           os2Metrics        = {};
	OTFPOSTTable          postScript        = {};
	std::vector<uint8_t>  binaryData        = {};
	std::vector<uint32_t> glyphOffsets      = {};
	std::vector<OTFInt16> controlValues     = {}; // cvt table
	std::vector<OTFUint8> programData       = {}; // fpgm table
	GlyphMap              glyphs            = {};
	ranged_set<char32_t>  loadedCodePoints  = {};
};

class OTFFontCollection
{
public:
	std::vector<OTFFont> fonts = {};
};

extern OTFResult otf_load_ttc(OTFFontCollection& font_collection, const uint8_t* data, const size_t size);
extern OTFResult otf_get_ttc_font_count(const uint8_t* data, const size_t size, uint32_t* font_count);
extern OTFResult otf_load_ttf(OTFFont& font, const uint8_t* data, const size_t size);
extern OTFResult otf_load_glyph_range(OTFFont& font, GlyphID start, GlyphID end);
extern OTFResult otf_load_code_range(OTFFont& font, char32_t start, char32_t end);
extern OTFResult otf_try_load_code_range(OTFFont& font, char32_t start, char32_t end);

VERA_NAMESPACE_END