#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include "../../include/vera/os/window.h"
#include "../impl/window_impl.h"

#include "../../include/vera/core/exception.h"
#include <unordered_map>
#include <GLFW/glfw3native.h> 

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

static uint32_t g_window_count = 0;

static Key get_key_from_glfw(int glfw_key)
{
	static const std::unordered_map<int, Key> key_map = {
		{ glfwGetKeyScancode(GLFW_KEY_SPACE), Key::Space },
		{ glfwGetKeyScancode(GLFW_KEY_SPACE), Key::Space },
		{ glfwGetKeyScancode(GLFW_KEY_APOSTROPHE), Key::Apostrophe },
		{ glfwGetKeyScancode(GLFW_KEY_COMMA), Key::Comma },
		{ glfwGetKeyScancode(GLFW_KEY_PERIOD), Key::Period },
		{ glfwGetKeyScancode(GLFW_KEY_SLASH), Key::Slash },
		{ glfwGetKeyScancode(GLFW_KEY_0), Key::Num0 },
		{ glfwGetKeyScancode(GLFW_KEY_1), Key::Num1 },
		{ glfwGetKeyScancode(GLFW_KEY_2), Key::Num2 },
		{ glfwGetKeyScancode(GLFW_KEY_3), Key::Num3 },
		{ glfwGetKeyScancode(GLFW_KEY_4), Key::Num4 },
		{ glfwGetKeyScancode(GLFW_KEY_5), Key::Num5 },
		{ glfwGetKeyScancode(GLFW_KEY_6), Key::Num6 },
		{ glfwGetKeyScancode(GLFW_KEY_7), Key::Num7 },
		{ glfwGetKeyScancode(GLFW_KEY_8), Key::Num8 },
		{ glfwGetKeyScancode(GLFW_KEY_9), Key::Num9 },
		{ glfwGetKeyScancode(GLFW_KEY_SEMICOLON), Key::Semicolon },
		{ glfwGetKeyScancode(GLFW_KEY_EQUAL), Key::Equal },
		{ glfwGetKeyScancode(GLFW_KEY_A), Key::A },
		{ glfwGetKeyScancode(GLFW_KEY_B), Key::B },
		{ glfwGetKeyScancode(GLFW_KEY_C), Key::C },
		{ glfwGetKeyScancode(GLFW_KEY_D), Key::D },
		{ glfwGetKeyScancode(GLFW_KEY_E), Key::E },
		{ glfwGetKeyScancode(GLFW_KEY_F), Key::F },
		{ glfwGetKeyScancode(GLFW_KEY_G), Key::G },
		{ glfwGetKeyScancode(GLFW_KEY_H), Key::H },
		{ glfwGetKeyScancode(GLFW_KEY_I), Key::I },
		{ glfwGetKeyScancode(GLFW_KEY_J), Key::J },
		{ glfwGetKeyScancode(GLFW_KEY_K), Key::K },
		{ glfwGetKeyScancode(GLFW_KEY_L), Key::L },
		{ glfwGetKeyScancode(GLFW_KEY_M), Key::M },
		{ glfwGetKeyScancode(GLFW_KEY_N), Key::N },
		{ glfwGetKeyScancode(GLFW_KEY_O), Key::O },
		{ glfwGetKeyScancode(GLFW_KEY_P), Key::P },
		{ glfwGetKeyScancode(GLFW_KEY_Q), Key::Q },
		{ glfwGetKeyScancode(GLFW_KEY_R), Key::R },
		{ glfwGetKeyScancode(GLFW_KEY_S), Key::S },
		{ glfwGetKeyScancode(GLFW_KEY_T), Key::T },
		{ glfwGetKeyScancode(GLFW_KEY_U), Key::U },
		{ glfwGetKeyScancode(GLFW_KEY_V), Key::V },
		{ glfwGetKeyScancode(GLFW_KEY_W), Key::W },
		{ glfwGetKeyScancode(GLFW_KEY_X), Key::X },
		{ glfwGetKeyScancode(GLFW_KEY_Y), Key::Y },
		{ glfwGetKeyScancode(GLFW_KEY_Z), Key::Z },
		{ glfwGetKeyScancode(GLFW_KEY_LEFT_BRACKET), Key::LBracket },
		{ glfwGetKeyScancode(GLFW_KEY_BACKSLASH), Key::Backslash },
		{ glfwGetKeyScancode(GLFW_KEY_RIGHT_BRACKET), Key::RBracket },
		{ glfwGetKeyScancode(GLFW_KEY_GRAVE_ACCENT), Key::Grave },
		{ glfwGetKeyScancode(GLFW_KEY_WORLD_1), Key::NonUsBackslash },
		{ glfwGetKeyScancode(GLFW_KEY_WORLD_2), Key::NonUsBackslash },
		{ glfwGetKeyScancode(GLFW_KEY_ESCAPE), Key::Escape },
		{ glfwGetKeyScancode(GLFW_KEY_ENTER), Key::Enter },
		{ glfwGetKeyScancode(GLFW_KEY_TAB), Key::Tab },
		{ glfwGetKeyScancode(GLFW_KEY_BACKSPACE), Key::Backspace },
		{ glfwGetKeyScancode(GLFW_KEY_INSERT), Key::Insert },
		{ glfwGetKeyScancode(GLFW_KEY_DELETE), Key::Delete },
		{ glfwGetKeyScancode(GLFW_KEY_RIGHT), Key::Right },
		{ glfwGetKeyScancode(GLFW_KEY_LEFT), Key::Left },
		{ glfwGetKeyScancode(GLFW_KEY_DOWN), Key::Down },
		{ glfwGetKeyScancode(GLFW_KEY_UP), Key::Up },
		{ glfwGetKeyScancode(GLFW_KEY_PAGE_UP), Key::PageUp },
		{ glfwGetKeyScancode(GLFW_KEY_PAGE_DOWN), Key::PageDown },
		{ glfwGetKeyScancode(GLFW_KEY_HOME), Key::Home },
		{ glfwGetKeyScancode(GLFW_KEY_END), Key::End },
		{ glfwGetKeyScancode(GLFW_KEY_CAPS_LOCK), Key::CapsLock },
		{ glfwGetKeyScancode(GLFW_KEY_SCROLL_LOCK), Key::ScrollLock },
		{ glfwGetKeyScancode(GLFW_KEY_NUM_LOCK), Key::NumLock },
		{ glfwGetKeyScancode(GLFW_KEY_PRINT_SCREEN), Key::PrintScreen },
		{ glfwGetKeyScancode(GLFW_KEY_PAUSE), Key::Pause },
		{ glfwGetKeyScancode(GLFW_KEY_F1), Key::F1 },
		{ glfwGetKeyScancode(GLFW_KEY_F2), Key::F2 },
		{ glfwGetKeyScancode(GLFW_KEY_F3), Key::F3 },
		{ glfwGetKeyScancode(GLFW_KEY_F4), Key::F4 },
		{ glfwGetKeyScancode(GLFW_KEY_F5), Key::F5 },
		{ glfwGetKeyScancode(GLFW_KEY_F6), Key::F6 },
		{ glfwGetKeyScancode(GLFW_KEY_F7), Key::F7 },
		{ glfwGetKeyScancode(GLFW_KEY_F8), Key::F8 },
		{ glfwGetKeyScancode(GLFW_KEY_F9), Key::F9 },
		{ glfwGetKeyScancode(GLFW_KEY_F10), Key::F10 },
		{ glfwGetKeyScancode(GLFW_KEY_F11), Key::F11 },
		{ glfwGetKeyScancode(GLFW_KEY_F12), Key::F12 },
		{ glfwGetKeyScancode(GLFW_KEY_F13), Key::F13 },
		{ glfwGetKeyScancode(GLFW_KEY_F14), Key::F14 },
		{ glfwGetKeyScancode(GLFW_KEY_F15), Key::F15 },
		{ glfwGetKeyScancode(GLFW_KEY_F16), Key::F16 },
		{ glfwGetKeyScancode(GLFW_KEY_F17), Key::F17 },
		{ glfwGetKeyScancode(GLFW_KEY_F18), Key::F18 },
		{ glfwGetKeyScancode(GLFW_KEY_F19), Key::F19 },
		{ glfwGetKeyScancode(GLFW_KEY_F20), Key::F20 },
		{ glfwGetKeyScancode(GLFW_KEY_F21), Key::F21 },
		{ glfwGetKeyScancode(GLFW_KEY_F22), Key::F22 },
		{ glfwGetKeyScancode(GLFW_KEY_F23), Key::F23 },
		{ glfwGetKeyScancode(GLFW_KEY_F24), Key::F24 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_0), Key::Num0 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_1), Key::Num1 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_2), Key::Num2 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_3), Key::Num3 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_4), Key::Num4 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_5), Key::Num5 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_6), Key::Num6 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_7), Key::Num7 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_8), Key::Num8 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_9), Key::Num9 },
		{ glfwGetKeyScancode(GLFW_KEY_KP_DECIMAL), Key::NumpadDecimal },
		{ glfwGetKeyScancode(GLFW_KEY_KP_DIVIDE), Key::NumpadDivide },
		{ glfwGetKeyScancode(GLFW_KEY_KP_MULTIPLY), Key::NumpadMultiply },
		{ glfwGetKeyScancode(GLFW_KEY_KP_SUBTRACT), Key::NumpadMinus },
		{ glfwGetKeyScancode(GLFW_KEY_KP_ADD), Key::NumpadPlus },
		{ glfwGetKeyScancode(GLFW_KEY_KP_ENTER), Key::NumpadEnter },
		{ glfwGetKeyScancode(GLFW_KEY_KP_EQUAL), Key::NumpadEqual },
		{ glfwGetKeyScancode(GLFW_KEY_LEFT_SHIFT), Key::LShift },
		{ glfwGetKeyScancode(GLFW_KEY_LEFT_CONTROL), Key::LControl },
		{ glfwGetKeyScancode(GLFW_KEY_LEFT_ALT), Key::LAlt },
		{ glfwGetKeyScancode(GLFW_KEY_LEFT_SUPER), Key::LSystem },
		{ glfwGetKeyScancode(GLFW_KEY_RIGHT_SHIFT), Key::RShift },
		{ glfwGetKeyScancode(GLFW_KEY_RIGHT_CONTROL), Key::RControl },
		{ glfwGetKeyScancode(GLFW_KEY_RIGHT_ALT), Key::RAlt },
		{ glfwGetKeyScancode(GLFW_KEY_RIGHT_SUPER), Key::RSystem },
		{ glfwGetKeyScancode(GLFW_KEY_MENU), Key::Menu }
	};

	auto it = key_map.find(glfw_key);
	return it != key_map.cend() ? it->second : Key::Unknown;
}

