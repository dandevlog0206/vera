#pragma once

#include "../core/assertion.h"
#include <type_traits>
#include <limits>
#include <map>

VERA_NAMESPACE_BEGIN

template <typename T>
class lookup_table
{
	static_assert(std::is_floating_point_v<T>, "lookup_table only supports floating point types");
public:
	enum interpolation_policy
	{
		LINEAR,
		Quadratic,
		CUBIC
	};

	static const VERA_CONSTEXPR T eps  = std::numeric_limits<T>::epsilon();
	static const VERA_CONSTEXPR T inf  = std::numeric_limits<T>::infinity();
	static const VERA_CONSTEXPR T zero = static_cast<T>(0.);

	using key_type       = T;
	using mapped_type    = T;
	using value_type     = std::pair<T, T>;
	using size_type      = size_t;
	using container      = std::vector<value_type>;
	using iterator       = typename container::iterator;
	using const_iterator = typename container::const_iterator;

	VERA_CONSTEXPR lookup_table() VERA_NOEXCEPT
		: m_table() {}

	VERA_NODISCARD VERA_CONSTEXPR T& operator[](const T x) VERA_NOEXCEPT
	{
		if (m_table.empty())
			return m_table.emplace_back(x, zero).second;

		auto it = std::lower_bound(VERA_SPAN(m_table), x);

		if (it != m_table.cend() && std::abs(it->first - x) < eps)
			return it->second;

		return m_table.insert(it, { x, zero })->second;
	}

	VERA_NODISCARD VERA_CONSTEXPR T operator()(const T x) const VERA_NOEXCEPT
	{
		return eval(x);
	}

	VERA_NODISCARD VERA_CONSTEXPR T eval(const T x) const VERA_NOEXCEPT
	{
		auto lo = std::lower_bound(VERA_SPAN(m_table), x);

		if (lo == m_table.end()) {
			VERA_ASSERT_MSG(!m_table.empty(), "lookup table is empty");
			return m_table.begin()->second;
		}

		auto hi = std::next(lo);

		if (hi == m_table.end())
			return lo->second;

		return lerp(lo, hi, x);
	}

	VERA_CONSTEXPR void reserve(size_t capacity) VERA_NOEXCEPT
	{
		m_table.reserve(capacity);
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t capacity() const VERA_NOEXCEPT
	{
		return m_table.capacity();
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return m_table.size();
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_table.empty();
	}

	VERA_CONSTEXPR void clear_range(const T first, const T last) VERA_NOEXCEPT
	{
		auto lo = std::lower_bound(VERA_SPAN(m_table), first);
		auto hi = std::upper_bound(VERA_SPAN(m_table), last);

		m_table.erase(lo, hi);
	}

	VERA_CONSTEXPR void clear()
	{
		m_table.clear();
		m_table.insert({ -inf, zero });
		m_table.insert({ inf, zero });
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator begin() const VERA_NOEXCEPT
	{
		return m_table.cbegin();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator end() const VERA_NOEXCEPT
	{
		return m_table.cend();
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() VERA_NOEXCEPT
	{
		return m_table.begin();
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() VERA_NOEXCEPT
	{
		return m_table.end();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cbegin() const VERA_NOEXCEPT
	{
		return m_table.cbegin();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cend() const VERA_NOEXCEPT
	{
		return m_table.cend();
	}

private:
	static VERA_NODISCARD VERA_CONSTEXPR T lerp(iterator lo, iterator hi, const T x) VERA_NOEXCEPT
	{
		T t = (x - lo->first) / (hi->first - lo->first);
		return lo->second + t * (hi->second - lo->second);
	}

private:
	std::vector<value_type> m_table;
};

VERA_NAMESPACE_END
