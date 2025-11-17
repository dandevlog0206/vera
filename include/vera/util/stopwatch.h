#pragma once

#include "../core/coredefs.h"
#include <chrono>

VERA_NAMESPACE_BEGIN

class StopWatch
{
public:
	using clock_t      = std::chrono::high_resolution_clock;
	using time_point_t = clock_t::time_point;

	VERA_FORCEINLINE StopWatch() VERA_NOEXCEPT
	{
		reset();
	}

	VERA_FORCEINLINE void start() VERA_NOEXCEPT
	{
		m_ticking = true;
		m_start   = clock_t::now();
	}

	VERA_FORCEINLINE void stop() VERA_NOEXCEPT
	{
		m_end     = clock_t::now();
		m_ticking = false;
	}

	VERA_NODISCARD VERA_FORCEINLINE float get_s() const VERA_NOEXCEPT
	{
		time_point_t now = clock_t::now();
		
		if (m_ticking) {
			return 1e-3f * std::chrono::duration<float, std::milli>(now - m_start).count();
		} else {
			return 1e-3f * std::chrono::duration<float, std::milli>(m_end - m_start).count();
		}
	}

	VERA_NODISCARD VERA_FORCEINLINE float get_ms() const VERA_NOEXCEPT
	{
		time_point_t now = clock_t::now();
		
		if (m_ticking) {
			return 1e-3f * std::chrono::duration<float, std::micro>(now - m_start).count();
		} else {
			return 1e-3f * std::chrono::duration<float, std::micro>(m_end - m_start).count();
		}
	}

	VERA_NODISCARD VERA_FORCEINLINE float get_us() const VERA_NOEXCEPT
	{
		time_point_t now = clock_t::now();
		
		if (m_ticking) {
			return 1e-3f * std::chrono::duration<float, std::nano>(now - m_start).count();
		} else {
			return 1e-3f * std::chrono::duration<float, std::nano>(m_end - m_start).count();
		}
	}

	VERA_NODISCARD VERA_FORCEINLINE float get_ns() const VERA_NOEXCEPT
	{
		time_point_t now = clock_t::now();
		
		if (m_ticking) {
			return std::chrono::duration<float, std::nano>(now - m_start).count();
		} else {
			return std::chrono::duration<float, std::nano>(m_end - m_start).count();
		}
	}

	VERA_FORCEINLINE void reset() VERA_NOEXCEPT
	{
		m_ticking = false;
		m_start   = clock_t::now();
		m_end     = m_start;
	}

private:
	time_point_t m_start;
	time_point_t m_end;
	bool         m_ticking;
};

VERA_NAMESPACE_END