static void glfw_window_pos_callback(GLFWwindow* window, int x_pos, int y_pos)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	// TODO: fix bug => on window maximize pos_callback -> maximize_callback -> size_callback
	int2 framebuffer_size;
	glfwGetFramebufferSize(window, &framebuffer_size.x, &framebuffer_size.y);

	//if (impl.swapchain) {
	//	auto& swapchain_impl = CoreObject::getImpl(impl.swapchain);

	//	swapchain_impl.width = framebuffer_size.x;
	//	swapchain_impl.height = framebuffer_size.y;

	//	impl.swapchain->recreate();
	//}

	WindowMoveArgs args = {
		.window = impl.self,
		.pos    = { x_pos, y_pos },
		.delta  = { x_pos - impl.prevPosition.x, y_pos - impl.prevPosition.y }
	};

	impl.prevPosition = { x_pos, y_pos };
	WindowEvent e(WindowEventType::Move, args);

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

	if (impl.swapchain)
		impl.swapchain->recreate();

	WindowResizeArgs args = {
		.window          = impl.self,
		.size            = { (uint32_t)width, (uint32_t)height },
		.framebufferSize = { (uint32_t)framebuffer_size.x, (uint32_t)framebuffer_size.y }
	};

	WindowEvent e(WindowEventType::Resize, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_window_close_callback(GLFWwindow* window)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	impl.needClose = true;

	EmptyArgs args = {
		.window = impl.self
	};

	WindowEvent e(WindowEventType::Close, args);

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

	EmptyArgs args = {
		.window = impl.self
	};

	WindowEvent e(focused ? WindowEventType::Focus : WindowEventType::LostFocus, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_window_iconify_callback(GLFWwindow* window, int iconified)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	EmptyArgs args = {
		.window = impl.self
	};

	WindowEvent e(iconified ? WindowEventType::Minimize : WindowEventType::Restore, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_window_maximize_callback(GLFWwindow* window, int maximized)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	EmptyArgs args = {
		.window = impl.self
	};

	WindowEvent e(maximized ? WindowEventType::Maximize : WindowEventType::Restore, args);

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

	WindowEventType event_type;
	double2         cursor_pos;

	MouseButtonArgs args = {
		.window = impl.self
	};

	glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);
	event_type = action == GLFW_PRESS ? WindowEventType::MouseButtonDown : WindowEventType::MouseButtonUp;

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
		.window = impl.self,
		.pos    = { (int32_t)x_pos, (int32_t)y_pos },
		.delta  = { (int32_t)(impl.prevCursorPosition.x - x_pos), (int32_t)(impl.prevCursorPosition.y - y_pos) }
	};

	impl.prevCursorPosition = { x_pos, y_pos };
	WindowEvent e(WindowEventType::MouseMove, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}

static void glfw_cursor_enter_callback(GLFWwindow* window, int entered)
{
	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	EmptyArgs args = {
		.window = impl.self
	};

	WindowEvent e(entered ? WindowEventType::MouseEnter : WindowEventType::MouseLeave, args);

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
		.window  = impl.self,
		.delta_x = (float)x_off,
		.delta_y = (float)y_off,
		.pos     = { (int32_t)cursor_pos.x, (int32_t)cursor_pos.y }
	};

	WindowEvent e(WindowEventType::MouseWheel, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
}
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_REPEAT) return;

	auto& impl = *(priv::WindowImpl*)glfwGetWindowUserPointer(window);

	KeyboardArgs args = {
		.window    = impl.self,
		.character = 0,
		.key       = get_key_from_glfw(scancode),
		.mods      = Keyboard::mods()
	};

	WindowEventType event_type = action == GLFW_PRESS ?
		WindowEventType::KeyDown :
		WindowEventType::KeyUp;

	WindowEvent e(event_type, args);

	if (impl.eventHandler)
		impl.eventHandler(*impl.self, e);
	else
		impl.eventQueue.push_back(e);
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
	Window()
{
	create(width, height, title);
}

