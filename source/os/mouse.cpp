#include "../../include/vera/os/mouse.h"

#include "../../include/vera/core/exception.h"
#include "../impl/window_impl.h"

///////////////////////////////////////////////////////////////////////////////
// Windows ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

void Mouse::setPosition(const int2& pos, const Window& rel_wnd)
{
	double pos_x = static_cast<double>(pos.x);
	double pos_y = static_cast<double>(pos.y);

	glfwSetCursorPos(rel_wnd.m_impl->window, pos_x, pos_y);
}

void Mouse::setPosition(const int2& pos)
{
	if (!SetCursorPos(pos.x, pos.y))
		throw Exception("Failed to set mouse position");
}

int2 Mouse::getPosition(const Window& rel_wnd)
{
	double2 pos;

	glfwGetCursorPos(rel_wnd.m_impl->window, &pos.x, &pos.y);

	return {
		static_cast<int32_t>(pos.x),
		static_cast<int32_t>(pos.y)
	};
}

int2 Mouse::getPosition()
{
	POINT p;

	if (!GetCursorPos(&p))
		throw Exception("Failed to get mouse position");

	return int2{ p.x, p.y };
}

bool Mouse::isDown(Button button)
{
	switch (button) {
	case LButton: return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
	case RButton: return GetAsyncKeyState(VK_RBUTTON) & 0x8000;
	case MButton: return GetAsyncKeyState(VK_MBUTTON) & 0x8000;
	}
	
	VERA_ASSERT_MSG(false, "unknown mouse button");
	return false;
}

bool Mouse::isUp(Button button)
{
	return !isDown(button);
}

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END
#endif // _Win32

///////////////////////////////////////////////////////////////////////////////
// Another Platform ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef ANOTHER_PLATFORM
VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END
#endif // ANOTHER_PLATFORM
