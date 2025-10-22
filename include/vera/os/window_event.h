#pragma once

#include "mouse.h"
#include "keyboard.h"
#include <variant>

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

class Window;

enum class WindowEventType VERA_ENUM
{
	Empty,

	Create,
	Close,
	Destroy,

	Focus,
	LostFocus,

	Restore,
	Minimize,
	Maximize,

	EnterMove,
	Move,
	ExitMove,
	EnterResize,
	Resize,
	ExitResize,

	// keyboard events
	KeyDown,
	KeyUp,
	Char,

	// mouse events
	MouseMove,
	MouseEnter,
	MouseLeave,
	MouseButtonDown,
	MouseButtonUp,
	MouseWheel
};

struct EmptyArgs
{
	Window* window;
};

struct WindowMoveArgs
{
	Window* window;
	int2    pos;
	int2    delta;
};

struct WindowResizeArgs
{
	Window* window;
	uint2   size;
	uint2   framebufferSize;
};

struct KeyboardArgs
{
	Window*  window;
	char32_t character;
	Key      key;
	Mod      mods;
};

struct MouseMoveArgs
{
	Window* window;
	int2    pos;
	int2    delta;
};

struct MouseButtonArgs
{
	Window*       window;
	int2          pos;
	Mouse::Button button;
};

struct MouseWheelArgs
{
	Window* window;
	float   delta_x;
	float   delta_y;
	int2    pos;
};

template <WindowEventType T> struct event_struct { using type = void; };
template <> struct event_struct<WindowEventType::Empty> { using type = void; };
template <> struct event_struct<WindowEventType::Create> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Close> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Destroy> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Focus> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::LostFocus> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Restore> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Minimize> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Maximize> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::EnterMove> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Move> { using type = WindowMoveArgs; };
template <> struct event_struct<WindowEventType::ExitMove> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::EnterResize> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::Resize> { using type = WindowResizeArgs; };
template <> struct event_struct<WindowEventType::ExitResize> { using type = EmptyArgs; };
template <> struct event_struct<WindowEventType::KeyDown> { using type = KeyboardArgs; };
template <> struct event_struct<WindowEventType::KeyUp> { using type = KeyboardArgs; };
template <> struct event_struct<WindowEventType::MouseMove> { using type = MouseMoveArgs; };
template <> struct event_struct<WindowEventType::MouseEnter> { using type = MouseMoveArgs; };
template <> struct event_struct<WindowEventType::MouseLeave> { using type = MouseMoveArgs; };
template <> struct event_struct<WindowEventType::MouseButtonDown> { using type = MouseButtonArgs; };
template <> struct event_struct<WindowEventType::MouseButtonUp> { using type = MouseButtonArgs; };
template <> struct event_struct<WindowEventType::MouseWheel> { using type = MouseWheelArgs; };

class WindowEvent
{
	using variant_t = std::variant<
		EmptyArgs,
		WindowMoveArgs,
		WindowResizeArgs,
		KeyboardArgs,
		MouseMoveArgs,
		MouseButtonArgs,
		MouseWheelArgs>;

public:
	VERA_CONSTEXPR WindowEvent() VERA_NOEXCEPT :
		m_type(WindowEventType::Empty), m_args() {}

	template <class ArgsType>
	VERA_CONSTEXPR WindowEvent(WindowEventType type, const ArgsType& args) VERA_NOEXCEPT :
		m_type(type), m_args(args) {}

	VERA_CONSTEXPR WindowEvent(WindowEventType type) VERA_NOEXCEPT :
		m_type(type), m_args(EmptyArgs()) {}

	VERA_NODISCARD VERA_CONSTEXPR WindowEventType type() const VERA_NOEXCEPT
	{
		return m_type;
	}

	template <class ArgsType>
	VERA_NODISCARD VERA_CONSTEXPR const ArgsType& get_if() const
	{
		return *std::get_if<ArgsType>(&m_args);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_type == WindowEventType::Empty;
	}

private:
	WindowEventType m_type;
	variant_t m_args;
};

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END