Window::Window(Window&& rhs) noexcept :
	m_impl(std::move(m_impl)) {
	m_impl->self = this;
}

Window::~Window()
{
	destroy();
}

void Window::create(uint32_t width, uint32_t height, std::string_view title)
{
	if (m_impl)
		throw Exception("window already created");

	if (glfwInit() != GLFW_TRUE)
		throw GLFWException();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	
	if (window == nullptr)
		throw GLFWException();

	glfwSetWindowPosCallback(window, glfw_window_pos_callback);
	glfwSetWindowSizeCallback(window, glfw_window_size_callback);
	glfwSetWindowCloseCallback(window, glfw_window_close_callback);
	//glfwSetWindowRefreshCallback(window, glfw_window_refresh_callback);
	glfwSetWindowFocusCallback(window, glfw_window_focus_callback);
	glfwSetWindowIconifyCallback(window, glfw_window_iconify_callback);
	glfwSetWindowMaximizeCallback(window, glfw_window_maximize_callback);
	//glfwSetFramebufferSizeCallback(window, glfw_window_framebuffer_size_callback);
	//glfwSetWindowContentScaleCallback(window, glfw_window_content_scale_callback);
	glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
	glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
	glfwSetCursorEnterCallback(window, glfw_cursor_enter_callback);
	glfwSetScrollCallback(window, glfw_scroll_callback);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetCharCallback(window, glfw_char_callback);
	glfwSetCharModsCallback(window, glfw_char_mods_callback);
	glfwSetDropCallback(window, glfw_drop_callback);
	
	m_impl = new priv::WindowImpl{};
	
	glfwSetWindowUserPointer(window, m_impl);

	m_impl->self         = this;
	m_impl->window       = window;
	m_impl->eventHandler = nullptr;
	glfwGetWindowPos(m_impl->window, &m_impl->prevPosition.x, &m_impl->prevPosition.y);
	glfwGetCursorPos(m_impl->window, &m_impl->prevCursorPosition.x, &m_impl->prevCursorPosition.y);
	glfwShowWindow(m_impl->window);
	m_impl->userPtr      = nullptr;
	m_impl->needClose    = false;

	g_window_count++;
}

void Window::destroy()
{
	if (!m_impl) return;

	glfwDestroyWindow(m_impl->window);
	delete std::exchange(m_impl, nullptr);

	if (--g_window_count == 0)
		glfwTerminate();
}

bool Window::hasEvent() const
{
	return !m_impl->eventQueue.empty();
}

bool Window::waitEvent(WindowEvent& e)
{
	glfwWaitEvents();
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

float Window::getAspect() const VERA_NOEXCEPT
{
	int2 size;
	glfwGetFramebufferSize(m_impl->window, &size.x, &size.y);

	return static_cast<float>(size.x) / size.y;
}

void Window::cancelClose() VERA_NOEXCEPT
{
	m_impl->needClose = false;
}

bool Window::needClose() const VERA_NOEXCEPT
{
	return m_impl->needClose;
}

bool Window::empty() VERA_NOEXCEPT
{
	return m_impl == nullptr;
}

void* Window::getNativeHandle() const VERA_NOEXCEPT
{
	return glfwGetWin32Window(m_impl->window);
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