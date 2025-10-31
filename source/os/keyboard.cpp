#include "../../include/vera/os/keyboard.h"

#include "../../include/vera/core/assertion.h"

///////////////////////////////////////////////////////////////////////////////
// Common keyboard handling across platforms //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

int32_t Keyboard::getNumber(Key key) VERA_NOEXCEPT
{
	switch (key) {
	case Key::Num1: return 1;
	case Key::Num2: return 2;
	case Key::Num3: return 3;
	case Key::Num4: return 4;
	case Key::Num5: return 5;
	case Key::Num6: return 6;
	case Key::Num7: return 7;
	case Key::Num8: return 8;
	case Key::Num9: return 9;
	case Key::Num0: return 0;
	}

	return -1;
}

int32_t Keyboard::getPadNumber(Key key) VERA_NOEXCEPT
{
	switch (key) {
	case Key::Numpad1: return 1;
	case Key::Numpad2: return 2;
	case Key::Numpad3: return 3;
	case Key::Numpad4: return 4;
	case Key::Numpad5: return 5;
	case Key::Numpad6: return 6;
	case Key::Numpad7: return 7;
	case Key::Numpad8: return 8;
	case Key::Numpad9: return 9;
	case Key::Numpad0: return 0;
	}

	return -1;
}

int32_t Keyboard::getFunctionID(Key key) VERA_NOEXCEPT
{
	switch (key) {
	case Key::F1:  return 1;
	case Key::F2:  return 2;
	case Key::F3:  return 3;
	case Key::F4:  return 4;
	case Key::F5:  return 5;
	case Key::F6:  return 6;
	case Key::F7:  return 7;
	case Key::F8:  return 8;
	case Key::F9:  return 9;
	case Key::F10: return 10;
	case Key::F11: return 11;
	case Key::F12: return 12;
	case Key::F13: return 13;
	case Key::F14: return 14;
	case Key::F15: return 15;
	case Key::F16: return 16;
	case Key::F17: return 17;
	case Key::F18: return 18;
	case Key::F19: return 19;
	case Key::F20: return 20;
	case Key::F21: return 21;
	case Key::F22: return 22;
	case Key::F23: return 23;
	case Key::F24: return 24;
	}

	return -1;
}

bool Keyboard::isNumber(Key key) VERA_NOEXCEPT
{
	return getNumber(key) != -1;
}

bool Keyboard::isPadNumber(Key key) VERA_NOEXCEPT
{
	return getPadNumber(key) != -1;
}

bool Keyboard::isFunction(Key key) VERA_NOEXCEPT
{
	return getFunctionID(key) != -1;
}

bool Keyboard::ctrl() VERA_NOEXCEPT
{
	return lctrl() || rctrl();
}

bool Keyboard::shft() VERA_NOEXCEPT
{
	return lshft() || rshft();
}

bool Keyboard::alt() VERA_NOEXCEPT
{
	return lalt() || ralt();
}

bool Keyboard::system() VERA_NOEXCEPT
{
	return lsystem() || rsystem();
}

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END

///////////////////////////////////////////////////////////////////////////////
// Windows ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

