#pragma once

#include "../core/coredefs.h"
#include <chrono>

VERA_NAMESPACE_BEGIN

class Timer
{
	using clock_t = std::chrono::high_resolution_clock;
	using time_t  = clock_t::time_point;
public:
	Timer() :
		m_global_begin(clock_t::now()),
		m_frame_begin(m_global_begin) {}

	VERA_INLINE void update() VERA_NOEXCEPT
	{
		m_frame_begin = clock_t::now();
	}

	VERA_NODISCARD VERA_INLINE float elapsed() const VERA_NOEXCEPT
	{
		return elapsed_ns() / 1e9f;
	}

	VERA_NODISCARD VERA_INLINE float elapsed_ms() const VERA_NOEXCEPT
	{
		return elapsed_ns() / 1e6f;
	}

	VERA_NODISCARD VERA_INLINE float elapsed_us() const VERA_NOEXCEPT
	{
		return elapsed_ns() / 1e3f;
	}

	VERA_NODISCARD VERA_INLINE float elapsed_ns() const VERA_NOEXCEPT
	{
		using namespace std::chrono;

		uint64_t count = duration_cast<nanoseconds>(clock_t::now() - m_global_begin).count();

		return static_cast<float>(count);
	}

	VERA_NODISCARD VERA_INLINE float dt() const VERA_NOEXCEPT
	{
		return dt_ns() / 1e9f;
	}

	VERA_NODISCARD VERA_INLINE float dt_ms() const VERA_NOEXCEPT
	{
		return dt_ns() / 1e6f;
	}

	VERA_NODISCARD VERA_INLINE float dt_us() const VERA_NOEXCEPT
	{
		return dt_ns() / 1e3f;
	}

	VERA_NODISCARD VERA_INLINE float dt_ns() const VERA_NOEXCEPT
	{
		using namespace std::chrono;

		uint64_t count = duration_cast<nanoseconds>(clock_t::now() - m_frame_begin).count();
		
		return static_cast<float>(count);
	}

	VERA_NODISCARD VERA_INLINE float frame() const VERA_NOEXCEPT
	{
		return 1e9f / dt_ns();
	}

private:
	time_t m_global_begin;
	time_t m_frame_begin;
};

VERA_NAMESPACE_END
