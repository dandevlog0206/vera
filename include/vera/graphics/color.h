#pragma once

#include "format.h"
#include "../math/vector_types.h"
#include <bit>

VERA_NAMESPACE_BEGIN

// color values in RGBA format
enum class Colors : uint32_t {
	TransparentBlack     = 0x00000000, // #000000
	TransparentWhite     = 0x00FFFFFF, // #FFFFFF
	AliceBlue            = 0xFFFFF8F0, // #F0F8FF
	AntiqueWhite         = 0xFFD7EBFA, // #FAEBD7
	Aqua                 = 0xFFFFFF00, // #00FFFF
	Aquamarine           = 0xFFD4FF7F, // #7FFFD4
	Azure                = 0xFFFFFFF0, // #F0FFFF
	Beige                = 0xFFDCF5F5, // #F5F5DC
	Bisque               = 0xFFC4E4FF, // #FFE4C4
	Black                = 0xFF000000, // #000000
	BlanchedAlmond       = 0xFFCDEBFF, // #FFEBCD
	Blue                 = 0xFFFF0000, // #0000FF
	BlueViolet           = 0xFFE22B8A, // #8A2BE2
	Brown                = 0xFF2A2AA5, // #A52A2A
	BurlyWood            = 0xFF87B8DE, // #DEB887
	CadetBlue            = 0xFFA09E5F, // #5F9EA0
	Chartreuse           = 0xFF00FF7F, // #7FFF00
	Chocolate            = 0xFF1E69D2, // #D2691E
	Coral                = 0xFF507FFF, // #FF7F50
	CornflowerBlue       = 0xFFED9564, // #6495ED
	Cornsilk             = 0xFFDCF8FF, // #FFF8DC
	Crimson              = 0xFF3C14DC, // #DC143C
	Cyan                 = 0xFFFFFF00, // #00FFFF
	DarkBlue             = 0xFF8B0000, // #00008B
	DarkCyan             = 0xFF8B8B00, // #008B8B
	DarkGoldenrod        = 0xFF0B86B8, // #B8860B
	DarkGray             = 0xFFA9A9A9, // #A9A9A9
	DarkGreen            = 0xFF006400, // #006400
	DarkKhaki            = 0xFF6BB7BD, // #BDB76B
	DarkMagenta          = 0xFF8B008B, // #8B008B
	DarkOliveGreen       = 0xFF2F6B55, // #556B2F
	DarkOrange           = 0xFF008CFF, // #FF8C00
	DarkOrchid           = 0xFFCC3299, // #9932CC
	DarkRed              = 0xFF00008B, // #8B0000
	DarkSalmon           = 0xFF7A96E9, // #E9967A
	DarkSeaGreen         = 0xFF8FBC8F, // #8FBC8F
	DarkSlateBlue        = 0xFF8B3D48, // #483D8B
	DarkSlateGray        = 0xFF4F4F2F, // #2F4F4F
	DarkTurquoise        = 0xFFD1CE00, // #00CED1
	DarkViolet           = 0xFFD30094, // #9400D3
	DeepPink             = 0xFF9314FF, // #FF1493
	DeepSkyBlue          = 0xFFFFBF00, // #00BFFF
	DimGray              = 0xFF696969, // #696969
	DodgerBlue           = 0xFFFF901E, // #1E90FF
	Firebrick            = 0xFF2222B2, // #B22222
	FloralWhite          = 0xFFF0FAFF, // #FFFAF0
	ForestGreen          = 0xFF228B22, // #228B22
	Fuchsia              = 0xFFFF00FF, // #FF00FF
	Gainsboro            = 0xFFDCDCDC, // #DCDCDC
	GhostWhite           = 0xFFFFF8F8, // #F8F8FF
	Gold                 = 0xFF00D7FF, // #FFD700
	Goldenrod            = 0xFF20A5DA, // #DAA520
	Gray                 = 0xFF808080, // #808080
	Green                = 0xFF008000, // #008000
	GreenYellow          = 0xFF2FFFAD, // #ADFF2F
	honeydew             = 0xFFF0FFF0, // #F0FFF0
	hotPink              = 0xFFB469FF, // #FF69B4
	IndianRed            = 0xFF5C5CCD, // #CD5C5C
	Indigo               = 0xFF82004B, // #4B0082
	Ivory                = 0xFFF0FFFF, // #FFFFF0
	Khaki                = 0xFF8CE6F0, // #F0E68C
	Lavender             = 0xFFFAE6E6, // #E6E6FA
	LavenderBlush        = 0xFFF5F0FF, // #FFF0F5
	LawnGreen            = 0xFF00FC7C, // #7CFC00
	LemonChiffon         = 0xFFCDFAFF, // #FFFACD
	LightBlue            = 0xFFE6D8AD, // #ADD8E6
	LightCoral           = 0xFF8080F0, // #F08080
	LightCyan            = 0xFFFFFFE0, // #E0FFFF
	LightGoldenrodYellow = 0xFFD2FAFA, // #FAFAD2
	LightGray            = 0xFFD3D3D3, // #D3D3D3
	LightGreen           = 0xFF90EE90, // #90EE90
	LightPink            = 0xFFC1B6FF, // #FFB6C1
	LightSalmon          = 0xFF7AA0FF, // #FFA07A
	LightSeaGreen        = 0xFFAAB220, // #20B2AA
	LightSkyBlue         = 0xFFFACE87, // #87CEFA
	LightSlateGray       = 0xFF998877, // #778899
	LightSteelBlue       = 0xFFDEC4B0, // #B0C4DE
	LightYellow          = 0xFFE0FFFF, // #FFFFE0
	Lime                 = 0xFF00FF00, // #00FF00
	LimeGreen            = 0xFF32CD32, // #32CD32
	Linen                = 0xFFE6F0FA, // #FAF0E6
	Magenta              = 0xFFFF00FF, // #FF00FF
	Maroon               = 0xFF000080, // #800000
	MediumAquamarine     = 0xFFAACD66, // #66CDAA
	MediumBlue           = 0xFFCD0000, // #0000CD
	MediumOrchid         = 0xFFD355BA, // #BA55D3
	MediumPurple         = 0xFFDB7093, // #9370DB
	MediumSeaGreen       = 0xFF71B33C, // #3CB371
	MediumSlateBlue      = 0xFFEE687B, // #7B68EE
	MediumSpringGreen    = 0xFF9AFA00, // #00FA9A
	MediumTurquoise      = 0xFFCCD148, // #48D1CC
	MediumVioletRed      = 0xFF8515C7, // #C71585
	MidnightBlue         = 0xFF701919, // #191970
	MintCream            = 0xFFFAFFF5, // #F5FFFA
	MistyRose            = 0xFFE1E4FF, // #FFE4E1
	Moccasin             = 0xFFB5E4FF, // #FFE4B5
	NavajoWhite          = 0xFFADDEFF, // #FFDEAD
	Navy                 = 0xFF800000, // #000080
	OldLace              = 0xFFE6F5FD, // #FDF5E6
	Olive                = 0xFF008080, // #808000
	OliveDrab            = 0xFF238E6B, // #6B8E23
	Orange               = 0xFF00A5FF, // #FFA500
	OrangeRed            = 0xFF0045FF, // #FF4500
	Orchid               = 0xFFD670DA, // #DA70D6
	PaleGoldenrod        = 0xFFAAE8EE, // #EEE8AA
	PaleGreen            = 0xFF98FB98, // #98FB98
	PaleTurquoise        = 0xFFEEEEAF, // #AFEEEE
	PaleVioletRed        = 0xFF9370DB, // #DB7093
	PapayaWhip           = 0xFFD5EFFF, // #FFEFD5
	PeachPuff            = 0xFFB9DAFF, // #FFDAB9
	Peru                 = 0xFF3F85CD, // #CD853F
	Pink                 = 0xFFCBC0FF, // #FFC0CB
	Plum                 = 0xFFDDA0DD, // #DDA0DD
	PowderBlue           = 0xFFE6E0B0, // #B0E0E6
	Purple               = 0xFF800080, // #800080
	Red                  = 0xFF0000FF, // #FF0000
	RosyBrown            = 0xFF8F8FBC, // #BC8F8F
	RoyalBlue            = 0xFFE16941, // #4169E1
	SaddleBrown          = 0xFF13458B, // #8B4513
	Salmon               = 0xFF7280FA, // #FA8072
	SandyBrown           = 0xFF60A4F4, // #F4A460
	SeaGreen             = 0xFF578B2E, // #2E8B57
	SeaShell             = 0xFFEEF5FF, // #FFF5EE
	Sienna               = 0xFF2D52A0, // #A0522D
	Silver               = 0xFFC0C0C0, // #C0C0C0
	SkyBlue              = 0xFFEBCE87, // #87CEEB
	SlateBlue            = 0xFFCD5A6A, // #6A5ACD
	SlateGray            = 0xFF908070, // #708090
	Snow                 = 0xFFFAFAFF, // #FFFAFA
	SpringGreen          = 0xFF7FFF00, // #00FF7F
	SteelBlue            = 0xFFB48246, // #4682B4
	Tan                  = 0xFF8CB4D2, // #D2B48C
	Teal                 = 0xFF808000, // #008080
	Thistle              = 0xFFD8BFD8, // #D8BFD8
	Tomato               = 0xFF4763FF, // #FF6347
	Transparent          = 0x00FFFFFF, // #FFFFFF
	Turquoise            = 0xFFD0E040, // #40E0D0
	Violet               = 0xFFEE82EE, // #EE82EE
	Wheat                = 0xFFB3DEF5, // #F5DEB3
	White                = 0xFFFFFFFF, // #FFFFFF
	WhiteSmoke           = 0xFFF5F5F5, // #F5F5F5
	Yellow               = 0xFF00FFFF, // #FFFF00
	YellowGreen	         = 0xFF32CD9A, // #9ACD32
	Error                = Magenta
};