bool Keyboard::isDown(Key key) VERA_NOEXCEPT
{
	switch (key) {
	case Key::A:                  return GetAsyncKeyState('A') & 0x8000;
	case Key::B:                  return GetAsyncKeyState('B') & 0x8000;
	case Key::C:                  return GetAsyncKeyState('C') & 0x8000;
	case Key::D:                  return GetAsyncKeyState('D') & 0x8000;
	case Key::E:                  return GetAsyncKeyState('E') & 0x8000;
	case Key::F:                  return GetAsyncKeyState('F') & 0x8000;
	case Key::G:                  return GetAsyncKeyState('G') & 0x8000;
	case Key::H:                  return GetAsyncKeyState('H') & 0x8000;
	case Key::I:                  return GetAsyncKeyState('I') & 0x8000;
	case Key::J:                  return GetAsyncKeyState('J') & 0x8000;
	case Key::K:                  return GetAsyncKeyState('K') & 0x8000;
	case Key::L:                  return GetAsyncKeyState('L') & 0x8000;
	case Key::M:                  return GetAsyncKeyState('M') & 0x8000;
	case Key::N:                  return GetAsyncKeyState('N') & 0x8000;
	case Key::O:                  return GetAsyncKeyState('O') & 0x8000;
	case Key::P:                  return GetAsyncKeyState('P') & 0x8000;
	case Key::Q:                  return GetAsyncKeyState('Q') & 0x8000;
	case Key::R:                  return GetAsyncKeyState('R') & 0x8000;
	case Key::S:                  return GetAsyncKeyState('S') & 0x8000;
	case Key::T:                  return GetAsyncKeyState('T') & 0x8000;
	case Key::U:                  return GetAsyncKeyState('U') & 0x8000;
	case Key::V:                  return GetAsyncKeyState('V') & 0x8000;
	case Key::W:                  return GetAsyncKeyState('W') & 0x8000;
	case Key::X:                  return GetAsyncKeyState('X') & 0x8000;
	case Key::Y:                  return GetAsyncKeyState('Y') & 0x8000;
	case Key::Z:                  return GetAsyncKeyState('Z') & 0x8000;
	case Key::Num1:               return GetAsyncKeyState('1') & 0x8000;
	case Key::Num2:               return GetAsyncKeyState('2') & 0x8000;
	case Key::Num3:               return GetAsyncKeyState('3') & 0x8000;
	case Key::Num4:               return GetAsyncKeyState('4') & 0x8000;
	case Key::Num5:               return GetAsyncKeyState('5') & 0x8000;
	case Key::Num6:               return GetAsyncKeyState('6') & 0x8000;
	case Key::Num7:               return GetAsyncKeyState('7') & 0x8000;
	case Key::Num8:               return GetAsyncKeyState('8') & 0x8000;
	case Key::Num9:               return GetAsyncKeyState('9') & 0x8000;
	case Key::Num0:               return GetAsyncKeyState('0') & 0x8000;
	case Key::Enter:              return GetAsyncKeyState(VK_RETURN) & 0x8000;
	case Key::Escape:             return GetAsyncKeyState(VK_ESCAPE) & 0x8000;
	case Key::Backspace:          return GetAsyncKeyState(VK_BACK) & 0x8000;
	case Key::Tab:                return GetAsyncKeyState(VK_TAB) & 0x8000;
	case Key::Space:              return GetAsyncKeyState(VK_SPACE) & 0x8000;
	case Key::Hyphen:             return GetAsyncKeyState(VK_OEM_MINUS) & 0x8000;
	case Key::Equal:              return GetAsyncKeyState(VK_OEM_PLUS) & 0x8000;
	case Key::LBracket:           return GetAsyncKeyState(VK_OEM_4) & 0x8000;
	case Key::RBracket:           return GetAsyncKeyState(VK_OEM_6) & 0x8000;
	case Key::Backslash:          return GetAsyncKeyState(VK_OEM_5) & 0x8000;
	case Key::Semicolon:          return GetAsyncKeyState(VK_OEM_1) & 0x8000;
	case Key::Apostrophe:         return GetAsyncKeyState(VK_OEM_7) & 0x8000;
	case Key::Grave:              return GetAsyncKeyState(VK_OEM_3) & 0x8000;
	case Key::Comma:              return GetAsyncKeyState(VK_OEM_COMMA) & 0x8000;
	case Key::Period:             return GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000;
	case Key::Slash:              return GetAsyncKeyState(VK_OEM_2) & 0x8000;
	case Key::F1:                 return GetAsyncKeyState(VK_F1) & 0x8000;
	case Key::F2:                 return GetAsyncKeyState(VK_F2) & 0x8000;
	case Key::F3:                 return GetAsyncKeyState(VK_F3) & 0x8000;
	case Key::F4:                 return GetAsyncKeyState(VK_F4) & 0x8000;
	case Key::F5:                 return GetAsyncKeyState(VK_F5) & 0x8000;
	case Key::F6:                 return GetAsyncKeyState(VK_F6) & 0x8000;
	case Key::F7:                 return GetAsyncKeyState(VK_F7) & 0x8000;
	case Key::F8:                 return GetAsyncKeyState(VK_F8) & 0x8000;
	case Key::F9:                 return GetAsyncKeyState(VK_F9) & 0x8000;
	case Key::F10:                return GetAsyncKeyState(VK_F10) & 0x8000;
	case Key::F11:                return GetAsyncKeyState(VK_F11) & 0x8000;
	case Key::F12:                return GetAsyncKeyState(VK_F12) & 0x8000;
	case Key::F13:                return GetAsyncKeyState(VK_F13) & 0x8000;
	case Key::F14:                return GetAsyncKeyState(VK_F14) & 0x8000;
	case Key::F15:                return GetAsyncKeyState(VK_F15) & 0x8000;
	case Key::F16:                return GetAsyncKeyState(VK_F16) & 0x8000;
	case Key::F17:                return GetAsyncKeyState(VK_F17) & 0x8000;
	case Key::F18:                return GetAsyncKeyState(VK_F18) & 0x8000;
	case Key::F19:                return GetAsyncKeyState(VK_F19) & 0x8000;
	case Key::F20:                return GetAsyncKeyState(VK_F20) & 0x8000;
	case Key::F21:                return GetAsyncKeyState(VK_F21) & 0x8000;
	case Key::F22:                return GetAsyncKeyState(VK_F22) & 0x8000;
	case Key::F23:                return GetAsyncKeyState(VK_F23) & 0x8000;
	case Key::F24:                return GetAsyncKeyState(VK_F24) & 0x8000;
	case Key::CapsLock:           return GetAsyncKeyState(VK_CAPITAL) & 0x8000;
	case Key::PrintScreen:        return GetAsyncKeyState(VK_SNAPSHOT) & 0x8000;
	case Key::ScrollLock:         return GetAsyncKeyState(VK_SCROLL) & 0x8000;
	case Key::Pause:              return GetAsyncKeyState(VK_PAUSE) & 0x8000;
	case Key::Insert:             return GetAsyncKeyState(VK_INSERT) & 0x8000;
	case Key::Home:               return GetAsyncKeyState(VK_HOME) & 0x8000;
	case Key::PageUp:             return GetAsyncKeyState(VK_PRIOR) & 0x8000;
	case Key::Delete:             return GetAsyncKeyState(VK_DELETE) & 0x8000;
	case Key::End:                return GetAsyncKeyState(VK_END) & 0x8000;
	case Key::PageDown:           return GetAsyncKeyState(VK_NEXT) & 0x8000;
	case Key::Right:              return GetAsyncKeyState(VK_RIGHT) & 0x8000;
	case Key::Left:               return GetAsyncKeyState(VK_LEFT) & 0x8000;
	case Key::Down:               return GetAsyncKeyState(VK_DOWN) & 0x8000;
	case Key::Up:                 return GetAsyncKeyState(VK_UP) & 0x8000;
	case Key::NumLock:            return GetAsyncKeyState(VK_NUMLOCK) & 0x8000;
	case Key::NumpadDivide:       return GetAsyncKeyState(VK_DIVIDE) & 0x8000;
	case Key::NumpadMultiply:     return GetAsyncKeyState(VK_MULTIPLY) & 0x8000;
	case Key::NumpadMinus:        return GetAsyncKeyState(VK_SUBTRACT) & 0x8000;
	case Key::NumpadPlus:         return GetAsyncKeyState(VK_ADD) & 0x8000;
	case Key::NumpadEqual:        return GetAsyncKeyState(VK_OEM_NEC_EQUAL) & 0x8000;
	case Key::NumpadEnter:        return GetAsyncKeyState(VK_RETURN) & 0x8000;
	case Key::NumpadDecimal:      return GetAsyncKeyState(VK_DECIMAL) & 0x8000;
	case Key::Numpad1:            return GetAsyncKeyState(VK_NUMPAD1) & 0x8000;
	case Key::Numpad2:            return GetAsyncKeyState(VK_NUMPAD2) & 0x8000;
	case Key::Numpad3:            return GetAsyncKeyState(VK_NUMPAD3) & 0x8000;
	case Key::Numpad4:            return GetAsyncKeyState(VK_NUMPAD4) & 0x8000;
	case Key::Numpad5:            return GetAsyncKeyState(VK_NUMPAD5) & 0x8000;
	case Key::Numpad6:            return GetAsyncKeyState(VK_NUMPAD6) & 0x8000;
	case Key::Numpad7:            return GetAsyncKeyState(VK_NUMPAD7) & 0x8000;
	case Key::Numpad8:            return GetAsyncKeyState(VK_NUMPAD8) & 0x8000;
	case Key::Numpad9:            return GetAsyncKeyState(VK_NUMPAD9) & 0x8000;
	case Key::Numpad0:            return GetAsyncKeyState(VK_NUMPAD0) & 0x8000;
	case Key::NonUsBackslash:     return GetAsyncKeyState(VK_OEM_102) & 0x8000;
	case Key::Application:        return GetAsyncKeyState(VK_APPS) & 0x8000;
	case Key::Execute:            return GetAsyncKeyState(VK_EXECUTE) & 0x8000;
	case Key::ModeChange:         return GetAsyncKeyState(VK_MODECHANGE) & 0x8000;
	case Key::Help:               return GetAsyncKeyState(VK_HELP) & 0x8000;
	case Key::Menu:               return GetAsyncKeyState(VK_MENU) & 0x8000;
	case Key::Select:             return GetAsyncKeyState(VK_SELECT) & 0x8000;
	case Key::VolumeMute:         return GetAsyncKeyState(VK_VOLUME_MUTE) & 0x8000;
	case Key::VolumeUp:           return GetAsyncKeyState(VK_VOLUME_UP) & 0x8000;
	case Key::VolumeDown:         return GetAsyncKeyState(VK_VOLUME_DOWN) & 0x8000;
	case Key::MediaPlayPause:     return GetAsyncKeyState(VK_MEDIA_PLAY_PAUSE) & 0x8000;
	case Key::MediaStop:          return GetAsyncKeyState(VK_MEDIA_STOP) & 0x8000;
	case Key::MediaNextTrack:     return GetAsyncKeyState(VK_MEDIA_NEXT_TRACK) & 0x8000;
	case Key::MediaPreviousTrack: return GetAsyncKeyState(VK_MEDIA_PREV_TRACK) & 0x8000;
	case Key::LControl:           return GetAsyncKeyState(VK_LCONTROL) & 0x8000;
	case Key::LShift:             return GetAsyncKeyState(VK_LSHIFT) & 0x8000;
	case Key::LAlt:               return GetAsyncKeyState(VK_LMENU) & 0x8000;
	case Key::LSystem:            return GetAsyncKeyState(VK_LWIN) & 0x8000;
	case Key::RControl:           return GetAsyncKeyState(VK_RCONTROL) & 0x8000;
	case Key::RShift:             return GetAsyncKeyState(VK_RSHIFT) & 0x8000;
	case Key::RAlt:               return GetAsyncKeyState(VK_RMENU) & 0x8000;
	case Key::RSystem:            return GetAsyncKeyState(VK_RWIN) & 0x8000;
	case Key::Back:               return GetAsyncKeyState(VK_BACK) & 0x8000;
	case Key::Forward:            return GetAsyncKeyState(VK_BROWSER_FORWARD) & 0x8000;
	case Key::Refresh:            return GetAsyncKeyState(VK_BROWSER_REFRESH) & 0x8000;
	case Key::Stop:               return GetAsyncKeyState(VK_BROWSER_STOP) & 0x8000;
	case Key::Search:             return GetAsyncKeyState(VK_BROWSER_SEARCH) & 0x8000;
	case Key::Favorites:          return GetAsyncKeyState(VK_BROWSER_FAVORITES) & 0x8000;
	case Key::HomePage:           return GetAsyncKeyState(VK_BROWSER_HOME) & 0x8000;
	case Key::LaunchApplication1: return GetAsyncKeyState(VK_LAUNCH_APP1) & 0x8000;
	case Key::LaunchApplication2: return GetAsyncKeyState(VK_LAUNCH_APP2) & 0x8000;
	case Key::LaunchMail:         return GetAsyncKeyState(VK_LAUNCH_MAIL) & 0x8000;
	case Key::LaunchMediaSelect:  return GetAsyncKeyState(VK_LAUNCH_MEDIA_SELECT) & 0x8000;
	}

	VERA_ASSERT_MSG(false, "unknown key");
	return false;
}

