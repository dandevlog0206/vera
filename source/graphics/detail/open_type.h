#pragma once

#include "../../../include/vera/util/result_message.h"
#include "../../../include/vera/util/ranged_set.h"
#include "../../../include/vera/geometry/path.h"
#include "../../parse.h"
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

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
typedef uint32_t OTFLONGDATETIME;
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
	InvalidFormat
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
	OTFUint16                                version;
	OTFUint16                                numSubtables;
	std::unordered_map<char32_t, OTFGlyphID> charToGlyphMap;
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
	std::vector<OTFLongHorMetric> longHorMetrics;
	std::vector<OTFFWORD>         leftSideBearings;
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
	OTFPlatformID            platformID;
	OTFEncodingID            encodingID;
	OTFLanguageID            languageID;
	std::vector<std::string> names;
};

class OTFNAMETable
{
public:
	OTFUint16                 version;
	OTFUint16                 count;
	OTFUint16                 storageOffset;
	std::vector<OTFNameEntry> names;
	std::vector<std::string>  languageTags;
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

class OTFGlyph
{
public:
	OTFGlyphID                   glyphID;
	OTFUint16                    numContours;
	uint16_t                     advanceWidth;
	std::vector<OTFUint8>        instructions;
	std::vector<QuadraticPath2D> contours;
};

class OTFFont
{
public:
	OTFHEADTable          head;
	OTFMAXPTable          maxProfile;
	OTFCMAPTable          characterMap;
	OTFHHEATable          horizontalHeader;
	OTFHMTXTable          horizontalMetrics;
	OTFNAMETable          nameTable;
	OTFOS2Table           os2Metrics;
	OTFPOSTTable          postScript;
	std::vector<uint8_t>  binaryData;
	std::vector<OTFGlyph> glyphs;
	std::vector<OTFInt16> controlValues; // cvt table
	std::vector<OTFUint8> programData;   // fpgm table
	ranged_set<uint16_t>  loadedGlyphIDs;
};

class OTFFontCollection
{
public:
	std::vector<OTFFont> fonts;
};

VERA_PRIV_NAMESPACE_BEGIN

static bool otf_has_flag(OTFGlyphFlagBits a, OTFGlyphFlagBits b)
{
	return static_cast<bool>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

static bool otf_has_flag(OTFComponentGlyphFlagBits a, OTFComponentGlyphFlagBits b)
{
	return static_cast<bool>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}

static OTFEncodingID otf_parse_encoding_id(const uint8_t* data, uint32_t& offset, OTFPlatformID platform_id)
{
	uint16_t encoding_id     = parse_u16_be(data, offset);
	uint16_t platform_offset = static_cast<uint16_t>(OTFEncodingID::__Platform_Offset__);
	uint16_t encoding_offset = platform_offset * static_cast<uint16_t>(platform_id);

	return static_cast<OTFEncodingID>(encoding_id + encoding_offset);
}

static OTFLanguageID otf_parse_language_id(const uint8_t* data, uint32_t& offset, OTFPlatformID platform_id)
{
	uint16_t language_id     = parse_u16_be(data, offset);
	uint16_t platform_offset = static_cast<uint16_t>(OTFLanguageID::__Platform_Offset__);
	uint16_t language_offset = platform_offset * static_cast<uint16_t>(platform_id);
	return static_cast<OTFLanguageID>(language_id + language_offset);
}

static void otf_decode_utf16be_to_utf8(std::string& str_buf)
{
	size_t new_buf_size = str_buf.size() / 2;

	for (size_t i = 0; i < new_buf_size; i++)
		str_buf[i] = str_buf[2 * i + 1];

	str_buf.resize(new_buf_size);
}

static OTFNameEntry* otf_find_name_entry_linear(
	OTFNAMETable& name_table,
	OTFPlatformID platform_id,
	OTFEncodingID encoding_id,
	OTFLanguageID language_id
) {
	for (auto& entry : name_table.names) {
		if (entry.platformID == platform_id &&
			entry.encodingID == encoding_id &&
			entry.languageID == language_id) {
			return &entry;
		}
	}

	return nullptr;
}

static bool otf_compare_name_entry(const OTFNameEntry& a, const OTFNameEntry& b) {
	if (a.platformID != b.platformID)
		return a.platformID < b.platformID;
	if (a.encodingID != b.encodingID)
		return a.encodingID < b.encodingID;
	return a.languageID < b.languageID;
}

static float otf_fixed_to_float(OTFFixed value)
{
	int16_t  integer  = static_cast<int16_t>((value >> 16) & 0xFFFF);
	uint16_t fraction = static_cast<uint16_t>(value & 0xFFFF);
	return static_cast<float>(integer) + static_cast<float>(fraction) / 65536.0f;
}

static OTFFixed otf_float_to_fixed(float value)
{
	int16_t  integer  = static_cast<int16_t>(value);
	uint16_t fraction = static_cast<uint16_t>((value - static_cast<float>(integer)) * 65536.0f);
	return (static_cast<OTFFixed>(integer) << 16) | static_cast<OTFFixed>(fraction);
}

static float otf_f2dot14_to_float(OTFF2DOT14 value)
{
	int16_t  integer  = static_cast<int16_t>((value >> 14) & 0x3);
	uint16_t fraction = static_cast<uint16_t>(value & 0x3FFF);
	return static_cast<float>(integer) + static_cast<float>(fraction) / 16384.0f;
}

static OTFF2DOT14 otf_float_to_f2dot14(float value)
{
	int16_t  integer  = static_cast<int16_t>(value);
	uint16_t fraction = static_cast<uint16_t>((value - static_cast<float>(integer)) * 16384.0f);
	return (static_cast<OTFF2DOT14>(integer) << 14) | static_cast<OTFF2DOT14>(fraction);
}

static uint16_t otf_get_version_major(OTFVersion version)
{
	return static_cast<uint16_t>((version >> 8) & 0xFF);
}

static uint16_t otf_get_version_minor(OTFVersion version)
{
	return static_cast<uint16_t>(version & 0xFF);
}

static OTFVersion otf_make_version(uint16_t major, uint16_t minor)
{
	return static_cast<OTFVersion>(major) << 8 |
		   static_cast<OTFVersion>(minor);
}

static uint32_t otf_parse_ttc_header(TTCHeader& header, const uint8_t* data, uint32_t offset)
{
	header.magic        = parse_enum_be<OTFTTCTag>(data, offset);
	header.majorVersion = parse_u16_be(data, offset);
	header.minorVersion = parse_u16_be(data, offset);
	header.numFonts     = parse_u32_be(data, offset);
	return offset;
}

static uint32_t otf_parse_ttf_header(TTFHeader& header, const uint8_t* data, uint32_t offset)
{
	header.sfntVersion   = parse_enum_be<OTFSFNTVersion>(data, offset);
	header.numTables     = parse_u16_be(data, offset);
	header.searchRange   = parse_u16_be(data, offset);
	header.entrySelector = parse_u16_be(data, offset);
	header.rangeShift    = parse_u16_be(data, offset);
	return offset;
}

static uint32_t otf_parse_table_record(OTFTableRecord& record, const uint8_t* data, uint32_t offset)
{
	record.tableTag = parse_enum_be<OTFTableTag>(data, offset);
	record.checkSum = parse_u32_be(data, offset);
	record.offset   = parse_u32_be(data, offset);
	record.length   = parse_u32_be(data, offset);
	return offset;
}

static uint32_t otf_parse_cmap_table_header(OTFCMAPTableHeader& header, const uint8_t* data, uint32_t offset)
{
	header.version      = parse_u16_be(data, offset);
	header.numSubtables = parse_u16_be(data, offset);
	return offset;
}

static uint32_t otf_parse_encoding_record(OTFEncodingRecord& record, const uint8_t* data, uint32_t offset)
{
	record.platformID     = parse_enum_be<OTFPlatformID>(data, offset);
	record.encodingID     = otf_parse_encoding_id(data, offset, record.platformID);
	record.subtableOffset = parse_u32_be(data, offset);
	return offset;
}

static uint32_t otf_parse_sequential_map_group(OTFSequentialMapGroup& group, const uint8_t* data, uint32_t offset)
{
	group.startCharCode = parse_u32_be(data, offset);
	group.endCharCode   = parse_u32_be(data, offset);
	group.startGlyphID  = parse_u32_be(data, offset);
	return offset;
}

static uint32_t otf_parse_long_hor_metric(OTFLongHorMetric& metric, const uint8_t* data, uint32_t offset)
{
	metric.advanceWidth    = parse_u16_be(data, offset);
	metric.leftSideBearing = parse_i16_be(data, offset);
	return offset;
}

static uint32_t otf_parse_name_table_header(OTFNAMETableHeader& header, const uint8_t* data, uint32_t offset)
{
	header.version       = parse_u16_be(data, offset);
	header.count         = parse_u16_be(data, offset);
	header.storageOffset = parse_u16_be(data, offset);
	return offset;
}

static uint32_t otf_parse_name_record(OTFNameRecord& record, const uint8_t* data, uint32_t offset)
{
	record.platformID   = parse_enum_be<OTFPlatformID>(data, offset);
	record.encodingID   = otf_parse_encoding_id(data, offset, record.platformID);
	record.languageID   = otf_parse_language_id(data, offset, record.platformID);
	record.nameID       = parse_enum_be<OTFNameID>(data, offset);
	record.length       = parse_u16_be(data, offset);
	record.stringOffset = parse_u16_be(data, offset);
	return offset;
}

static uint32_t otf_parse_post_table_header(OTFPOSTTableHeader& header, const uint8_t* data, uint32_t offset)
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

static uint32_t otf_parse_glyph_header(OTFGlyphHeader& header, const uint8_t* data, uint32_t offset)
{
	header.numberOfContours = parse_i16_be(data, offset);
	header.xMin             = parse_i16_be(data, offset);
	header.yMin             = parse_i16_be(data, offset);
	header.xMax             = parse_i16_be(data, offset);
	header.yMax             = parse_i16_be(data, offset);
	return offset;
}

static OTFResult otf_parse_head_table(OTFHEADTable& head_table, const uint8_t* data, uint32_t offset)
{
	head_table.version            = parse_u32_be(data, offset);
	head_table.fontRevision       = parse_u32_be(data, offset);
	head_table.checkSumAdjustment = parse_u32_be(data, offset);
	head_table.magicNumber        = parse_u32_be(data, offset);
	head_table.flags              = parse_u16_be(data, offset);
	head_table.unitsPerEm         = parse_u16_be(data, offset);
	head_table.created            = parse_u64_be(data, offset);
	head_table.modified           = parse_u64_be(data, offset);
	head_table.xMin               = parse_i16_be(data, offset);
	head_table.yMin               = parse_i16_be(data, offset);
	head_table.xMax               = parse_i16_be(data, offset);
	head_table.yMax               = parse_i16_be(data, offset);
	head_table.macStyle           = parse_u16_be(data, offset);
	head_table.lowestRecPPEM      = parse_u16_be(data, offset);
	head_table.fontDirectionHint  = parse_i16_be(data, offset);
	head_table.indexToLocFormat   = parse_enum_be<OTFIndexFormat>(data, offset);
	head_table.glyphDataFormat    = parse_i16_be(data, offset);

	if (head_table.magicNumber != 0x5f0f3cf5)
		return { OTFResultType::InvalidMagic, "invalid 'head' table magic number" };

	return OTFResultType::Success;
}

static OTFResult otf_parse_maxp_table(OTFMAXPTable& maxp_table, const uint8_t* data, uint32_t offset)
{
	maxp_table.version               = parse_u32_be(data, offset);
	maxp_table.numGlyphs             = parse_u16_be(data, offset);

	if (maxp_table.version == otf_make_version(0, 5))
		return OTFResultType::Success;

	maxp_table.maxPoints             = parse_u16_be(data, offset);
	maxp_table.maxContours           = parse_u16_be(data, offset);
	maxp_table.maxCompositePoints    = parse_u16_be(data, offset);
	maxp_table.maxCompositeContours  = parse_u16_be(data, offset);
	maxp_table.maxZones              = parse_u16_be(data, offset);
	maxp_table.maxTwilightPoints     = parse_u16_be(data, offset);
	maxp_table.maxStorage            = parse_u16_be(data, offset);
	maxp_table.maxFunctionDefs       = parse_u16_be(data, offset);
	maxp_table.maxInstructionDefs    = parse_u16_be(data, offset);
	maxp_table.maxStackElements      = parse_u16_be(data, offset);
	maxp_table.maxSizeOfInstructions = parse_u16_be(data, offset);
	maxp_table.maxComponentElements  = parse_u16_be(data, offset);
	maxp_table.maxComponentDepth     = parse_u16_be(data, offset);

	return OTFResultType::Success;
}

static void fill_glyph_map_segment(OTFCMAPTable& cmap_table, char32_t start_code, char32_t end_code, int16_t id_delta)
{
	for (char32_t i = start_code; i <= end_code; ++i) {
		auto glyph_idx = static_cast<OTFUint16>(i + id_delta);
		cmap_table.charToGlyphMap[i] = glyph_idx;
	}
}

static void fill_glyph_map_segment_with_id_range(
	OTFCMAPTable&  cmap_table,
	const uint8_t* data,
	char32_t       start_code,
	char32_t       end_code,
	uint32_t       address_offset,
	uint32_t       id_range_offset,
	int16_t        id_delta
) {
	for (char32_t i = start_code; i <= end_code; ++i) {
		uint32_t  idx_offset = id_range_offset + 2 * (i - start_code) + address_offset;
		OTFUint16 glyph_idx  = parse_u16_be(data, idx_offset);

		if (glyph_idx != 0)
			glyph_idx += id_delta;

		cmap_table.charToGlyphMap[i] = glyph_idx;
	}
}

static bool otf_supported_cmap_encoding_format4(OTFEncodingID encoding_id) {
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

static OTFResult otf_parse_cmap_format4(
	OTFCMAPTable&            cmap_table,
	const OTFEncodingRecord& encoding_record,
	const uint8_t*           data,
	uint32_t                 offset
) {
	OTFUint16     length         = parse_u16_be(data, offset);
	OTFLanguageID language       = otf_parse_language_id(data, offset, encoding_record.platformID);
	OTFUint16     seg_count      = parse_u16_be(data, offset) / 2;
/*  OTFUint16     search_range   = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
/*  OTFUint16     entry_selector = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
/*  OTFUint16     range_shift    = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
	
	uint32_t seg_count_size      = seg_count * sizeof(OTFUint16);
	uint32_t start_code_offset   = offset + seg_count_size + sizeof(uint16_t);
	uint32_t end_code_offset     = offset;
	uint32_t id_delta_offset     = start_code_offset + seg_count_size;
	uint32_t id_range_off_offset = id_delta_offset + seg_count_size;

	if (!otf_supported_cmap_encoding_format4(encoding_record.encodingID))
		return { OTFResultType::Unsupported, "unsupported cmap format4 encoding" };

	for (uint16_t i = 0; i < seg_count; ++i) {
		char32_t start_code      = static_cast<char32_t>(parse_u16_be(data, start_code_offset));
		char32_t end_code        = static_cast<char32_t>(parse_u16_be(data, end_code_offset));
		uint32_t id_range_offset = static_cast<uint32_t>(parse_u16_be(data, id_range_off_offset));
		int16_t  id_delta        = parse_i16_be(data, id_delta_offset);

		if (id_range_offset == 0) {
			fill_glyph_map_segment(cmap_table, start_code, end_code, id_delta);
		} else {
			fill_glyph_map_segment_with_id_range(
				cmap_table,
				data,
				start_code,
				end_code,
				offset,
				id_range_offset,
				id_delta
			);
		}
	}

	return OTFResultType::Success;
}

static OTFResult otf_parse_cmap_format6(
	OTFCMAPTable&            cmap_table,
	const OTFEncodingRecord& encoding_record,
	const uint8_t*           data,
	uint32_t                 offset
) {
	OTFUint16     length      = parse_u16_be(data, offset);
	OTFLanguageID language    = otf_parse_language_id(data, offset, encoding_record.platformID);
	OTFUint16     first_code  = parse_u16_be(data, offset);
	OTFUint16     entry_count = parse_u16_be(data, offset);

	for (uint16_t i = 0; i < entry_count; ++i) {
		char32_t  code     = static_cast<char32_t>(first_code + i);
		OTFUint16 glyph_id = parse_u16_be(data, offset);

		cmap_table.charToGlyphMap[code] = glyph_id;
	}

	return OTFResultType::Success;
}

static void otf_fill_glyph_map_sequential(OTFCMAPTable& cmap_table, const OTFSequentialMapGroup& group) {
	uint32_t code_count      = group.endCharCode - group.startCharCode + 1;
	uint32_t first_glyph_idx = group.startGlyphID;
	uint32_t last_glyph_idx  = first_glyph_idx + code_count;
	char32_t code            = static_cast<char32_t>(group.startCharCode);

	for (uint32_t i = first_glyph_idx; i < last_glyph_idx; ++i, ++ code)
		cmap_table.charToGlyphMap[code] = i;
}

static OTFResult otf_parse_cmap_format12(
	OTFCMAPTable&            cmap_table,
	const OTFEncodingRecord& encoding_record,
	const uint8_t*           data,
	uint32_t                 offset
) {
	OTFSequentialMapGroup map_group;

/*  OTFUint16 reserved   = parse_u16_be(data, offset); */ offset += sizeof(OTFUint16);
	OTFUint32 length     = parse_u32_be(data, offset);
	OTFUint32 language   = parse_u32_be(data, offset);
	OTFUint32 num_groups = parse_u32_be(data, offset);

	for (uint32_t i = 0; i < num_groups; ++i) {
		offset = otf_parse_sequential_map_group(map_group, data, offset);
		otf_fill_glyph_map_sequential(cmap_table, map_group);
	}

	return OTFResultType::Success;
}

static OTFResult otf_parse_cmap_table(OTFCMAPTable& cmap_table, const uint8_t* data, uint32_t offset)
{
	OTFCMAPTableHeader header;
	OTFEncodingRecord  encoding_record;
	
	uint32_t encoding_offset = otf_parse_cmap_table_header(header, data, offset);

	cmap_table.version	    = header.version;
	cmap_table.numSubtables = header.numSubtables;

	for (uint16_t i = 0; i < header.numSubtables; ++i) {
		encoding_offset = otf_parse_encoding_record(encoding_record, data, encoding_offset);

		uint32_t      subtable_offset = offset + encoding_record.subtableOffset;
		OTFCMAPFormat format          = parse_enum_be<OTFCMAPFormat>(data, subtable_offset);

		switch (format) {
		case OTFCMAPFormat::ByteEncodingTable:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::HighByteMappingThroughTable:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::SegmentMappingToDeltaValues:
			OTF_CHECK(otf_parse_cmap_format4(cmap_table, encoding_record, data, subtable_offset));
			break;
		case OTFCMAPFormat::TrimmedTableMapping:
			OTF_CHECK(otf_parse_cmap_format6(cmap_table, encoding_record, data, subtable_offset));
			break;
		case OTFCMAPFormat::Mixed16And32BitMapping:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::TrimmedArray:
			return { OTFResultType::Unsupported, "unsupported cmap subtable format" };
		case OTFCMAPFormat::SegmentedCoverage:
			OTF_CHECK(otf_parse_cmap_format12(cmap_table, encoding_record, data, subtable_offset));
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

static OTFResult otf_parse_hhea_table(OTFHHEATable& hhea_table, const uint8_t* data, uint32_t offset)
{
	hhea_table.majorVersion        = parse_u16_be(data, offset);
	hhea_table.minorVersion        = parse_u16_be(data, offset);
	hhea_table.ascender            = parse_i16_be(data, offset);
	hhea_table.descender           = parse_i16_be(data, offset);
	hhea_table.lineGap             = parse_i16_be(data, offset);
	hhea_table.advanceWidthMax     = parse_u16_be(data, offset);
	hhea_table.minLeftSideBearing  = parse_i16_be(data, offset);
	hhea_table.minRightSideBearing = parse_i16_be(data, offset);
	hhea_table.xMaxExtent          = parse_i16_be(data, offset);
	hhea_table.caretSlopeRise      = parse_i16_be(data, offset);
	hhea_table.caretSlopeRun       = parse_i16_be(data, offset);
	hhea_table.caretOffset         = parse_i16_be(data, offset);
	hhea_table.reserved0           = parse_i16_be(data, offset);
	hhea_table.reserved1           = parse_i16_be(data, offset);
	hhea_table.reserved2           = parse_i16_be(data, offset);
	hhea_table.reserved3           = parse_i16_be(data, offset);
	hhea_table.metricDataFormat    = parse_i16_be(data, offset);
	hhea_table.numberOfHMetrics    = parse_u16_be(data, offset);

	return OTFResultType::Success;
}

static OTFResult otf_parse_hmtx_table(
	OTFHMTXTable&       hmtx_table,
	const OTFMAXPTable& maxp_table,
	const OTFHHEATable& hhea_table,
	const uint8_t*      data,
	uint32_t            offset
) {
	OTFLongHorMetric metric;
	OTFFWORD         bearing;
	uint16_t         num_metrics = hhea_table.numberOfHMetrics;
	uint16_t         num_glyphs  = maxp_table.numGlyphs;
	
	for (uint16_t i = 0; i < num_metrics; ++i) {
		offset = otf_parse_long_hor_metric(metric, data, offset);
		hmtx_table.longHorMetrics.push_back(metric);
	}

	for (uint16_t i = num_metrics; i < num_glyphs; ++i) {
		bearing = parse_i16_be(data, offset);
		hmtx_table.leftSideBearings.push_back(bearing);
	}

	return OTFResultType::Success;
}

static OTFNameEntry* otf_find_name_entry(
	OTFNAMETable& name_table,
	OTFPlatformID platform_id,
	OTFEncodingID encoding_id,
	OTFLanguageID language_id
) {
	auto cmp_entry = OTFNameEntry{ platform_id, encoding_id, language_id, {} };
	auto it        = std::lower_bound(VERA_SPAN(name_table.names), cmp_entry, otf_compare_name_entry);

	return
		it != name_table.names.end()&&
		it->platformID == platform_id &&
		it->encodingID == encoding_id &&
		it->languageID == language_id ?
		&(*it) : nullptr;
}

static OTFResult otf_parse_name_table(OTFNAMETable& name_table, const uint8_t* data, uint32_t offset)
{
	OTFNAMETableHeader header;
	OTFNameRecord      name_record;
	std::string        str_buf;

	offset = otf_parse_name_table_header(header, data, offset);

	name_table.version       = header.version;
	name_table.count         = header.count;
	name_table.storageOffset = header.storageOffset;

	uint32_t storage_offset = offset + header.storageOffset;

	for (uint16_t i = 0; i < header.count; ++i) {
		offset = otf_parse_name_record(name_record, data, offset);

		if (name_record.nameID >= OTFNameID::__Count__)
			return { OTFResultType::InvalidID, "invalid name id" };

		uint32_t str_offset = storage_offset + name_record.stringOffset;

		str_buf.resize(name_record.length);
		memcpy(str_buf.data(), data + str_offset, str_buf.size());

		if (str_buf[0] == '\0')
			otf_decode_utf16be_to_utf8(str_buf);

		OTFNameEntry* name_entry = otf_find_name_entry_linear(
			name_table,
			name_record.platformID,
			name_record.encodingID,
			name_record.languageID);

		if (name_entry == nullptr) {
			OTFNameEntry& new_entry = name_table.names.emplace_back();
			new_entry.platformID = name_record.platformID;
			new_entry.encodingID = name_record.encodingID;
			new_entry.languageID = name_record.languageID;
			new_entry.names.resize(static_cast<size_t>(OTFNameID::__Count__));
			new_entry.names[static_cast<size_t>(name_record.nameID)] = str_buf;
		} else {
			name_entry->names[static_cast<size_t>(name_record.nameID)] = str_buf;
		}
	}

	if (name_table.version == 1) {
		OTFLangTagRecord lang_tag;
		OTFUint16 lang_tag_count = parse_u16_be(data, offset);

		for (uint16_t i = 0; i < lang_tag_count; ++i) {
			lang_tag.length        = parse_u16_be(data, offset);
			lang_tag.langTagOffset = parse_u16_be(data, offset);

			uint32_t str_offset = storage_offset + lang_tag.langTagOffset;

			str_buf.resize(lang_tag.length);
			memcpy(str_buf.data(), data + str_offset, str_buf.size());

			otf_decode_utf16be_to_utf8(str_buf);

			name_table.languageTags.push_back(str_buf);
		}
	}

	std::sort(VERA_SPAN(name_table.names), otf_compare_name_entry);

	return OTFResultType::Success;
}

static OTFResult otf_parse_os2_table(OTFOS2Table& os2_table, const uint8_t* data, uint32_t offset)
{
	os2_table.version             = parse_u16_be(data, offset);
	os2_table.xAvgCharWidth       = parse_i16_be(data, offset);
	os2_table.usWeightClass       = parse_u16_be(data, offset);
	os2_table.usWidthClass        = parse_u16_be(data, offset);
	os2_table.fsType              = parse_u16_be(data, offset);
	os2_table.ySubscriptXSize     = parse_i16_be(data, offset);
	os2_table.ySubscriptYSize     = parse_i16_be(data, offset);
	os2_table.ySubscriptXOffset   = parse_i16_be(data, offset);
	os2_table.ySubscriptYOffset   = parse_i16_be(data, offset);
	os2_table.ySuperscriptXSize   = parse_i16_be(data, offset);
	os2_table.ySuperscriptYSize   = parse_i16_be(data, offset);
	os2_table.ySuperscriptXOffset = parse_i16_be(data, offset);
	os2_table.ySuperscriptYOffset = parse_i16_be(data, offset);
	os2_table.yStrikeoutSize      = parse_i16_be(data, offset);
	os2_table.yStrikeoutPosition  = parse_i16_be(data, offset);
	os2_table.sFamilyClass        = parse_i16_be(data, offset);
	parse_array_be(os2_table.panose, 10, data, offset);
	os2_table.ulUnicodeRange1     = parse_u32_be(data, offset);
	os2_table.ulUnicodeRange2     = parse_u32_be(data, offset);
	os2_table.ulUnicodeRange3     = parse_u32_be(data, offset);
	os2_table.ulUnicodeRange4     = parse_u32_be(data, offset);
	parse_array_be(os2_table.achVendID, 4, data, offset);
	os2_table.fsSelection         = parse_u16_be(data, offset);
	os2_table.usFirstCharIndex    = parse_u16_be(data, offset);
	os2_table.usLastCharIndex     = parse_u16_be(data, offset);
	os2_table.sTypoAscender       = parse_i16_be(data, offset);
	os2_table.sTypoDescender      = parse_i16_be(data, offset);
	os2_table.sTypoLineGap        = parse_i16_be(data, offset);
	os2_table.usWinAscent         = parse_u16_be(data, offset);
	os2_table.usWinDescent        = parse_u16_be(data, offset);
	
	if (os2_table.version >= 1) {
		os2_table.ulCodePageRange1 = parse_u32_be(data, offset);
		os2_table.ulCodePageRange2 = parse_u32_be(data, offset);
	}
	
	if (os2_table.version >= 2) {
		os2_table.sxHeight      = parse_i16_be(data, offset);
		os2_table.sCapHeight    = parse_i16_be(data, offset);
		os2_table.usDefaultChar = parse_u16_be(data, offset);
		os2_table.usBreakChar   = parse_u16_be(data, offset);
		os2_table.usMaxContext  = parse_u16_be(data, offset);
	}
	
	if (os2_table.version >= 5) {
		os2_table.usLowerOpticalPointSize = parse_u16_be(data, offset);
		os2_table.usUpperOpticalPointSize = parse_u16_be(data, offset);
	}

	return OTFResultType::Success;
}

static const char* otf_get_macintosh_glyph_names(OTFUint16 glyph_name_idx)
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

static uint16_t otf_get_glyph_name_count(OTFUint16 num_glyphs, const uint8_t* data, uint32_t offset)
{
	OTFUint16 max_count = 0;

	for (uint16_t i = 0; i < num_glyphs; ++i) {
		OTFUint16 glyph_name_idx = parse_u16_be(data, offset);
		max_count = std::max<OTFUint16>(max_count, glyph_name_idx + 1);
	}

	return max_count;
}

static OTFResult otf_parse_post_table(OTFPOSTTable& post_table, const uint8_t* data, uint32_t offset)
{
	auto& header = reinterpret_cast<OTFPOSTTableHeader&>(post_table);

	offset = otf_parse_post_table_header(header, data, offset);

	if (otf_get_version_major(header.version) < 2)
		return OTFResultType::Success;

	std::vector<std::string> glyph_names;
	OTFUint16                num_glyphs    = parse_u16_be(data, offset);
	uint16_t                 string_count  = otf_get_glyph_name_count(num_glyphs, data, offset);
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
			const char* glyph_name = otf_get_macintosh_glyph_names(glyph_name_idx);
			post_table.glyphNames.push_back(glyph_name);
		} else {
			std::string& glyph_name = glyph_names[glyph_name_idx - 258];
			post_table.glyphNames.push_back(glyph_name);
		}
	}

	return OTFResultType::Success;
}

static OTFResult otf_parse_cvt_table(
	std::vector<OTFFWORD>& control_values,
	const uint8_t*         data,
	const OTFTableRecord&  table_record
) {
	if (table_record.length % sizeof(OTFUint16) != 0)
		return { OTFResultType::InvalidSize, "invalid 'cvt' table size" };

	uint32_t offset = table_record.offset;
	uint32_t count  = table_record.length / sizeof(OTFUint16);

	control_values.resize(count);

	for (uint32_t i = 0; i < count; ++i)
		control_values[i] = parse_u16_be(data, offset);

	return OTFResultType::Success;
}

static OTFResult otf_parse_fpgm_table(
	std::vector<uint8_t>&  program_data,
	const uint8_t*         data,
	const OTFTableRecord&  table_record
) {
	uint32_t offset = table_record.offset;
	uint32_t count  = table_record.length;

	program_data.resize(count);
	memcpy(program_data.data(), data + offset, count);

	return OTFResultType::Success;
}

static OTFResult otf_parse_loca_table(
	std::vector<uint32_t>& glyph_offsets,
	const OTFHEADTable&    head_table,
	const OTFMAXPTable&    maxp_table,
	const uint8_t*         data,
	const OTFTableRecord&  table_record
) {
	uint32_t offset = table_record.offset;
	uint32_t count  = maxp_table.numGlyphs + 1;

	glyph_offsets.resize(count);

	if (head_table.indexToLocFormat == OTFIndexFormat::ShortOffsets) {
		for (uint32_t i = 0; i < count; ++i)
			glyph_offsets[i] = parse_u16_be(data, offset) * 2;
	} else if (head_table.indexToLocFormat == OTFIndexFormat::LongOffsets) {
		for (uint32_t i = 0; i < count; ++i)
			glyph_offsets[i] = parse_u32_be(data, offset);
	} else {
		return { OTFResultType::InvalidFormat, "invalid 'loca' table index format" };
	}

	return OTFResultType::Success;
}

void otf_make_vector_parser(
	OTFGlyphFlagBits flags,
	OTFVectorParser& x_vector_func,
	OTFVectorParser& y_vector_func
) {
	static const auto short_positive_vector = [](const uint8_t* data, uint32_t& offset) {
		return static_cast<float>(parse_u8_be(data, offset));
	};

	static const auto short_negative_vector = [](const uint8_t* data, uint32_t& offset) {
		return -static_cast<float>(parse_u8_be(data, offset));
	};

	static const auto long_vector = [](const uint8_t* data, uint32_t& offset) {
		return static_cast<float>(parse_i16_be(data, offset));
	};

	static const auto zero_vector = [](const uint8_t* data, uint32_t& offset) {
		return 0.f;
	};

	if (otf_has_flag(flags, OTFGlyphFlagBits::XShortVector)) {
		if (otf_has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive))
			x_vector_func = short_positive_vector;
		else
			x_vector_func = short_negative_vector;
	} else {
		if (otf_has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive))
			x_vector_func = zero_vector;
		else
			x_vector_func = long_vector;
	}

	if (otf_has_flag(flags, OTFGlyphFlagBits::YShortVector)) {
		if (otf_has_flag(flags, OTFGlyphFlagBits::YIsSameOrPositive))
			y_vector_func = short_positive_vector;
		else
			y_vector_func = short_negative_vector;
	} else {
		if (otf_has_flag(flags, OTFGlyphFlagBits::YIsSameOrPositive))
			y_vector_func = zero_vector;
		else
			y_vector_func = long_vector;
	}
}

static void otf_get_simple_glyph_point_offset(
	const uint8_t* data,
	uint32_t       offset,
	uint32_t       point_count,
	uint32_t&      x_point_offset,
	uint32_t&      y_point_offset
) {
	OTFGlyphFlagBits flags;
	uint32_t         repeat_count;
	uint32_t         x_point_size = 0;

	while (0 < point_count) {
		flags = parse_enum_be<OTFGlyphFlagBits>(data, offset);

		if (otf_has_flag(flags, OTFGlyphFlagBits::RepeatFlag)) {
			repeat_count = static_cast<uint32_t>(parse_u8_be(data, offset) + 1);

			point_count -= repeat_count;

			if (otf_has_flag(flags, OTFGlyphFlagBits::XShortVector))
				x_point_size += repeat_count;
			else if (!otf_has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive))
				x_point_size += repeat_count * sizeof(OTFInt16);
		} else {
			point_count -= 1;

			if (otf_has_flag(flags, OTFGlyphFlagBits::XShortVector))
				x_point_size += 1;
			else if (!otf_has_flag(flags, OTFGlyphFlagBits::XIsSameOrPositive))
				x_point_size += sizeof(OTFInt16);
		}
	}

	x_point_offset = offset;
	y_point_offset = x_point_offset + x_point_size;
}

static OTFResult otf_parse_simple_glyph(
	OTFGlyph&               glyph,
	uint32_t                glyph_idx,
	const OTFGlyphHeader&   glyph_header,
	std::vector<OTFUint16>& end_indicies,
	const uint8_t*          data,
	uint32_t                offset
) {
	uint32_t num_contours = glyph_header.numberOfContours;

	glyph.contours.resize(num_contours);

	end_indicies.resize(num_contours);
	parse_array_be(end_indicies.data(), num_contours, data, offset);

	OTFUint16 inst_length = parse_u16_be(data, offset);

	glyph.instructions.resize(inst_length);
	parse_array_be(glyph.instructions.data(), inst_length, data, offset);

	uint32_t x_point_offset;
	uint32_t y_point_offset;
	uint32_t point_count  = end_indicies.back() + 1;

	otf_get_simple_glyph_point_offset(
		data,
		offset,
		point_count,
		x_point_offset,
		y_point_offset);

	float2   p0;
	float2   p1;
	float2   pc0;
	float2   pc1;
	auto     curr_path    = glyph.contours.begin();
	auto     curr_end     = end_indicies.begin();
	uint32_t point_comp   = 0;
	uint32_t repeat_count = 0;
	float    curr_x       = 0.f;
	float    curr_y       = 0.f;
	bool     is_control   = false;

	OTFVectorParser x_vector_func = nullptr;
	OTFVectorParser y_vector_func = nullptr;

	for (uint32_t i = 0; i < point_count; ++i) {
		if (repeat_count == 0) {
			auto flags = parse_enum_be<OTFGlyphFlagBits>(data, offset);

			if (otf_has_flag(flags, OTFGlyphFlagBits::RepeatFlag))
				repeat_count = parse_u8_be(data, offset) + 1;
			else
				repeat_count = 1;

			is_control = !otf_has_flag(flags, OTFGlyphFlagBits::OnCurvePoint);

			otf_make_vector_parser(flags, x_vector_func, y_vector_func);
		}
		
		curr_x += x_vector_func(data, x_point_offset);
		curr_y += y_vector_func(data, y_point_offset);

		switch (point_comp) {
		case 0:
			if (is_control) {
				return { OTFResultType::InvalidFormat, "invalid simple glyph point format" };
			} else {
				p0         = float2{ curr_x, curr_y };
				point_comp = 1;
			}
			break;
		case 1:
			if (is_control) {
				pc0        = float2{ curr_x, curr_y };
				point_comp = 2;
			} else {
				p1 = float2{ curr_x, curr_y };
				curr_path->append(p0, p1);
				p0 = p1;
			}
			break;
		case 2:
			if (is_control) {
				pc1 = float2{ curr_x, curr_y };
				p1  = 0.5f * (pc0 + pc1);
				curr_path->append(p0, pc0, p1);
				p0  = p1;
				pc0 = pc1;
			} else {
				p1 = float2{ curr_x, curr_y };
				curr_path->append(p0, pc0, p1);
				p0         = p1;
				point_comp = 1;
			}
			break;
		}

		if (i == *curr_end) {
			point_comp = 0;
			curr_x     = 0.f;
			curr_y     = 0.f;
			++curr_path;
			++curr_end;
		}

		--repeat_count;
	}

	glyph.glyphID = glyph_idx;

	return OTFResultType::Success;
}

static OTFResult otf_parse_composite_glyph(
	OTFGlyph&               glyph,
	const OTFFont&          font,
	uint32_t                glyph_idx,
	const OTFGlyphHeader&   glyph_header,
	const uint8_t*          data,
	uint32_t                offset
) {
	OTFComponentGlyphFlagBits flags;
	OTFGlyphID                glyph_id;

	float      transform[6];
	float      arg1;
	float      arg2;
	float      x_scale;
	float      y_scale;

	do {
		flags    = parse_enum_be<OTFComponentGlyphFlagBits>(data, offset);
		glyph_id = parse_u16_be(data, offset);

		if (otf_has_flag(flags, OTFComponentGlyphFlagBits::Arg1And2AreWords)) {
			if (otf_has_flag(flags, OTFComponentGlyphFlagBits::ArgsAreXYValues)) {
				arg1 = static_cast<float>(parse_i16_be(data, offset));
				arg2 = static_cast<float>(parse_i16_be(data, offset));
			} else {
				arg1 = static_cast<float>(parse_u16_be(data, offset));
				arg2 = static_cast<float>(parse_u16_be(data, offset));
			}
		} else {
			if (otf_has_flag(flags, OTFComponentGlyphFlagBits::ArgsAreXYValues)) {
				arg1 = static_cast<float>(parse_i8_be(data, offset));
				arg2 = static_cast<float>(parse_i8_be(data, offset));
			} else {
				arg1 = static_cast<float>(parse_u8_be(data, offset));
				arg2 = static_cast<float>(parse_u8_be(data, offset));
			}
		}

		if (otf_has_flag(flags, OTFComponentGlyphFlagBits::WeHaveAScale)) {
			float scale = otf_f2dot14_to_float(parse_i16_be(data, offset));

			transform[0] = scale;
			transform[1] = 0.f;
			transform[2] = 0.f;
			transform[3] = scale;
		} else if (otf_has_flag(flags, OTFComponentGlyphFlagBits::WeHaveAnXAndYScale)) {
			float x_scale = otf_f2dot14_to_float(parse_i16_be(data, offset));
			float y_scale = otf_f2dot14_to_float(parse_i16_be(data, offset));

			transform[0] = x_scale;
			transform[1] = 0.f;
			transform[2] = 0.f;
			transform[3] = y_scale;
		} else if (otf_has_flag(flags, OTFComponentGlyphFlagBits::WeHaveATwoByTwo)) {
			transform[0] = otf_f2dot14_to_float(parse_i16_be(data, offset));
			transform[1] = otf_f2dot14_to_float(parse_i16_be(data, offset));
			transform[2] = otf_f2dot14_to_float(parse_i16_be(data, offset));
			transform[3] = otf_f2dot14_to_float(parse_i16_be(data, offset));
		}

		if (otf_has_flag(flags, OTFComponentGlyphFlagBits::ArgsAreXYValues)) {
			transform[4] = arg1;
			transform[5] = arg2;

			if (otf_has_flag(flags, OTFComponentGlyphFlagBits::ScaledComponentOffset)) {
				transform[4] *= transform[0];
				transform[5] *= transform[3];
			}
		}

		if (otf_has_flag(flags, OTFComponentGlyphFlagBits::WeHaveInstructions)) {
			OTFUint16 inst_length = parse_u16_be(data, offset);
			glyph.instructions.resize(inst_length);
			parse_array_be(glyph.instructions.data(), inst_length, data, offset);
		}

		if (font.glyphs.size() <= glyph_id)
			return { OTFResultType::InvalidIndex, "invalid composite glyph component index" };

		const OTFGlyph& composite_glyph = font.glyphs[glyph_id];

		if (composite_glyph.contours.empty())
			return { OTFResultType::InvalidFormat, "invalid composite glyph component format" };

		auto transform_point = [&](const float2& p) {
			return float2{
				transform[0] * p.x + transform[1] * p.y + transform[4],
				transform[2] * p.x + transform[3] * p.y + transform[5]
			};
		};

		// TODO: optimize transform

		glyph.contours.reserve(glyph.contours.size() + composite_glyph.contours.size());

		for (const auto& contour : composite_glyph.contours) {
			auto&  curr_path = glyph.contours.emplace_back();
			float2 p0        = transform_point(contour[0].p0());

			curr_path.reserve(contour.size());

			for (const auto& curve : contour) {
				float2 c  = transform_point(curve.controlPoint());
				float2 p1 = transform_point(curve.p1());

				curr_path.append(p0, c, p1);

				p0 = p1;
			}
		}

	} while (otf_has_flag(flags, OTFComponentGlyphFlagBits::MoreComponents));

	return OTFResultType::Success;
}

static OTFResult otf_parse_glyf_table(
	OTFFont&                     font,
	const std::vector<uint32_t>& glyph_offsets,
	const uint8_t*               data,
	const OTFTableRecord&        table_record
) {
	std::vector<OTFUint16> end_indicies;
	OTFGlyphHeader         glyph_header;

	if (glyph_offsets.empty())
		return { OTFResultType::MissingTable, "empty glyph offsets, missing 'loca' table" };

	uint16_t num_glyph    = font.maxProfile.numGlyphs;
	uint32_t start_offset = table_record.offset;

	font.glyphs.resize(num_glyph);

	for (uint16_t glyph_idx = 0; glyph_idx < num_glyph; ++glyph_idx) {
		uint32_t glyph_offset = start_offset + glyph_offsets[glyph_idx];

		glyph_offset = otf_parse_glyph_header(glyph_header, data, glyph_offset);

		if (glyph_header.numberOfContours >= 0) {
			OTF_CHECK(otf_parse_simple_glyph(
				font.glyphs[glyph_idx],
				glyph_idx,
				glyph_header,
				end_indicies,
				data,
				glyph_offset));
		} else {
			OTF_CHECK(otf_parse_composite_glyph(
				font.glyphs[glyph_idx],
				font,
				glyph_idx,
				glyph_header,
				data,
				glyph_offset));
		}
	}

	return OTFResultType::Success;
}

static OTFResult otf_parse_table(
	OTFFont&                                        font,
	std::unordered_map<OTFTableTag, OTFTableRecord> table_map,
	const uint8_t*                                  data
) {
	if (auto it = table_map.find(OTFTableTag::HEAD); it != table_map.cend()) {
		OTF_CHECK(otf_parse_head_table(font.head, data, it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'head' table" };
	}

	if (auto it = table_map.find(OTFTableTag::MAXP); it != table_map.cend()) {
		OTF_CHECK(otf_parse_maxp_table(font.maxProfile, data, it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'maxp' table" };
	}

	if (auto it = table_map.find(OTFTableTag::CMAP); it != table_map.cend()) {
		OTF_CHECK(otf_parse_cmap_table(font.characterMap, data, it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'cmap' table" };
	}

	if (auto it = table_map.find(OTFTableTag::HHEA); it != table_map.cend()) {
		OTF_CHECK(otf_parse_hhea_table(font.horizontalHeader, data, it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'hhea' table" };
	}

	if (auto it = table_map.find(OTFTableTag::HMTX); it != table_map.cend()) {
		OTF_CHECK(otf_parse_hmtx_table(
			font.horizontalMetrics,
			font.maxProfile,
			font.horizontalHeader,
			data,
			it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'hmtx' table" };
	}

	if (auto it = table_map.find(OTFTableTag::NAME); it != table_map.cend()) {
		OTF_CHECK(otf_parse_name_table(font.nameTable, data, it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'name' table" };
	}

	if (auto it = table_map.find(OTFTableTag::OS2); it != table_map.cend()) {
		OTF_CHECK(otf_parse_os2_table(font.os2Metrics, data, it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'os/2' table" };
	}

	if (auto it = table_map.find(OTFTableTag::POST); it != table_map.cend()) {
		OTF_CHECK(otf_parse_post_table(font.postScript, data, it->second.offset));
	} else {
		return { OTFResultType::MissingTable, "missing 'post' table" };
	}

	if (auto it = table_map.find(OTFTableTag::CVT); it != table_map.cend())
		OTF_CHECK(otf_parse_cvt_table(font.controlValues, data, it->second));

	if (auto it = table_map.find(OTFTableTag::FPGM); it != table_map.cend())
		OTF_CHECK(otf_parse_fpgm_table(font.programData, data, it->second));

	std::vector<uint32_t> glyph_offsets;
	if (auto it = table_map.find(OTFTableTag::LOCA); it != table_map.cend())
		OTF_CHECK(otf_parse_loca_table(glyph_offsets, font.head, font.maxProfile, data, it->second));

	if (auto it = table_map.find(OTFTableTag::GLYF); it != table_map.cend())
		OTF_CHECK(otf_parse_glyf_table(font, glyph_offsets, data, it->second));

	return OTFResultType::Success;
}

static OTFResult otf_load_font(OTFFont& font, const uint8_t* data, uint32_t offset)
{
	TTFHeader      header;
	OTFTableRecord record;

	offset = otf_parse_ttf_header(header, data, offset);

	std::unordered_map<OTFTableTag, OTFTableRecord> table_map;
	for (uint16_t i = 0; i < header.numTables; ++i) {
		offset = otf_parse_table_record(record, data, offset);
		table_map[record.tableTag] = record;
	}

	OTF_CHECK(otf_parse_table(font, table_map, data));

	return OTFResultType::Success;
}

VERA_PRIV_NAMESPACE_END

static OTFResult otf_load_ttc(OTFFontCollection& font_collection, const uint8_t* data, const size_t size)
{
	TTCHeader header;
	uint32_t  offset = 0;
	
	offset = priv::otf_parse_ttc_header(header, data, offset);

	if (header.magic != OTFTTCTag::Value)
		return { OTFResultType::InvalidMagic, "invalid TTC header magic" };

	font_collection.fonts.resize(header.numFonts);

	for (uint32_t i = 0; i < header.numFonts; ++i) {
		uint32_t font_offset = parse_u32_be(data, offset);
		auto&    font        = font_collection.fonts[i];

		OTF_CHECK(priv::otf_load_font(font, data, font_offset));
	}

	return OTFResultType::Success;
}

static OTFResult otf_load_ttf(OTFFont& font, const uint8_t* data, const size_t size)
{
	return priv::otf_load_font(font, data, 0);
}

VERA_NAMESPACE_END