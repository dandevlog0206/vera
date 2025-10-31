#pragma once

#include "../core/coredefs.h"
#include <vector>

VERA_NAMESPACE_BEGIN

enum class UnicodeRange VERA_ENUM64
{
	Unknown                              = 0x0000000000000000,
	BasicLatin                           = 0x000000000000007F,
	Latin1Supplement                     = 0x000000A0000000FF,
	LatinExtendedA                       = 0x000001000000017F,
	LatinExtendedB                       = 0x000001800000024F,
	IPAExtensions                        = 0x00000250000002AF,
	SpacingModifierLetters               = 0x000002B0000002FF,
	CombiningDiacriticalMarks            = 0x000003000000036F,
	GreekAndCoptic                       = 0x00000370000003FF,
	Cyrillic                             = 0x00000400000004FF,
	CyrillicSupplement                   = 0x000005000000052F,
	Armenian                             = 0x000005300000058F,
	Hebrew                               = 0x00000590000005FF,
	Arabic                               = 0x00000600000006FF,
	Syriac                               = 0x000007000000074F,
	Thaana                               = 0x00000780000007BF,
	Devanagari                           = 0x000009000000097F,
	Bengali                              = 0x00000980000009FF,
	Gurmukhi                             = 0x00000A0000000A7F,
	Gujarati                             = 0x00000A8000000AFF,
	Oriya                                = 0x00000B0000000B7F,
	Tamil                                = 0x00000B8000000BFF,
	Telugu                               = 0x00000C0000000C7F,
	Kannada                              = 0x00000C8000000CFF,
	Malayalam                            = 0x00000D0000000D7F,
	Sinhala                              = 0x00000D8000000DFF,
	Thai                                 = 0x00000E0000000E7F,
	Lao                                  = 0x00000E8000000EFF,
	Tibetan                              = 0x00000F0000000FFF,
	Myanmar                              = 0x000010000000109F,
	Georgian                             = 0x000010A0000010FF,
	HangulJamo                           = 0x00001100000011FF,
	Ethiopic                             = 0x000012000000137F,
	Cherokee                             = 0x000013A0000013FF,
	UnifiedCanadianAboriginalSyllabics   = 0x000014000000167F,
	Ogham                                = 0x000016800000169F,
	Runic                                = 0x000016A0000016FF,
	Tagalog                              = 0x000017000000171F,
	Hanunoo                              = 0x000017200000173F,
	Buhid                                = 0x000017400000175F,
	Tagbanwa                             = 0x000017600000177F,
	Khmer                                = 0x00001780000017FF,
	Mongolian                            = 0x00001800000018AF,
	Limbu                                = 0x000019000000194F,
	TaiLe                                = 0x000019500000197F,
	KhmerSymbols                         = 0x000019E0000019FF,
	PhoneticExtensions                   = 0x00001D0000001D7F,
	LatinExtendedAdditional              = 0x00001E0000001EFF,
	GreekExtended                        = 0x00001F0000001FFF,
	GeneralPunctuation                   = 0x000020000000206F,
	SuperscriptsAndSubscripts            = 0x000020700000209F,
	CurrencySymbols                      = 0x000020A0000020CF,
	CombiningDiacriticalMarksForSymbols  = 0x000020D0000020FF,
	LetterlikeSymbols                    = 0x000021000000214F,
	NumberForms                          = 0x000021500000218F,
	Arrows                               = 0x00002190000021FF,
	MathematicalOperators                = 0x00002200000022FF,
	MiscellaneousTechnical               = 0x00002300000023FF,
	ControlPictures                      = 0x000024000000243F,
	OpticalCharacterRecognition          = 0x000024400000245F,
	EnclosedAlphanumerics                = 0x00002460000024FF,
	BoxDrawing                           = 0x000025000000257F,
	BlockElements                        = 0x000025800000259F,
	GeometricShapes                      = 0x000025A0000025FF,
	MiscellaneousSymbols                 = 0x00002600000026FF,
	Dingbats                             = 0x00002700000027BF,
	MiscellaneousMathematicalSymbolsA    = 0x000027C0000027EF,
	SupplementalArrowsA                  = 0x000027F0000027FF,
	BraillePatterns                      = 0x00002800000028FF,
	SupplementalArrowsB                  = 0x000029000000297F,
	MiscellaneousMathematicalSymbolsB    = 0x00002980000029FF,
	SupplementalMathematicalOperators    = 0x00002A0000002AFF,
	MiscellaneousSymbolsAndArrows        = 0x00002B0000002BFF,
	CJKRadicalsSupplement                = 0x00002E8000002EFF,
	KangxiRadicals                       = 0x00002F0000002FDF,
	IdeographicDescriptionCharacters     = 0x00002FF000002FFF,
	CJKSymbolsAndPunctuation             = 0x000030000000303F,
	Hiragana                             = 0x000030400000309F,
	Katakana                             = 0x000030A0000030FF,
	Bopomofo                             = 0x000031000000312F,
	HangulCompatibilityJamo              = 0x000031300000318F,
	Kanbun                               = 0x000031900000319F,
	BopomofoExtended                     = 0x000031A0000031BF,
	KatakanaPhoneticExtensions           = 0x000031F0000031FF,
	EnclosedCJKLettersAndMonths          = 0x00003200000032FF,
	CJKCompatibility                     = 0x00003300000033FF,
	CJKUnifiedIdeographsExtensionA       = 0x0000340000004DBF,
	YijingHexagramSymbols                = 0x00004DC000004DFF,
	CJKUnifiedIdeographs                 = 0x00004E0000009FFF,
	YiSyllables                          = 0x0000A0000000A48F,
	YiRadicals                           = 0x0000A4900000A4CF,
	HangulSyllables                      = 0x0000AC000000D7AF,
	HighSurrogates                       = 0x0000D8000000D87F,
	HighPrivateUseSurrogates             = 0x0000DB800000DFFF,
	LowSurrogates                        = 0x0000DC000000DFFF,
	PrivateUseArea                       = 0x0000E0000000F8FF,
	CJKCompatibilityIdeographs           = 0x0000F9000000FAFF,
	AlphabeticPresentationForms          = 0x0000FB000000FB4F,
	ArabicPresentationFormsA             = 0x0000FB500000FDFF,
	VariationSelectors                   = 0x0000FE000000FE0F,
	CombiningHalfMarks                   = 0x0000FE200000FE2F,
	CJKCompatibilityForms                = 0x0000FE300000FE4F,
	SmallFormVariants                    = 0x0000FE500000FE6F,
	ArabicPresentationFormsB             = 0x0000FE700000FEFF,
	HalfwidthAndFullwidthForms           = 0x0000FF000000FFE0,
	Specials                             = 0x0000FFF00000FFFF,
	LinearBSyllabary                     = 0x000100000001007F,
	LinearBIdeograms                     = 0x00010080000100FF,
	AegeanNumbers                        = 0x000101000001013F,
	OldItalic                            = 0x000103000001032F,
	Gothic                               = 0x000103300001034F,
	Ugaritic                             = 0x000103800001039F,
	Deseret                              = 0x000104000001044F,
	Shavian                              = 0x000104500001047F,
	Osmanya                              = 0x00010480000104AF,
	CypriotSyllabary                     = 0x000108000001083F,
	ByzantineMusicalSymbols              = 0x0001D0000001D0FF,
	MusicalSymbols                       = 0x0001D1000001D1FF,
	TaiXuanJingSymbols                   = 0x0001D3000001D35F,
	MathematicalAlphanumericSymbols      = 0x0001D4000001D7FF,
	CJKUnifiedIdeographsExtensionB       = 0x000200000002A6DF,
	CJKCompatibilityIdeographsSupplement = 0x0002F8000002FA1F,
	Tags                                 = 0x000E0000000E007F,
	ALL                                  = 0x000000000010FFFF
};

