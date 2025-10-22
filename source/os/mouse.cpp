#include "../../include/vera/os/mouse.h"

#include "../impl/window_impl.h"
#include <Windows.h>

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

void Mouse::setPosition(const int2& pos, const Window& rel_wnd)
{
	VERA_ASSERT_MSG(false, "not implemented");
}

void Mouse::setPosition(const int2& pos)
{
	VERA_ASSERT_MSG(false, "not implemented");
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
	VERA_ASSERT_MSG(false, "not implemented");
	return int2{0, 0};
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
