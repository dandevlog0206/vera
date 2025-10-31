#pragma once

#include "window_event.h"
#include "../util/extent.h"
#include "../util/property.h"
#include <string>
#include <string_view>
#include <memory>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

struct WindowImpl;

VERA_PRIV_NAMESPACE_END

class Swapchain;

VERA_OS_NAMESPACE_BEGIN

class Window;

typedef void (*WindowEventHandler)(Window&, const WindowEvent&);

class Window
{
	friend class ::vr::Swapchain;
	friend class Keyboard;
	friend class Mouse;

	PROPERTY_INIT(Window);
	VERA_NOCOPY(Window);
public:
	Window() = default;
	Window(uint32_t width, uint32_t height, std::string_view title);
	Window(Window&& rhs) noexcept;
	~Window();

	void create(uint32_t width, uint32_t height, std::string_view title);
	void destroy();

	bool hasEvent() const;
	bool waitEvent(WindowEvent& e);
	bool pollEvent(WindowEvent& e);
	void registerEventHandler(WindowEventHandler hanlder);
	void handleEvent();

	float getAspect() const VERA_NOEXCEPT;

	void cancelClose() VERA_NOEXCEPT;
	bool needClose() const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() VERA_NOEXCEPT;

	VERA_NODISCARD void* getNativeHandle() const VERA_NOEXCEPT;

	PROPERTY(std::string, Title);
	PROPERTY(int2, Position);
	PROPERTY(extent2d, WindowSize);
	PROPERTY(extent2d, FramebufferSize, readonly);
	PROPERTY(bool, Visible);
	PROPERTY(bool, Focussed);
	PROPERTY(bool, Minimized);
	PROPERTY(bool, Maximized);
	PROPERTY(void*, UserPtr);

private:
	PROPERTY_GET_SET_DECL(Title);
	PROPERTY_GET_SET_DECL(Position);
	PROPERTY_GET_SET_DECL(WindowSize);
	PROPERTY_GET_DECL(FramebufferSize);
	PROPERTY_GET_SET_DECL(Visible);
	PROPERTY_GET_SET_DECL(Focussed);
	PROPERTY_GET_SET_DECL(Minimized);
	PROPERTY_GET_SET_DECL(Maximized);
	PROPERTY_GET_SET_DECL(UserPtr);

private:
	priv::WindowImpl* m_impl;
};

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END