#pragma once

#include "../core/assertion.h"
#include "bezier.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class QuadraticPath2D
{
public:
	using curve_type = QuadraticBezier2D;
	using iterator   = std::vector<curve_type>::const_iterator;

	VERA_CONSTEXPR QuadraticPath2D() VERA_NOEXCEPT :
		m_curves() {}

	template <class Iter>
	VERA_CONSTEXPR QuadraticPath2D(Iter first, Iter last) VERA_NOEXCEPT :
		m_curves(first, last) {}

	VERA_NODISCARD VERA_CONSTEXPR const QuadraticBezier2D& operator[](size_t idx) const VERA_NOEXCEPT
	{
		return m_curves[idx];
	}

	VERA_NODISCARD VERA_CONSTEXPR float2 operator()(float t) const VERA_NOEXCEPT
	{
		return m_curves[static_cast<size_t>(t)](fmodf(t, 1.f));
	}

	VERA_NODISCARD VERA_CONSTEXPR float2 evaluate(float t) const VERA_NOEXCEPT
	{
		return (*this)(t);
	}

	VERA_NODISCARD VERA_CONSTEXPR const QuadraticBezier2D& getCurve(size_t idx) const VERA_NOEXCEPT
	{
		return m_curves[idx];
	}

	VERA_NODISCARD VERA_CONSTEXPR const float2& getControlPoint(size_t idx) const VERA_NOEXCEPT
	{
		return m_curves[idx].controlPoint();
	}

	VERA_NODISCARD VERA_CONSTEXPR float2& getControlPoint(size_t idx) VERA_NOEXCEPT
	{
		return m_curves[idx].controlPoint();
	}

	VERA_CONSTEXPR void append(const float2 p0, const float2& p1) VERA_NOEXCEPT
	{
		if (m_curves.empty()) {
			m_curves.emplace_back(p0, 0.5f * (p0 + p1), p1);
			return;
		}

		m_curves.back().p1() = p0;
		m_curves.emplace_back(p0, 0.5f * (p0 + p1), p1);
	}

	VERA_CONSTEXPR void append(const float2 p0, const float2& c, const float2& p1) VERA_NOEXCEPT
	{
		if (m_curves.empty()) {
			m_curves.emplace_back(p0, c, p1);
			return;
		}

		m_curves.back().p1() = p0;
		m_curves.emplace_back(p0, c, p1);
	}

	VERA_CONSTEXPR void append(const QuadraticBezier2D& curve) VERA_NOEXCEPT
	{
		if (m_curves.empty()) {
			m_curves.push_back(curve);
			return;
		}

		m_curves.back().p1() = curve.p0();
		m_curves.push_back(curve);
	}

	VERA_CONSTEXPR void moveTo(const float2& p0) VERA_NOEXCEPT
	{
		m_curves.emplace_back(p0, p0, p0);
	}

	VERA_CONSTEXPR void lineTo(const float2& p1) VERA_NOEXCEPT
	{
		if (m_curves.empty()) {
			m_curves.emplace_back(p1, p1, p1);
			return;
		}

		const auto& last_curve = m_curves.back();
		const auto& p0         = last_curve.p1();
		
		m_curves.emplace_back(p0, 0.5f * (p0 + p1), p1);
	}

	VERA_CONSTEXPR void curveTo(const float2& c, const float2& p1) VERA_NOEXCEPT
	{
		if (m_curves.empty()) {
			m_curves.emplace_back(p1, c, p1);
			return;
		}

		const auto& last_curve = m_curves.back();
		const auto& p0         = last_curve.p1();
		
		m_curves.emplace_back(p0, c, p1);
	}

	VERA_CONSTEXPR void connect(const QuadraticPath2D& path) VERA_NOEXCEPT
	{
		if (path.m_curves.empty()) return;

		if (m_curves.empty()) {
			m_curves = path.m_curves;
			return;
		}

		const auto& last_curve = m_curves.back();
		const auto& p0         = last_curve.p1();
		const auto  offset     = path.m_curves.size() - 1;

		m_curves.insert(
			m_curves.cend(),
			path.m_curves.begin(),
			path.m_curves.end());

		m_curves[offset + 1].p0() = p0;
	}

	VERA_CONSTEXPR void popFront() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!m_curves.empty(), "Cannot pop front from an empty path.");
		m_curves.erase(m_curves.begin());
	}

	VERA_CONSTEXPR void popBack() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!m_curves.empty(), "Cannot pop back from an empty path.");
		m_curves.pop_back();
	}

	VERA_NODISCARD VERA_CONSTEXPR std::pair<QuadraticPath2D, QuadraticPath2D> split(size_t idx) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(idx <= m_curves.size(), "split index is out of range.");

		std::pair<QuadraticPath2D, QuadraticPath2D> result;

		const auto first = m_curves.begin();
		const auto mid   = first + static_cast<std::ptrdiff_t>(idx);
		const auto last  = m_curves.end();

		result.first.m_curves.assign(first, mid);
		result.second.m_curves.assign(mid, last);

		return result;
	}

	VERA_NODISCARD VERA_CONSTEXPR QuadraticPath2D subpath(size_t start_idx, size_t end_idx) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(start_idx <= end_idx && end_idx <= m_curves.size(),
			"subpath indices are out of range.");
		
		QuadraticPath2D result;
		
		const auto first = m_curves.begin() + static_cast<std::ptrdiff_t>(start_idx);
		const auto last  = m_curves.begin() + static_cast<std::ptrdiff_t>(end_idx);
		
		result.m_curves.assign(first, last);
		
		return result;
	}

	VERA_NODISCARD VERA_CONSTEXPR float length(size_t segments_per_curve = 10) const VERA_NOEXCEPT
	{
		float total_length = 0.0f;
	
		for (const auto& curve : m_curves)
			total_length += curve.length(segments_per_curve);
		
		return total_length;
	}

	VERA_CONSTEXPR QuadraticPath2D& operator+=(const float2& p1) VERA_NOEXCEPT
	{
		lineTo(p1);
		return *this;
	}

	VERA_CONSTEXPR QuadraticPath2D& operator+=(const QuadraticBezier2D& curve) VERA_NOEXCEPT
	{
		append(curve);
		return *this;
	}

	VERA_CONSTEXPR QuadraticPath2D& operator+=(const QuadraticPath2D& rhs) VERA_NOEXCEPT
	{
		connect(rhs);
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR QuadraticPath2D operator+(const float2& p1) const VERA_NOEXCEPT
	{
		QuadraticPath2D result = *this;
		result.lineTo(p1);
		return result;
	}

	VERA_NODISCARD VERA_CONSTEXPR QuadraticPath2D& operator+(const QuadraticBezier2D& curve) VERA_NOEXCEPT
	{
		QuadraticPath2D result = *this;
		result.append(curve);
		return result;
	}

	VERA_NODISCARD VERA_CONSTEXPR QuadraticPath2D operator+(const QuadraticPath2D& rhs) const VERA_NOEXCEPT
	{
		QuadraticPath2D result = *this;
		result.connect(rhs);
		return result;
	}

	VERA_CONSTEXPR void reserve(size_t capacity) VERA_NOEXCEPT
	{
		m_curves.reserve(capacity);
	}

	VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return m_curves.size();
	}

	VERA_CONSTEXPR void clear() VERA_NOEXCEPT
	{
		m_curves.clear();
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() const VERA_NOEXCEPT
	{
		return m_curves.cbegin();
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() const VERA_NOEXCEPT
	{
		return m_curves.cend();
	}

private:
	std::vector<QuadraticBezier2D> m_curves;
};

VERA_NAMESPACE_END
