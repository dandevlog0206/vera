#include "../include/vera/os/window.h"
#include "impl/window_impl.h"
#include "impl/swapchain_impl.h"

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

static uint32_t g_window_count = 0;

static void glfw_window_pos_callback(GLFWwindow* window, int x_pos, int y_pos)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	// TODO: fix bug => on window maximize pos_callback -> maximize_callback -> size_callback
	int2 framebuffer_size;
	glfwGetFramebufferSize(window, &framebuffer_size.x, &framebuffer_size.y);

	if (impl.swapchain) {
		auto& swapchain_impl = CoreObject::getImpl(impl.swapchain);

		swapchain_impl.width = framebuffer_size.x;
		swapchain_impl.height = framebuffer_size.y;

		impl.swapchain->recreate();
	}

	WindowMoveArgs args = {
		.pos   = { x_pos, y_pos },
		.delta = { x_pos - impl.prevPosition.x, y_pos - impl.prevPosition.y }
	};

	impl.prevPosition = { x_pos, y_pos };
	WindowEvent e(EventType::Move, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_window_size_callback(GLFWwindow* window, int width, int height)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	int2 framebuffer_size;
	glfwGetFramebufferSize(window, &framebuffer_size.x, &framebuffer_size.y);

	if (impl.swapchain) {
		auto& swapchain_impl = CoreObject::getImpl(impl.swapchain);

		swapchain_impl.width = framebuffer_size.x;
		swapchain_impl.height = framebuffer_size.y;

		impl.swapchain->recreate();
	}

	WindowResizeArgs args = {
		.size            = { (uint32_t)width, (uint32_t)height },
		.framebufferSize = { (uint32_t)framebuffer_size.x, (uint32_t)framebuffer_size.y }
	};

	WindowEvent e(EventType::Resize, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_window_close_callback(GLFWwindow* window)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	WindowEvent e(EventType::Close);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

//static void glfw_window_refresh_callback(GLFWwindow* window)
//{
//    auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);
//}

static void glfw_window_focus_callback(GLFWwindow* window, int focused)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	WindowEvent e(focused ? EventType::Focus : EventType::LostFocus);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_window_iconify_callback(GLFWwindow* window, int iconified)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	WindowEvent e(iconified ? EventType::Minimize : EventType::Restore);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_window_maximize_callback(GLFWwindow* window, int maximized)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	WindowEvent e(maximized ? EventType::Maximize : EventType::Restore);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

//static void glfw_window_framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);
//}

//static void glfw_window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
//{
//    auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);
//}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	MouseButtonArgs args;
	EventType       event_type;
	double2         cursor_pos;

	glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);
	event_type = action == GLFW_PRESS ? EventType::MouseButtonDown : EventType::MouseButtonUp;

	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:   args.button = Mouse::LButton; break;
	case GLFW_MOUSE_BUTTON_MIDDLE: args.button = Mouse::MButton; break;
	case GLFW_MOUSE_BUTTON_RIGHT:  args.button = Mouse::RButton; break;
	default:                       args.button = Mouse::Unknown; break;
	}
	args.pos = { (int32_t)cursor_pos.x, (int32_t)cursor_pos.y };

	WindowEvent e(event_type, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	MouseMoveArgs args = {
		.pos   = { (int32_t)x_pos, (int32_t)y_pos },
		.delta = { (int32_t)(impl.prevCursorPosition.x - x_pos), (int32_t)(impl.prevCursorPosition.y - y_pos) }
	};

	impl.prevCursorPosition = { x_pos, y_pos };
	WindowEvent e(EventType::MouseMove, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_cursor_enter_callback(GLFWwindow* window, int entered)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	WindowEvent e(entered ? EventType::MouseEnter : EventType::MouseLeave);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_scroll_callback(GLFWwindow* window, double x_off, double y_off)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	double2 cursor_pos;
	glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);

	MouseWheelArgs args = {
		.delta_x = (float)x_off,
		.delta_y = (float)y_off,
		.pos     = { (int32_t)cursor_pos.x, (int32_t)cursor_pos.y }
	};

	WindowEvent e(EventType::MouseWheel, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);
}

static void glfw_char_callback(GLFWwindow* window, unsigned int codepoint)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);
}

static void glfw_char_mods_callback(GLFWwindow* window, unsigned int codepoint, int mods)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);
}

static void glfw_drop_callback(GLFWwindow* window, int path_count, const char* paths[])
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);
}

class GLFWException : public Exception
{
public:
	GLFWException() {
		const char* err_msg = nullptr;
		int         err_code = 0;

		err_code = glfwGetError(&err_msg);

		m_msg = std::format("GLFW exception. error code: {:#010x} msg: {}", err_code, err_msg);
	}
};

