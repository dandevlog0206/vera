#pragma once

#include "mouse.h"
#include "keyboard.h"
#include <variant>

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

enum class EventType {
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

struct EmptyArgs {};

struct WindowMoveArgs {
	int2 pos;
	int2 delta;
};

struct WindowResizeArgs {
	uint2 size;
	uint2 framebufferSize;
};

struct KeyboardArgs {
	Key      key;
	Scancode scancode;
	Mod      mods;
};

struct MouseMoveArgs {
	int2 pos;
	int2 delta;
};

struct MouseButtonArgs {
	int2          pos;
	Mouse::Button button;
};

struct MouseWheelArgs {
	float delta_x;
	float delta_y;
	int2  pos;
};

template <EventType T> struct event_struct { using type = void; };
template <> struct event_struct<EventType::Empty> { using type = void; };
template <> struct event_struct<EventType::Create> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Close> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Destroy> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Focus> { using type = EmptyArgs; };
template <> struct event_struct<EventType::LostFocus> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Restore> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Minimize> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Maximize> { using type = EmptyArgs; };
template <> struct event_struct<EventType::EnterMove> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Move> { using type = WindowMoveArgs; };
template <> struct event_struct<EventType::ExitMove> { using type = EmptyArgs; };
template <> struct event_struct<EventType::EnterResize> { using type = EmptyArgs; };
template <> struct event_struct<EventType::Resize> { using type = WindowResizeArgs; };
template <> struct event_struct<EventType::ExitResize> { using type = EmptyArgs; };
template <> struct event_struct<EventType::KeyDown> { using type = KeyboardArgs; };
template <> struct event_struct<EventType::KeyUp> { using type = KeyboardArgs; };
template <> struct event_struct<EventType::MouseMove> { using type = MouseMoveArgs; };
template <> struct event_struct<EventType::MouseEnter> { using type = MouseMoveArgs; };
template <> struct event_struct<EventType::MouseLeave> { using type = MouseMoveArgs; };
template <> struct event_struct<EventType::MouseButtonDown> { using type = MouseButtonArgs; };
template <> struct event_struct<EventType::MouseButtonUp> { using type = MouseButtonArgs; };
template <> struct event_struct<EventType::MouseWheel> { using type = MouseWheelArgs; };

class WindowEvent {
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
		m_type(EventType::Empty), m_args() {
	}

	template <class ArgsType>
	VERA_CONSTEXPR WindowEvent(EventType type, const ArgsType& args) VERA_NOEXCEPT :
		m_type(type), m_args(args) {
	}

	VERA_CONSTEXPR WindowEvent(EventType type) VERA_NOEXCEPT :
		m_type(type), m_args(EmptyArgs()) {
	}

	VERA_NODISCARD VERA_CONSTEXPR EventType type() const VERA_NOEXCEPT {
		return m_type;
	}

	template <class ArgsType>
	VERA_NODISCARD VERA_CONSTEXPR ArgsType get_if() const {
		return std::get_if<ArgsType>(m_args);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT {
		return m_type == EventType::Empty;
	}

private:
	EventType m_type;
	variant_t m_args;
};

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END