#pragma once

#include "../core/coredefs.h"
#include <stdint.h>

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

enum class Key VERA_ENUM
{
	Unknown,            // Unknown key
	A,                  // Keyboard a and A key
	B,                  // Keyboard b and B key
	C,                  // Keyboard c and C key
	D,                  // Keyboard d and D key
	E,                  // Keyboard e and E key
	F,                  // Keyboard f and F key
	G,                  // Keyboard g and G key
	H,                  // Keyboard h and H key
	I,                  // Keyboard i and I key
	J,                  // Keyboard j and J key
	K,                  // Keyboard k and K key
	L,                  // Keyboard l and L key
	M,                  // Keyboard m and M key
	N,                  // Keyboard n and N key
	O,                  // Keyboard o and O key
	P,                  // Keyboard p and P key
	Q,                  // Keyboard q and Q key
	R,                  // Keyboard r and R key
	S,                  // Keyboard s and S key
	T,                  // Keyboard t and T key
	U,                  // Keyboard u and U key
	V,                  // Keyboard v and V key
	W,                  // Keyboard w and W key
	X,                  // Keyboard x and X key
	Y,                  // Keyboard y and Y key
	Z,                  // Keyboard z and Z key
	Num1,               // Keyboard 1 and ! key
	Num2,               // Keyboard 2 and @ key
	Num3,               // Keyboard 3 and # key
	Num4,               // Keyboard 4 and $ key
	Num5,               // Keyboard 5 and % key
	Num6,               // Keyboard 6 and ^ key
	Num7,               // Keyboard 7 and & key
	Num8,               // Keyboard 8 and * key
	Num9,               // Keyboard 9 and ) key
	Num0,               // Keyboard 0 and ) key
	Enter,              // Keyboard Enter/Return key
	Escape,             // Keyboard Escape key
	Backspace,          // Keyboard Backspace key
	Tab,                // Keyboard Tab key
	Space,              // Keyboard Space key
	Hyphen,             // Keyboard - and _ key
	Equal,              // Keyboard = and +
	LBracket,           // Keyboard [ and { key
	RBracket,           // Keyboard ] and } key
	Backslash,          // Keyboard \ and | key OR various keys for Non-US keyboards
	Semicolon,          // Keyboard ; and : key
	Apostrophe,         // Keyboard ' and " key
	Grave,              // Keyboard ` and ~ key
	Comma,              // Keyboard , and < key
	Period,             // Keyboard . and > key
	Slash,              // Keyboard / and ? key
	F1,                 // Keyboard F1 key
	F2,                 // Keyboard F2 key
	F3,                 // Keyboard F3 key
	F4,                 // Keyboard F4 key
	F5,                 // Keyboard F5 key
	F6,                 // Keyboard F6 key
	F7,                 // Keyboard F7 key
	F8,                 // Keyboard F8 key
	F9,                 // Keyboard F9 key
	F10,                // Keyboard F10 key
	F11,                // Keyboard F11 key
	F12,                // Keyboard F12 key
	F13,                // Keyboard F13 key
	F14,                // Keyboard F14 key
	F15,                // Keyboard F15 key
	F16,                // Keyboard F16 key
	F17,                // Keyboard F17 key
	F18,                // Keyboard F18 key
	F19,                // Keyboard F19 key
	F20,                // Keyboard F20 key
	F21,                // Keyboard F21 key
	F22,                // Keyboard F22 key
	F23,                // Keyboard F23 key
	F24,                // Keyboard F24 key
	CapsLock,           // Keyboard Caps %Lock key
	PrintScreen,        // Keyboard Print Screen key
	ScrollLock,         // Keyboard Scroll %Lock key
	Pause,              // Keyboard Pause key
	Insert,             // Keyboard Insert key
	Home,               // Keyboard Home key
	PageUp,             // Keyboard Page Up key
	Delete,             // Keyboard Delete Forward key
	End,                // Keyboard End key
	PageDown,           // Keyboard Page Down key
	Right,              // Keyboard Right Arrow key
	Left,               // Keyboard Left Arrow key
	Down,               // Keyboard Down Arrow key
	Up,                 // Keyboard Up Arrow key
	NumLock,            // Keypad Num %Lock and Clear key
	NumpadDivide,       // Keypad / key
	NumpadMultiply,     // Keypad * key
	NumpadMinus,        // Keypad - key
	NumpadPlus,         // Keypad + key
	NumpadEqual,        // keypad = key
	NumpadEnter,        // Keypad Enter/Return key
	NumpadDecimal,      // Keypad . and Delete key
	Numpad1,            // Keypad 1 and End key
	Numpad2,            // Keypad 2 and Down Arrow key
	Numpad3,            // Keypad 3 and Page Down key
	Numpad4,            // Keypad 4 and Left Arrow key
	Numpad5,            // Keypad 5 key
	Numpad6,            // Keypad 6 and Right Arrow key
	Numpad7,            // Keypad 7 and Home key
	Numpad8,            // Keypad 8 and Up Arrow key
	Numpad9,            // Keypad 9 and Page Up key
	Numpad0,            // Keypad 0 and Insert key
	NonUsBackslash,     // Keyboard Non-US \ and | key
	Application,        // Keyboard Application key
	Execute,            // Keyboard Execute key
	ModeChange,         // Keyboard Mode Change key
	Help,               // Keyboard Help key
	Menu,               // Keyboard Menu key
	Select,             // Keyboard Select key
	Paste,              // Keyboard Paste key
	VolumeMute,         // Keyboard Volume Mute key
	VolumeUp,           // Keyboard Volume Up key
	VolumeDown,         // Keyboard Volume Down key
	MediaPlayPause,     // Keyboard Media Play Pause key
	MediaStop,          // Keyboard Media Stop key
	MediaNextTrack,     // Keyboard Media Next Track key
	MediaPreviousTrack, // Keyboard Media Previous Track key
	LControl,           // Keyboard Left Control key
	LShift,             // Keyboard Left Shift key
	LAlt,               // Keyboard Left Alt key
	LSystem,            // Keyboard Left System key
	RControl,           // Keyboard Right Control key
	RShift,             // Keyboard Right Shift key
	RAlt,               // Keyboard Right Alt key
	RSystem,            // Keyboard Right System key
	Back,               // Keyboard Back key
	Forward,            // Keyboard Forward key
	Refresh,            // Keyboard Refresh key
	Stop,               // Keyboard Stop key
	Search,             // Keyboard Search key
	Favorites,          // Keyboard Favorites key
	HomePage,           // Keyboard Home Page key
	LaunchApplication1, // Keyboard Launch Application 1 key
	LaunchApplication2, // Keyboard Launch Application 2 key
	LaunchMail,         // Keyboard Launch Mail key
	LaunchMediaSelect,  // Keyboard Launch Media Select key
};

