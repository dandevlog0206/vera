#pragma once

#include "range.h"
#include <algorithm>
#include <vector>

VERA_NAMESPACE_BEGIN

template <class T>
class ranged_set
{
public:
	using value_type     = basic_range<T>;
	using reference      = value_type&;
	using size_type      = size_t;
	using iterator       = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;

	VERA_CONSTEXPR ranged_set() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR ranged_set(T first, T last) VERA_NOEXCEPT
	{
		m_ranges.emplace_back(first, last);
	}

	VERA_CONSTEXPR ranged_set(const value_type& range) VERA_NOEXCEPT
	{
		if (!range.empty())
			m_ranges.push_back(range);
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
		m_ranges.push_back(rhs);

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR const value_type& operator[](size_t idx) const VERA_NOEXCEPT
	{
		return m_ranges[idx];
	}

	VERA_CONSTEXPR void insert(const T value) VERA_NOEXCEPT
	{
		if (m_ranges.empty()) {
			m_ranges.emplace_back(value, value + 1);
			return;
		}

		auto it = std::lower_bound(VERA_SPAN(m_ranges), value,
			[](const value_type& range, T val) {
				return range < val;
			});

		if (it != m_ranges.end() && it->contain(value)) return;

		if (it != m_ranges.begin()) {
			auto prev = std::prev(it);

			if (prev->last() == value) {
				if (it != m_ranges.end() && it->first() == value + 1) {
					prev->m_last = it->last();
					m_ranges.erase(it);
				} else {
					prev->m_last += 1;
				}

				return;
			}
		}

		if (it != m_ranges.end() && it->first() == value + 1) {
			it->m_first -= 1;
			return;
		}

		m_ranges.emplace(it, value, value + 1);
	}

	VERA_CONSTEXPR void insert(const T first, const T last) VERA_NOEXCEPT
	{
		insert(value_type(first, last));
	}

	VERA_CONSTEXPR void insert(const value_type& range) VERA_NOEXCEPT
	{
		if (range.empty()) return;

		if (m_ranges.empty()) {
			m_ranges.push_back(range);
			return;
		}

		auto it = std::lower_bound(VERA_SPAN(m_ranges), range,
			[](const value_type& range, const value_type& val) {
				return range < val.first();
			});
		
		if (it != m_ranges.end() && it->contain(range)) return;
		
		if (it != m_ranges.begin()) {
			auto prev = std::prev(it);

			if (prev->last() == range.first()) {
				if (it != m_ranges.end() && it->first() == range.last()) {
					prev->m_last = it->last();
					m_ranges.erase(it);
				} else {
					prev->m_last = range.last();
				}

				return;
			}
		}
		
		if (it != m_ranges.end() && it->first() == range.last()) {
			it->m_first = range.first();
			return;
		}

		m_ranges.emplace(it, range);
	}

	VERA_CONSTEXPR bool erase(const T value) VERA_NOEXCEPT
	{
		if (m_ranges.empty()) return false;

		auto it = std::lower_bound(VERA_SPAN(m_ranges), value,
			[](const value_type& range, T val) {
				return range < val;
			});

		if (it == m_ranges.end() || !it->contain(value)) return false;

		if (it->first() == value) {
			if (it->last() == value + 1) {
				m_ranges.erase(it);
			} else {
				it->m_first += 1;
			}
		} else if (it->last() == value + 1) {
			it->m_last -= 1;
		} else {
			T old_last = it->last();
			it->m_last = value;
			m_ranges.emplace(std::next(it), value + 1, old_last);
		}

		return true;
	}

	VERA_CONSTEXPR bool erase(const T first, const T last) VERA_NOEXCEPT
	{
		return erase(value_type(first, last));
	}

	VERA_CONSTEXPR bool erase(const value_type& range) VERA_NOEXCEPT
	{
		if (range.empty() || m_ranges.empty())
			return false;

		auto it = std::lower_bound(m_ranges.begin(), m_ranges.end(), range.first(),
			[](const value_type& r, const T& val) {
				return r.last() <= val;
			});

		if (it == m_ranges.end() || it->first() >= range.last())
			return false;

		auto first_affected = it;
		auto last_affected  = std::lower_bound(first_affected, m_ranges.end(), range.last(),
			[](const value_type& r, const T& val) {
				return r.first() < val;
			});

		bool first_partially_covered = first_affected->first() < range.first();
		bool last_partially_covered  = std::prev(last_affected)->last() > range.last();

		if (first_partially_covered && last_partially_covered && std::distance(first_affected, last_affected) == 1) {
			T old_last = first_affected->last();
			first_affected->m_last = range.first();
			m_ranges.emplace(std::next(first_affected), range.last(), old_last);
		} else {
			auto erase_begin = first_affected;
			auto erase_end   = last_affected;

			if (first_partially_covered) {
				first_affected->m_last = range.first();
				erase_begin = std::next(first_affected);
			}

			if (last_partially_covered) {
				std::prev(last_affected)->m_first = range.last();
				erase_end = std::prev(last_affected);
			}

			if (erase_begin < erase_end) {
				m_ranges.erase(erase_begin, erase_end);
			}
		}

		return true;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const T value) const VERA_NOEXCEPT
	{
		auto it = std::lower_bound(VERA_SPAN(m_ranges), value,
			[](const value_type& range, T val) {
				return range < val;
			});

		return it != m_ranges.end() && it->contain(value);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const value_type& range) const VERA_NOEXCEPT
	{
		auto it = std::lower_bound(VERA_SPAN(m_ranges), range,
			[](const value_type& range, T val) {
				return range < val;
			});

		return it != m_ranges.end() && it->contain(range);
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
	std::vector<value_type> m_ranges;
};

VERA_NAMESPACE_END