Window::Window(uint32_t width, uint32_t height, std::string_view title) :
	m_impl(std::make_unique<priv::WindowImpl>())
{
	if (glfwInit() != GLFW_TRUE)
		throw GLFWException();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_impl->window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	
	if (m_impl->window == nullptr)
		throw GLFWException();

	glfwSetWindowUserPointer(m_impl->window, m_impl.get());
	glfwSetWindowPosCallback(m_impl->window, glfw_window_pos_callback);
	glfwSetWindowSizeCallback(m_impl->window, glfw_window_size_callback);
	glfwSetWindowCloseCallback(m_impl->window, glfw_window_close_callback);
	//glfwSetWindowRefreshCallback(m_impl->window, glfw_window_refresh_callback);
	glfwSetWindowFocusCallback(m_impl->window, glfw_window_focus_callback);
	glfwSetWindowIconifyCallback(m_impl->window, glfw_window_iconify_callback);
	glfwSetWindowMaximizeCallback(m_impl->window, glfw_window_maximize_callback);
	//glfwSetFramebufferSizeCallback(m_impl->window, glfw_window_framebuffer_size_callback);
	//glfwSetWindowContentScaleCallback(m_impl->window, glfw_window_content_scale_callback);
	glfwSetMouseButtonCallback(m_impl->window, glfw_mouse_button_callback);
	glfwSetCursorPosCallback(m_impl->window, glfw_cursor_pos_callback);
	glfwSetCursorEnterCallback(m_impl->window, glfw_cursor_enter_callback);
	glfwSetScrollCallback(m_impl->window, glfw_scroll_callback);
	glfwSetKeyCallback(m_impl->window, glfw_key_callback);
	glfwSetCharCallback(m_impl->window, glfw_char_callback);
	glfwSetCharModsCallback(m_impl->window, glfw_char_mods_callback);
	glfwSetDropCallback(m_impl->window, glfw_drop_callback);

	m_impl->self         = this;
	m_impl->eventHandler = nullptr;
	glfwGetWindowPos(m_impl->window, &m_impl->prevPosition.x, &m_impl->prevPosition.y);
	glfwGetCursorPos(m_impl->window, &m_impl->prevCursorPosition.x, &m_impl->prevCursorPosition.y);
	glfwShowWindow(m_impl->window);

	g_window_count++;
}

Window::Window(Window&& rhs) noexcept :
	m_impl(std::move(m_impl)) {
	m_impl->self = this;
}

Window::~Window()
{
	if (--g_window_count == 0)
		glfwTerminate();
}

bool Window::hasEvent() const
{
	return !m_impl->eventQueue.empty();
}

bool Window::waitEvent(WindowEvent& e)
{
	glfwPollEvents();
	return pollEvent(e);
}

bool Window::pollEvent(WindowEvent& e)
{
	glfwPollEvents();

	if (m_impl->eventQueue.empty())
		return false;

	e = m_impl->eventQueue.front();
	m_impl->eventQueue.pop_front();
	return true;
}

void Window::registerEventHandler(WindowEventHandler hanlder)
{
	m_impl->eventHandler = hanlder;
}

void Window::handleEvent()
{
	glfwPollEvents();
}

PROPERTY_GET_IMPL(Window, Title)
{
	return glfwGetWindowTitle(m_impl->window);
}

PROPERTY_SET_IMPL(Window, Title)
{
	glfwSetWindowTitle(m_impl->window, value.c_str());
}

PROPERTY_GET_IMPL(Window, Position)
{
	int2 pos;
	glfwGetWindowPos(m_impl->window, &pos.x, &pos.y);
	
	return pos;
}

PROPERTY_SET_IMPL(Window, Position)
{
	glfwSetWindowPos(m_impl->window, value.x, value.y);
}

PROPERTY_GET_IMPL(Window, WindowSize)
{
	int2 size;
	glfwGetWindowSize(m_impl->window, &size.x, &size.y);

	return { (uint32_t)size.x, (uint32_t)size.y };
}

PROPERTY_SET_IMPL(Window, WindowSize)
{
	glfwSetWindowSize(m_impl->window, value.width, value.height);
}

PROPERTY_GET_IMPL(Window, FramebufferSize)
{
	int2 size;
	glfwGetFramebufferSize(m_impl->window, &size.x, &size.y);

	return { (uint32_t)size.x, (uint32_t)size.y };
}

PROPERTY_GET_IMPL(Window, Visible)
{
	return glfwGetWindowAttrib(m_impl->window, GLFW_VISIBLE);
}

PROPERTY_SET_IMPL(Window, Visible)
{
	if (value)
		glfwShowWindow(m_impl->window);
	else
		glfwHideWindow(m_impl->window);
}

PROPERTY_GET_IMPL(Window, Focussed)
{
	return glfwGetWindowAttrib(m_impl->window, GLFW_FOCUSED);
}

PROPERTY_SET_IMPL(Window, Focussed)
{
	if (value)
		glfwFocusWindow(m_impl->window);
	else
		glfwSetWindowAttrib(m_impl->window, GLFW_FOCUSED, GLFW_FALSE);
}

PROPERTY_GET_IMPL(Window, Minimized)
{
	return glfwGetWindowAttrib(m_impl->window, GLFW_ICONIFIED);
}

PROPERTY_SET_IMPL(Window, Minimized)
{
	if (value)
		glfwIconifyWindow(m_impl->window);
	else
		glfwRestoreWindow(m_impl->window);
}

PROPERTY_GET_IMPL(Window, Maximized)
{
	return glfwGetWindowAttrib(m_impl->window, GLFW_MAXIMIZED);
}

PROPERTY_SET_IMPL(Window, Maximized)
{
	if (value)
		glfwMaximizeWindow(m_impl->window);
	else
		glfwRestoreWindow(m_impl->window);
}

PROPERTY_GET_IMPL(Window, UserPtr)
{
	return m_impl->userPtr;
}

PROPERTY_SET_IMPL(Window, UserPtr)
{
	m_impl->userPtr = value;
}

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END