class CodeRange
{
public:
	VERA_CONSTEXPR CodeRange() VERA_NOEXCEPT :
		m_start(0),
		m_end(0),
		m_range(UnicodeRange::Unknown) {}

	VERA_CONSTEXPR CodeRange(char32_t codepoint) VERA_NOEXCEPT :
		m_start(codepoint),
		m_end(codepoint),
		m_range(UnicodeRange::Unknown) {}

	VERA_CONSTEXPR CodeRange(char32_t start, char32_t end) VERA_NOEXCEPT :
		m_start(start),
		m_end(end),
		m_range(UnicodeRange::Unknown) {}

	VERA_CONSTEXPR CodeRange(UnicodeRange range) VERA_NOEXCEPT :
		m_start(static_cast<char32_t>(static_cast<uint64_t>(range) >> 32)),
		m_end(static_cast<char32_t>(static_cast<uint64_t>(range) & 0xFFFFFFFF)),
		m_range(range) {}

	VERA_NODISCARD VERA_CONSTEXPR char32_t start() const VERA_NOEXCEPT
	{
		return m_start;
	}

	VERA_NODISCARD VERA_CONSTEXPR char32_t end() const VERA_NOEXCEPT
	{
		return m_end;
	}

	VERA_NODISCARD VERA_CONSTEXPR UnicodeRange getUnicodeRange() const VERA_NOEXCEPT
	{
		return m_range;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contains(char32_t code_point) const VERA_NOEXCEPT
	{
		return m_start <= code_point && code_point <= m_end;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contains(const CodeRange& other) const VERA_NOEXCEPT
	{
		return m_start <= other.m_start && other.m_end <= m_end;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_start == 0 && m_end == 0;
	}

private:
	// Start and end code points (inclusive)
	char32_t     m_start;
	char32_t     m_end;
	UnicodeRange m_range;
};

VERA_NAMESPACE_END