bool Keyboard::isUp(Key key) VERA_NOEXCEPT
{
	return !isDown(key);
}

Mod Keyboard::mods() VERA_NOEXCEPT
{
	uint8_t mods = 0;

	mods |= (GetAsyncKeyState(VK_LCONTROL) & 0x8000) ? Mod::LCtrl : 0;
	mods |= (GetAsyncKeyState(VK_RCONTROL) & 0x8000) ? Mod::RCtrl : 0;
	mods |= (GetAsyncKeyState(VK_LSHIFT) & 0x8000) ? Mod::LShft : 0;
	mods |= (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? Mod::RShft : 0;
	mods |= (GetAsyncKeyState(VK_LMENU) & 0x8000) ? Mod::LAlt : 0;
	mods |= (GetAsyncKeyState(VK_RMENU) & 0x8000) ? Mod::RAlt : 0;
	
	return Mod(mods);
}

bool Keyboard::lctrl() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LCONTROL) & 0x8000;
}

bool Keyboard::rctrl() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RCONTROL) & 0x8000;
}

bool Keyboard::lshft() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LSHIFT) & 0x8000;
}

bool Keyboard::rshft() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RSHIFT) & 0x8000;
}

bool Keyboard::lalt() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LMENU) & 0x8000;
}

bool Keyboard::ralt() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RMENU) & 0x8000;
}