class Mod
{
public:
	enum Key {
		LCtrl = 1 << 1,
		RCtrl = 1 << 2,
		LShft = 1 << 3,
		RShft = 1 << 4,
		LAlt  = 1 << 5,
		RAlt  = 1 << 6,
		Ctrl  = LCtrl | RCtrl,
		Shft  = LShft | RShft,
		Alt   = LAlt | RAlt,
	};

	VERA_CONSTEXPR Mod(uint8_t mods) VERA_NOEXCEPT :
		m_mods(mods) {}

	VERA_NODISCARD VERA_CONSTEXPR bool has(uint8_t mods) const VERA_NOEXCEPT
	{
		return m_mods & mods;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool ctrl() const VERA_NOEXCEPT
	{
		return m_mods & Ctrl;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool lctrl() const VERA_NOEXCEPT
	{
		return m_mods & RCtrl;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool rctrl() const VERA_NOEXCEPT
	{
		return m_mods & RCtrl;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool shft() const VERA_NOEXCEPT
	{
		return m_mods & Shft;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool lshft() const VERA_NOEXCEPT
	{
		return m_mods & LShft;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool rshft() const VERA_NOEXCEPT
	{
		return m_mods & RShft;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool alt() const VERA_NOEXCEPT
	{
		return m_mods & Alt;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool lalt() const VERA_NOEXCEPT
	{
		return m_mods & LAlt;
	}
	VERA_NODISCARD VERA_CONSTEXPR bool ralt() const VERA_NOEXCEPT
	{
		return m_mods & RAlt;
	}

private:
	uint8_t m_mods;
};

class Keyboard
{
public:
	VERA_NODISCARD static int32_t getNumber(Key key) VERA_NOEXCEPT;
	VERA_NODISCARD static int32_t getPadNumber(Key key) VERA_NOEXCEPT;
	VERA_NODISCARD static int32_t getFunctionID(Key key) VERA_NOEXCEPT;
	VERA_NODISCARD static bool isNumber(Key key) VERA_NOEXCEPT;
	VERA_NODISCARD static bool isPadNumber(Key key) VERA_NOEXCEPT;
	VERA_NODISCARD static bool isFunction(Key key) VERA_NOEXCEPT;

	VERA_NODISCARD static bool isDown(Key key) VERA_NOEXCEPT;
	VERA_NODISCARD static bool isUp(Key key) VERA_NOEXCEPT;

	VERA_NODISCARD static Mod mods() VERA_NOEXCEPT;
	
	VERA_NODISCARD static bool ctrl() VERA_NOEXCEPT;
	VERA_NODISCARD static bool lctrl() VERA_NOEXCEPT;
	VERA_NODISCARD static bool rctrl() VERA_NOEXCEPT;
	VERA_NODISCARD static bool shft() VERA_NOEXCEPT;
	VERA_NODISCARD static bool lshft() VERA_NOEXCEPT;
	VERA_NODISCARD static bool rshft() VERA_NOEXCEPT;
	VERA_NODISCARD static bool alt() VERA_NOEXCEPT;
	VERA_NODISCARD static bool lalt() VERA_NOEXCEPT;
	VERA_NODISCARD static bool ralt() VERA_NOEXCEPT;
	VERA_NODISCARD static bool system() VERA_NOEXCEPT;
	VERA_NODISCARD static bool lsystem() VERA_NOEXCEPT;
	VERA_NODISCARD static bool rsystem() VERA_NOEXCEPT;
};

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END