#include "../../include/vera/os/keyboard.h"

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

// TODO: implement keyboard
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

Key Keyboard::toKey(Scancode scancode)
{
	return Key();
}

Scancode Keyboard::toScancode(Key key)
{
	return Scancode();
}

bool Keyboard::isDown(Key key)
{
	switch (key) {
	case Key::Up:     return GetAsyncKeyState(VK_UP);
	case Key::Down:   return GetAsyncKeyState(VK_DOWN);
	case Key::Left:   return GetAsyncKeyState(VK_LEFT);
	case Key::Right:  return GetAsyncKeyState(VK_RIGHT);
	case Key::Space:  return GetAsyncKeyState(VK_SPACE);
	case Key::LShift: return GetAsyncKeyState(VK_LSHIFT);
	}
	return bool();
}

bool Keyboard::isUp(Key key)
{
	return bool();
}

bool Keyboard::isDown(Scancode scancode)
{
	switch (scancode) {
	case Scancode::W:      return GetAsyncKeyState('W') & 0x8000;
	case Scancode::A:      return GetAsyncKeyState('A') & 0x8000;
	case Scancode::S:      return GetAsyncKeyState('S') & 0x8000;
	case Scancode::D:      return GetAsyncKeyState('D') & 0x8000;
	case Scancode::Up:     return GetAsyncKeyState(VK_UP) & 0x8000;
	case Scancode::Down:   return GetAsyncKeyState(VK_DOWN) & 0x8000;
	case Scancode::Left:   return GetAsyncKeyState(VK_LEFT) & 0x8000;
	case Scancode::Right:  return GetAsyncKeyState(VK_RIGHT) & 0x8000;
	case Scancode::Space:  return GetAsyncKeyState(VK_SPACE) & 0x8000;
	case Scancode::LShift: return GetAsyncKeyState(VK_LSHIFT) & 0x8000;
	}
	return bool();
}

bool Keyboard::isUp(Scancode scancode)
{
	return bool();
}

bool Keyboard::ctrl()
{
	return bool();
}

bool Keyboard::lctrl()
{
	return bool();
}

bool Keyboard::rctrl()
{
	return bool();
}

bool Keyboard::shft()
{
	return bool();
}

bool Keyboard::lshft()
{
	return bool();
}

bool Keyboard::rshft()
{
	return bool();
}

bool Keyboard::alt()
{
	return bool();
}

bool Keyboard::lalt()
{
	return bool();
}

bool Keyboard::ralt()
{
	return bool();
}

bool Keyboard::system()
{
	return bool();
}

bool Keyboard::lsystem()
{
	return bool();
}

bool Keyboard::rsystem()
{
	return bool();
}

#endif

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END