bool Keyboard::lsystem() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LWIN) & 0x8000;
}

bool Keyboard::rsystem() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RWIN) & 0x8000;
}

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END
#endif // _Win32

#ifdef ANOTHER_PLATFORM

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

bool Keyboard::isDown(Key key) VERA_NOEXCEPT
{
	switch (key) {
	case Key::A:                  return GetAsyncKeyState('A') & 0x8000;
	case Key::B:                  return GetAsyncKeyState('B') & 0x8000;
	case Key::C:                  return GetAsyncKeyState('C') & 0x8000;
	case Key::D:                  return GetAsyncKeyState('D') & 0x8000;
	case Key::E:                  return GetAsyncKeyState('E') & 0x8000;
	case Key::F:                  return GetAsyncKeyState('F') & 0x8000;
	case Key::G:                  return GetAsyncKeyState('G') & 0x8000;
	case Key::H:                  return GetAsyncKeyState('H') & 0x8000;
	case Key::I:                  return GetAsyncKeyState('I') & 0x8000;
	case Key::J:                  return GetAsyncKeyState('J') & 0x8000;
	case Key::K:                  return GetAsyncKeyState('K') & 0x8000;
	case Key::L:                  return GetAsyncKeyState('L') & 0x8000;
	case Key::M:                  return GetAsyncKeyState('M') & 0x8000;
	case Key::N:                  return GetAsyncKeyState('N') & 0x8000;
	case Key::O:                  return GetAsyncKeyState('O') & 0x8000;
	case Key::P:                  return GetAsyncKeyState('P') & 0x8000;
	case Key::Q:                  return GetAsyncKeyState('Q') & 0x8000;
	case Key::R:                  return GetAsyncKeyState('R') & 0x8000;
	case Key::S:                  return GetAsyncKeyState('S') & 0x8000;
	case Key::T:                  return GetAsyncKeyState('T') & 0x8000;
	case Key::U:                  return GetAsyncKeyState('U') & 0x8000;
	case Key::V:                  return GetAsyncKeyState('V') & 0x8000;
	case Key::W:                  return GetAsyncKeyState('W') & 0x8000;
	case Key::X:                  return GetAsyncKeyState('X') & 0x8000;
	case Key::Y:                  return GetAsyncKeyState('Y') & 0x8000;
	case Key::Z:                  return GetAsyncKeyState('Z') & 0x8000;
	case Key::Num1:               return GetAsyncKeyState('1') & 0x8000;
	case Key::Num2:               return GetAsyncKeyState('2') & 0x8000;
	case Key::Num3:               return GetAsyncKeyState('3') & 0x8000;
	case Key::Num4:               return GetAsyncKeyState('4') & 0x8000;
	case Key::Num5:               return GetAsyncKeyState('5') & 0x8000;
	case Key::Num6:               return GetAsyncKeyState('6') & 0x8000;
	case Key::Num7:               return GetAsyncKeyState('7') & 0x8000;
	case Key::Num8:               return GetAsyncKeyState('8') & 0x8000;
	case Key::Num9:               return GetAsyncKeyState('9') & 0x8000;
	case Key::Num0:               return GetAsyncKeyState('0') & 0x8000;
	case Key::Enter:              return GetAsyncKeyState(VK_RETURN) & 0x8000;
	case Key::Escape:             return GetAsyncKeyState(VK_ESCAPE) & 0x8000;
	case Key::Backspace:          return GetAsyncKeyState(VK_BACK) & 0x8000;
	case Key::Tab:                return GetAsyncKeyState(VK_TAB) & 0x8000;
	case Key::Space:              return GetAsyncKeyState(VK_SPACE) & 0x8000;
	case Key::Hyphen:             return GetAsyncKeyState(VK_OEM_MINUS) & 0x8000;
	case Key::Equal:              return GetAsyncKeyState(VK_OEM_PLUS) & 0x8000;
	case Key::LBracket:           return GetAsyncKeyState(VK_OEM_4) & 0x8000;
	case Key::RBracket:           return GetAsyncKeyState(VK_OEM_6) & 0x8000;
	case Key::Backslash:          return GetAsyncKeyState(VK_OEM_5) & 0x8000;
	case Key::Semicolon:          return GetAsyncKeyState(VK_OEM_1) & 0x8000;
	case Key::Apostrophe:         return GetAsyncKeyState(VK_OEM_7) & 0x8000;
	case Key::Grave:              return GetAsyncKeyState(VK_OEM_3) & 0x8000;
	case Key::Comma:              return GetAsyncKeyState(VK_OEM_COMMA) & 0x8000;
	case Key::Period:             return GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000;
	case Key::Slash:              return GetAsyncKeyState(VK_OEM_2) & 0x8000;
	case Key::F1:                 return GetAsyncKeyState(VK_F1) & 0x8000;
	case Key::F2:                 return GetAsyncKeyState(VK_F2) & 0x8000;
	case Key::F3:                 return GetAsyncKeyState(VK_F3) & 0x8000;
	case Key::F4:                 return GetAsyncKeyState(VK_F4) & 0x8000;
	case Key::F5:                 return GetAsyncKeyState(VK_F5) & 0x8000;
	case Key::F6:                 return GetAsyncKeyState(VK_F6) & 0x8000;
	case Key::F7:                 return GetAsyncKeyState(VK_F7) & 0x8000;
	case Key::F8:                 return GetAsyncKeyState(VK_F8) & 0x8000;
	case Key::F9:                 return GetAsyncKeyState(VK_F9) & 0x8000;
	case Key::F10:                return GetAsyncKeyState(VK_F10) & 0x8000;
	case Key::F11:                return GetAsyncKeyState(VK_F11) & 0x8000;
	case Key::F12:                return GetAsyncKeyState(VK_F12) & 0x8000;
	case Key::F13:                return GetAsyncKeyState(VK_F13) & 0x8000;
	case Key::F14:                return GetAsyncKeyState(VK_F14) & 0x8000;
	case Key::F15:                return GetAsyncKeyState(VK_F15) & 0x8000;
	case Key::F16:                return GetAsyncKeyState(VK_F16) & 0x8000;
	case Key::F17:                return GetAsyncKeyState(VK_F17) & 0x8000;
	case Key::F18:                return GetAsyncKeyState(VK_F18) & 0x8000;
	case Key::F19:                return GetAsyncKeyState(VK_F19) & 0x8000;
	case Key::F20:                return GetAsyncKeyState(VK_F20) & 0x8000;
	case Key::F21:                return GetAsyncKeyState(VK_F21) & 0x8000;
	case Key::F22:                return GetAsyncKeyState(VK_F22) & 0x8000;
	case Key::F23:                return GetAsyncKeyState(VK_F23) & 0x8000;
	case Key::F24:                return GetAsyncKeyState(VK_F24) & 0x8000;
	case Key::CapsLock:           return GetAsyncKeyState(VK_CAPITAL) & 0x8000;
	case Key::PrintScreen:        return GetAsyncKeyState(VK_SNAPSHOT) & 0x8000;
	case Key::ScrollLock:         return GetAsyncKeyState(VK_SCROLL) & 0x8000;
	case Key::Pause:              return GetAsyncKeyState(VK_PAUSE) & 0x8000;
	case Key::Insert:             return GetAsyncKeyState(VK_INSERT) & 0x8000;
	case Key::Home:               return GetAsyncKeyState(VK_HOME) & 0x8000;
	case Key::PageUp:             return GetAsyncKeyState(VK_PRIOR) & 0x8000;
	case Key::Delete:             return GetAsyncKeyState(VK_DELETE) & 0x8000;
	case Key::End:                return GetAsyncKeyState(VK_END) & 0x8000;
	case Key::PageDown:           return GetAsyncKeyState(VK_NEXT) & 0x8000;
	case Key::Right:              return GetAsyncKeyState(VK_RIGHT) & 0x8000;
	case Key::Left:               return GetAsyncKeyState(VK_LEFT) & 0x8000;
	case Key::Down:               return GetAsyncKeyState(VK_DOWN) & 0x8000;
	case Key::Up:                 return GetAsyncKeyState(VK_UP) & 0x8000;
	case Key::NumLock:            return GetAsyncKeyState(VK_NUMLOCK) & 0x8000;
	case Key::NumpadDivide:       return GetAsyncKeyState(VK_DIVIDE) & 0x8000;
	case Key::NumpadMultiply:     return GetAsyncKeyState(VK_MULTIPLY) & 0x8000;
	case Key::NumpadMinus:        return GetAsyncKeyState(VK_SUBTRACT) & 0x8000;
	case Key::NumpadPlus:         return GetAsyncKeyState(VK_ADD) & 0x8000;
	case Key::NumpadEqual:        return GetAsyncKeyState(VK_OEM_NEC_EQUAL) & 0x8000;
	case Key::NumpadEnter:        return GetAsyncKeyState(VK_RETURN) & 0x8000;
	case Key::NumpadDecimal:      return GetAsyncKeyState(VK_DECIMAL) & 0x8000;
	case Key::Numpad1:            return GetAsyncKeyState(VK_NUMPAD1) & 0x8000;
	case Key::Numpad2:            return GetAsyncKeyState(VK_NUMPAD2) & 0x8000;
	case Key::Numpad3:            return GetAsyncKeyState(VK_NUMPAD3) & 0x8000;
	case Key::Numpad4:            return GetAsyncKeyState(VK_NUMPAD4) & 0x8000;
	case Key::Numpad5:            return GetAsyncKeyState(VK_NUMPAD5) & 0x8000;
	case Key::Numpad6:            return GetAsyncKeyState(VK_NUMPAD6) & 0x8000;
	case Key::Numpad7:            return GetAsyncKeyState(VK_NUMPAD7) & 0x8000;
	case Key::Numpad8:            return GetAsyncKeyState(VK_NUMPAD8) & 0x8000;
	case Key::Numpad9:            return GetAsyncKeyState(VK_NUMPAD9) & 0x8000;
	case Key::Numpad0:            return GetAsyncKeyState(VK_NUMPAD0) & 0x8000;
	case Key::NonUsBackslash:     return GetAsyncKeyState(VK_OEM_102) & 0x8000;
	case Key::Application:        return GetAsyncKeyState(VK_APPS) & 0x8000;
	case Key::Execute:            return GetAsyncKeyState(VK_EXECUTE) & 0x8000;
	case Key::ModeChange:         return GetAsyncKeyState(VK_MODECHANGE) & 0x8000;
	case Key::Help:               return GetAsyncKeyState(VK_HELP) & 0x8000;
	case Key::Menu:               return GetAsyncKeyState(VK_MENU) & 0x8000;
	case Key::Select:             return GetAsyncKeyState(VK_SELECT) & 0x8000;
	case Key::VolumeMute:         return GetAsyncKeyState(VK_VOLUME_MUTE) & 0x8000;
	case Key::VolumeUp:           return GetAsyncKeyState(VK_VOLUME_UP) & 0x8000;
	case Key::VolumeDown:         return GetAsyncKeyState(VK_VOLUME_DOWN) & 0x8000;
	case Key::MediaPlayPause:     return GetAsyncKeyState(VK_MEDIA_PLAY_PAUSE) & 0x8000;
	case Key::MediaStop:          return GetAsyncKeyState(VK_MEDIA_STOP) & 0x8000;
	case Key::MediaNextTrack:     return GetAsyncKeyState(VK_MEDIA_NEXT_TRACK) & 0x8000;
	case Key::MediaPreviousTrack: return GetAsyncKeyState(VK_MEDIA_PREV_TRACK) & 0x8000;
	case Key::LControl:           return GetAsyncKeyState(VK_LCONTROL) & 0x8000;
	case Key::LShift:             return GetAsyncKeyState(VK_LSHIFT) & 0x8000;
	case Key::LAlt:               return GetAsyncKeyState(VK_LMENU) & 0x8000;
	case Key::LSystem:            return GetAsyncKeyState(VK_LWIN) & 0x8000;
	case Key::RControl:           return GetAsyncKeyState(VK_RCONTROL) & 0x8000;
	case Key::RShift:             return GetAsyncKeyState(VK_RSHIFT) & 0x8000;
	case Key::RAlt:               return GetAsyncKeyState(VK_RMENU) & 0x8000;
	case Key::RSystem:            return GetAsyncKeyState(VK_RWIN) & 0x8000;
	case Key::Back:               return GetAsyncKeyState(VK_BACK) & 0x8000;
	case Key::Forward:            return GetAsyncKeyState(VK_BROWSER_FORWARD) & 0x8000;
	case Key::Refresh:            return GetAsyncKeyState(VK_BROWSER_REFRESH) & 0x8000;
	case Key::Stop:               return GetAsyncKeyState(VK_BROWSER_STOP) & 0x8000;
	case Key::Search:             return GetAsyncKeyState(VK_BROWSER_SEARCH) & 0x8000;
	case Key::Favorites:          return GetAsyncKeyState(VK_BROWSER_FAVORITES) & 0x8000;
	case Key::HomePage:           return GetAsyncKeyState(VK_BROWSER_HOME) & 0x8000;
	case Key::LaunchApplication1: return GetAsyncKeyState(VK_LAUNCH_APP1) & 0x8000;
	case Key::LaunchApplication2: return GetAsyncKeyState(VK_LAUNCH_APP2) & 0x8000;
	case Key::LaunchMail:         return GetAsyncKeyState(VK_LAUNCH_MAIL) & 0x8000;
	case Key::LaunchMediaSelect:  return GetAsyncKeyState(VK_LAUNCH_MEDIA_SELECT) & 0x8000;
	}

	VERA_ASSERT_MSG(false, "unknown key");
	return false;
}

