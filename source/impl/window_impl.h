#pragma once

#include "../../include/vera/core/swapchain.h"
#include "../../include/vera/os/window.h"

#include <format>
#include <deque>
#include <GLFW/glfw3.h>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

class WindowImpl
{
public:
	os::Window*                 self;

	GLFWwindow*                 window;
	std::deque<os::WindowEvent> eventQueue;

	obj<Swapchain>              swapchain;

	os::WindowEventHandler      eventHandler;
	int2                        prevPosition;
	double2                     prevCursorPosition;
	void*                       userPtr;
	bool                        needClose;
};

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END
