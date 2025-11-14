#pragma once

#include "range.h"
#include <algorithm>
#include <set>
#include <vector>

VERA_NAMESPACE_BEGIN

template <class T>
class ranged_set
{
public:
	using value_type     = basic_range<T>;
	using reference      = const value_type&;
	using size_type      = size_t;
	using iterator       = typename std::set<value_type>::iterator;
	using const_iterator = typename std::set<value_type>::const_iterator;

	VERA_CONSTEXPR ranged_set() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR ranged_set(T first, T last) VERA_NOEXCEPT
	{
		if (first < last)
			m_ranges.emplace(first, last);
	}

	VERA_CONSTEXPR ranged_set(const value_type& range) VERA_NOEXCEPT
	{
		if (!range.empty())
			m_ranges.insert(range);
	}

	VERA_CONSTEXPR ranged_set(const ranged_set& rhs) VERA_NOEXCEPT
		: m_ranges(rhs.m_ranges) {}

	VERA_CONSTEXPR ranged_set(ranged_set&& rhs) VERA_NOEXCEPT
		: m_ranges(std::move(rhs.m_ranges)) {}

	VERA_CONSTEXPR ranged_set& operator=(const ranged_set& rhs) VERA_NOEXCEPT
	{
		if (this != &rhs)
			m_ranges = rhs.m_ranges;

		return *this;
	}

	VERA_CONSTEXPR ranged_set& operator=(ranged_set&& rhs) VERA_NOEXCEPT
	{
		if (this != &rhs)
			m_ranges = std::move(rhs.m_ranges);

		return *this;
	}

	VERA_CONSTEXPR ranged_set& operator=(const value_type& rhs) VERA_NOEXCEPT
	{
		m_ranges.clear();
		if (!rhs.empty())
			m_ranges.insert(rhs);

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR value_type operator[](size_t idx) const VERA_NOEXCEPT
	{
		return *std::next(m_ranges.begin(), idx);
	}

	VERA_CONSTEXPR void insert(const T value) VERA_NOEXCEPT
	{
		insert(value_type(value, value + 1));
	}

	VERA_CONSTEXPR void insert(const T first, const T last) VERA_NOEXCEPT
	{
		insert(value_type(first, last));
	}

	VERA_CONSTEXPR void insert(const value_type& range) VERA_NOEXCEPT
	{
		if (range.empty()) return;

		T first = range.first();
		T last  = range.last();

		auto it = m_ranges.lower_bound(range);
		if (it != m_ranges.begin()) {
			it = std::prev(it);
		}

		std::set<const_iterator> to_erase;
		while (it != m_ranges.end() && it->first() <= last) {
			if (it->last() >= first) {
				first = std::min(first, it->first());
				last  = std::max(last, it->last());
				to_erase.insert(it);
			}
			++it;
		}

		for (auto erase_it : to_erase) {
			m_ranges.erase(erase_it);
		}
		m_ranges.emplace(first, last);
	}

	VERA_CONSTEXPR bool erase(const T value) VERA_NOEXCEPT
	{
		return erase(value_type(value, value + 1));
	}

	VERA_CONSTEXPR bool erase(const T first, const T last) VERA_NOEXCEPT
	{
		return erase(value_type(first, last));
	}

	VERA_CONSTEXPR bool erase(const value_type& range) VERA_NOEXCEPT
	{
		if (range.empty() || m_ranges.empty())
			return false;

		auto it = m_ranges.lower_bound(range);
		if (it != m_ranges.begin()) {
			it = std::prev(it);
		}

		bool changed = false;
		std::set<const_iterator> to_erase;
		std::set<value_type> to_add;

		while (it != m_ranges.end() && it->first() < range.last()) {
			if (it->last() > range.first()) {
				changed = true;
				to_erase.insert(it);

				if (it->first() < range.first()) {
					to_add.emplace(it->first(), range.first());
				}
				if (it->last() > range.last()) {
					to_add.emplace(range.last(), it->last());
				}
			}
			++it;
		}

		if (!changed) {
			return false;
		}

		for (auto erase_it : to_erase) {
			m_ranges.erase(erase_it);
		}
		m_ranges.insert(to_add.begin(), to_add.end());

		return true;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const T value) const VERA_NOEXCEPT
	{
		if (m_ranges.empty()) {
			return false;
		}
		auto it = m_ranges.lower_bound(value_type(value, value + 1));

		if (it != m_ranges.end() && it->contain(value))
			return true;
		
		if (it != m_ranges.begin())
			return std::prev(it)->contain(value);

		return false;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const value_type& range) const VERA_NOEXCEPT
	{
		if (m_ranges.empty() || range.empty()) {
			return false;
		}
		auto it = m_ranges.lower_bound(range);

		if (it != m_ranges.end() && it->contain(range))
			return true;

		if (it != m_ranges.begin()) {
			auto prev_it = std::prev(it);
			if(prev_it->contain(range))
				return true;
		}

		return false;
	}

	// TODO: implement ranged_set::make_union()
	VERA_NODISCARD VERA_CONSTEXPR ranged_set<T> make_union(const ranged_set<T>& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(false, "not implemented yet");
		return {};
	}

	VERA_NODISCARD VERA_CONSTEXPR ranged_set<T> make_intersect(const ranged_set<T>& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(false, "not implemented yet");
		return {};
	}

	VERA_NODISCARD VERA_CONSTEXPR ranged_set<T> make_difference(const ranged_set<T>& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(false, "not implemented yet");
		return {};
	}

	VERA_CONSTEXPR void operator+=(const T value) VERA_NOEXCEPT
	{
		insert(value);
	}

	VERA_CONSTEXPR void operator+=(const value_type& range) VERA_NOEXCEPT
	{
		insert(range);
	}

	VERA_CONSTEXPR void operator-=(const T value) VERA_NOEXCEPT
	{
		(void)erase(value);
	}

	VERA_CONSTEXPR void operator-=(const value_type& range) VERA_NOEXCEPT
	{
		(void)erase(range);
	}

	VERA_CONSTEXPR void clear() VERA_NOEXCEPT
	{
		m_ranges.clear();
	}

	VERA_NODISCARD VERA_CONSTEXPR size_type size() const VERA_NOEXCEPT
	{
		return m_ranges.size();
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_ranges.empty();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator begin() const VERA_NOEXCEPT
	{
		return m_ranges.begin();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator end() const VERA_NOEXCEPT
	{
		return m_ranges.end();
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() VERA_NOEXCEPT
	{
		return m_ranges.begin();
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() VERA_NOEXCEPT
	{
		return m_ranges.end();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cbegin() const VERA_NOEXCEPT
	{
		return m_ranges.begin();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cend() const VERA_NOEXCEPT
	{
		return m_ranges.end();
	}

private:
	std::set<value_type> m_ranges;
};

VERA_NAMESPACE_END