bool Keyboard::isUp(Key key) VERA_NOEXCEPT
{
	return !isDown(key);
}

Mod Keyboard::mods() VERA_NOEXCEPT
{
	uint8_t mods = 0;

	mods |= (GetAsyncKeyState(VK_LCONTROL) & 0x8000) ? Mod::LCtrl : 0;
	mods |= (GetAsyncKeyState(VK_RCONTROL) & 0x8000) ? Mod::RCtrl : 0;
	mods |= (GetAsyncKeyState(VK_LSHIFT) & 0x8000) ? Mod::LShft : 0;
	mods |= (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? Mod::RShft : 0;
	mods |= (GetAsyncKeyState(VK_LMENU) & 0x8000) ? Mod::LAlt : 0;
	mods |= (GetAsyncKeyState(VK_RMENU) & 0x8000) ? Mod::RAlt : 0;
	
	return Mod(mods);
}

bool Keyboard::lctrl() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LCONTROL) & 0x8000;
}

bool Keyboard::rctrl() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RCONTROL) & 0x8000;
}

bool Keyboard::lshft() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LSHIFT) & 0x8000;
}

bool Keyboard::rshft() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RSHIFT) & 0x8000;
}

bool Keyboard::lalt() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LMENU) & 0x8000;
}

bool Keyboard::ralt() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RMENU) & 0x8000;
}

bool Keyboard::lsystem() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_LWIN) & 0x8000;
}

bool Keyboard::rsystem() VERA_NOEXCEPT
{
	return GetAsyncKeyState(VK_RWIN) & 0x8000;
}

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END
#endif // ANOTHER_PLATFORM