struct Color {
	static VERA_CONSTEXPR Format format = Format::RGBA8Unorm;

	static VERA_CONSTEXPR Color RGB(uint32_t r, uint32_t g, uint32_t b) VERA_NOEXCEPT
	{
		return Color(
			static_cast<uint8_t>(r),
			static_cast<uint8_t>(g),
			static_cast<uint8_t>(b),
			static_cast<uint8_t>(255));
	}

	static VERA_CONSTEXPR Color RGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a) VERA_NOEXCEPT
	{
		return Color(
			static_cast<uint8_t>(r),
			static_cast<uint8_t>(g),
			static_cast<uint8_t>(b),
			static_cast<uint8_t>(a));
	}

	static VERA_CONSTEXPR Color RGB(float r, float g, float b) VERA_NOEXCEPT
	{
		return Color(r, g, b, 1.f);
	}

	static VERA_CONSTEXPR Color RGBA(float r, float g, float b, float a) VERA_NOEXCEPT
	{
		return Color(r, g, b, a);
	}

	static VERA_CONSTEXPR Color HSV(float h, float s, float v) VERA_NOEXCEPT
	{
		h == 360.f ? h = 0.f : h /= 60.f;

		float fract = h - (int32_t)(h);
		float p     = v * (1.f - s);
		float q     = v * (1.f - s * fract);
		float t     = v * (1.f - s * (1.f - fract));
	
		switch ((uint32_t)h) {
		case 0: return Color(v, t, p);
		case 1: return Color(q, v, p);
		case 2: return Color(p, v, t);
		case 3: return Color(p, q, v);
		case 4: return Color(t, p, v);
		case 5: return Color(v, p, q);
		default: return Color(0.f, 0.f, 0.f);
		}
	}

	VERA_CONSTEXPR Color() VERA_NOEXCEPT :
		r(), g(), b(), a() {}
		
	explicit VERA_CONSTEXPR Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) VERA_NOEXCEPT :
		r(r), g(g), b(b), a(a) {}
	
	explicit VERA_CONSTEXPR Color(float r, float g, float b, float a = 1.f) VERA_NOEXCEPT :
		r(static_cast<uint8_t>(255.99f * r)),
		g(static_cast<uint8_t>(255.99f * g)),
		b(static_cast<uint8_t>(255.99f * b)),
		a(static_cast<uint8_t>(255.99f * a)) {}

	explicit VERA_CONSTEXPR Color(const float3& rgb, float a = 1.f) VERA_NOEXCEPT :
		Color(rgb.x, rgb.y, rgb.z, a) {}

	explicit VERA_CONSTEXPR Color(const float4& rgba) VERA_NOEXCEPT :
		Color(rgba.x, rgba.y, rgba.z, rgba.w) {}

	VERA_CONSTEXPR Color(Colors color) VERA_NOEXCEPT :
		Color()
	{
		*this = std::bit_cast<Color>(color);
	}

	VERA_CONSTEXPR Color(Colors color, uint8_t a) VERA_NOEXCEPT :
	Color()
	{
		*this = std::bit_cast<Color>(((uint32_t)color & 0x00ffffff) | a << 24);
	}

	VERA_NODISCARD VERA_CONSTEXPR uint32_t pack() const VERA_NOEXCEPT
	{
		return std::bit_cast<uint32_t>(*this);
	}

	VERA_NODISCARD VERA_CONSTEXPR float4 unorm() const VERA_NOEXCEPT
	{
		return { r / 255.f, g / 255.f, b / 255.f, a / 255.f };
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Color& rhs) const VERA_NOEXCEPT {
		return pack() == rhs.pack();
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Color& rhs) const VERA_NOEXCEPT {
		return pack() != rhs.pack();
	}

	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

VERA_NAMESPACE_END