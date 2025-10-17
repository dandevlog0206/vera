#pragma once

#include "../core/assertion.h"
#include <type_traits>
#include <iterator>
#include <limits>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

template <class RangeType>
class range_iterator
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type        = typename RangeType::value_type;
	using difference_type   = std::ptrdiff_t;

	VERA_CONSTEXPR range_iterator() VERA_NOEXCEPT :
		value(),
		step() {}

	VERA_CONSTEXPR range_iterator(value_type value, value_type step) VERA_NOEXCEPT :
		value(value),
		step(step) {}

	VERA_NODISCARD VERA_CONSTEXPR value_type operator*() const VERA_NOEXCEPT
	{
		return value;
	}

	VERA_CONSTEXPR range_iterator& operator++() VERA_NOEXCEPT
	{
		value += step;
		return *this;
	}

	VERA_CONSTEXPR range_iterator operator++(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		++*this;
		return temp;
	}

	VERA_CONSTEXPR range_iterator& operator--() VERA_NOEXCEPT
	{
		value -= step;
		return *this;
	}

	VERA_CONSTEXPR range_iterator operator--(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		--*this;
		return temp;
	}

	VERA_CONSTEXPR range_iterator& operator+=(difference_type n) VERA_NOEXCEPT
	{
		value += step * n;
		return *this;
	}

	VERA_CONSTEXPR range_iterator& operator-=(difference_type n) VERA_NOEXCEPT
	{
		value -= step * n;
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR range_iterator operator+(difference_type n) const VERA_NOEXCEPT
	{
		auto temp = *this;
		temp += n;
		return temp;
	}

	VERA_NODISCARD VERA_CONSTEXPR difference_type operator-(const range_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT(step == rhs.step);
		return (value - rhs.value) / step;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const range_iterator& rhs) const VERA_NOEXCEPT
	{
		return value == rhs.value;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const range_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const range_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT(step == rhs.step);
		return 0 < step ? value < rhs.value : rhs.value < value;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const range_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this > rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const range_iterator& rhs) const VERA_NOEXCEPT
	{
		return rhs < *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const range_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this < rhs);
	}

private:
	value_type value;
	value_type step;
};

template <class RangeType>
class range_floating_point_iterator : public range_iterator<RangeType>
{
	using my_base = range_iterator<RangeType>;
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type        = typename RangeType::value_type;
	using difference_type   = std::ptrdiff_t;

	using my_base::my_base;

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const range_floating_point_iterator& rhs) const VERA_NOEXCEPT
	{
		static VERA_CONSTEXPR auto eps = std::numeric_limits<value_type>::epsilon() * 4;

		if (this->step > 0) {
			return this->value + eps >= rhs.value && rhs.value + eps >= this->value;
		} else if (this->step < 0) {
			return this->value - eps <= rhs.value && rhs.value - eps <= this->value;
		}
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const range_floating_point_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}
};

template <class RangeType>
class basic_range_iterator
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type        = typename RangeType::value_type;
	using size_type         = typename RangeType::size_type;
	using difference_type   = typename RangeType::difference_type;

	VERA_CONSTEXPR basic_range_iterator() VERA_NOEXCEPT :
		value() {}

	VERA_CONSTEXPR basic_range_iterator(value_type value) VERA_NOEXCEPT :
		value(value) {}

	VERA_NODISCARD VERA_CONSTEXPR value_type operator*() const VERA_NOEXCEPT
	{
		return value;
	}

	VERA_CONSTEXPR basic_range_iterator& operator++() VERA_NOEXCEPT
	{
		value++;
		return *this;
	}

	VERA_CONSTEXPR basic_range_iterator operator++(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		++*this;
		return temp;
	}

	VERA_CONSTEXPR basic_range_iterator& operator--() VERA_NOEXCEPT
	{
		value--;
		return *this;
	}

	VERA_CONSTEXPR basic_range_iterator operator--(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		--*this;
		return temp;
	}

	VERA_CONSTEXPR basic_range_iterator& operator+=(difference_type n) VERA_NOEXCEPT
	{
		value += n;
		return *this;
	}

	VERA_CONSTEXPR basic_range_iterator& operator-=(difference_type n) VERA_NOEXCEPT
	{
		value -= n;
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR basic_range_iterator operator+(difference_type n) const VERA_NOEXCEPT
	{
		auto temp = *this;
		temp += n;
		return temp;
	}

	VERA_NODISCARD VERA_CONSTEXPR difference_type operator-(const basic_range_iterator& rhs) const VERA_NOEXCEPT
	{
		return (value - rhs.value);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const basic_range_iterator& rhs) const VERA_NOEXCEPT
	{
		return value == rhs.value;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const basic_range_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const basic_range_iterator& rhs) const VERA_NOEXCEPT
	{
		return value < rhs.value;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const basic_range_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this > rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const basic_range_iterator& rhs) const VERA_NOEXCEPT
	{
		return rhs < *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const basic_range_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this < rhs);
	}

private:
	value_type value;
};

VERA_PRIV_NAMESPACE_END

// A simple range class that works like Python's range(), [first, last)
template <class T>
class range
{
public:
	using value_type = T;
	using iterator   = std::conditional_t<std::is_floating_point_v<value_type>,
		priv::range_floating_point_iterator<range<T>>,
		priv::range_iterator<range<T>>>;

	VERA_CONSTEXPR range() VERA_NOEXCEPT {}

	VERA_CONSTEXPR range(T first, T last) VERA_NOEXCEPT :
		first(first), last(last), step(first < last ? make_one() : -make_one()) {}

	VERA_CONSTEXPR range(T first, T last, T step) VERA_NOEXCEPT :
		first(first), last(last), step(step)
	{
		VERA_ASSERT((first < last && 0 < step) || (last < first && step < 0));
	}

	VERA_NODISCARD VERA_CONSTEXPR range make_union(const range& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT(step == rhs.step || step == 0 || rhs.step == 0);

		if (empty()) return rhs;
		if (rhs.empty()) return *this;
		
		return {
			first < rhs.first ? first : rhs.first,
			last > rhs.last ? last : rhs.last,
			step != 0 ? step : rhs.step
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR bool intersect(const range& rhs) const VERA_NOEXCEPT
	{
		return first < rhs.last && rhs.first < last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(value_type value) const VERA_NOEXCEPT
	{
		return first <= value && value < last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const range& rhs) const VERA_NOEXCEPT
	{
		return first <= rhs.first && rhs.last <= last;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		if (step == 0 || (first < last) != (step > 0)) return 0;
		return static_cast<size_t>((last - first + step - (step > 0 ? make_one() : -make_one())) / step);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return first == last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const range& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
		return first == rhs.first && last == rhs.last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const range& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const range& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
		return 0 < step ? last <= rhs.first : first <= rhs.last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const range& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
		return 0 < step ? rhs.last <= first : rhs.first <= last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const range& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
		return 0 < step ? first <= rhs.first : last <= rhs.last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const range& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
		return 0 < step ? rhs.first <= first : rhs.last <= last;
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() const VERA_NOEXCEPT
	{
		return iterator(first, step);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() const VERA_NOEXCEPT
	{
		return iterator(last, step);
	}

private:
	VERA_CONSTEXPR T make_one() VERA_NOEXCEPT
	{
		auto val = T();
		return ++val;
	}

private:
	T first;
	T last;
	T step;
};

//template <class T>
//class range
//{
//public:
//	using value_type = T;
//	using iterator   = std::conditional_t<std::is_floating_point_v<value_type>,
//		priv::range_floating_point_iterator<range<T>>,
//		priv::range_iterator<range<T>>>;
//
//	VERA_CONSTEXPR range() VERA_NOEXCEPT {}
//
//	VERA_CONSTEXPR range(T first, T last) VERA_NOEXCEPT :
//		first(first), last(last), step(make_one)
//	{
//		VERA_ASSERT_MSG(first <= last, "cannot have reversed range on unsigned types");
//	}
//
//	VERA_CONSTEXPR range(T first, T last, T step) VERA_NOEXCEPT :
//		first(first), last(last), step(step)
//	{
//		VERA_ASSERT_MSG(first <= last && 0 < step, "cannot have reversed range on unsigned types");
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR range make_union(const range& rhs) const VERA_NOEXCEPT
//	{
//		VERA_ASSERT(step == rhs.step || step == 0 || rhs.step == 0);
//
//		if (empty()) return rhs;
//		if (rhs.empty()) return *this;
//		
//		return {
//			first < rhs.first ? first : rhs.first,
//			last > rhs.last ? last : rhs.last,
//			step != 0 ? step : rhs.step
//		};
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool intersect(const range& rhs) const VERA_NOEXCEPT
//	{
//		return first < rhs.last && rhs.first < last;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool contain(value_type value) const VERA_NOEXCEPT
//	{
//		return first <= value && value < last;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool contain(const range& rhs) const VERA_NOEXCEPT
//	{
//		return first <= rhs.first && rhs.last <= last;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
//	{
//		if (step == 0 || first > last) return 0;
//		return static_cast<size_t>((last - first + step - make_one()) / step);
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
//	{
//		return first == last;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const range& rhs) const VERA_NOEXCEPT
//	{
//		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
//		return first == rhs.first && last == rhs.last;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const range& rhs) const VERA_NOEXCEPT
//	{
//		return !(*this == rhs);
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const range& rhs) const VERA_NOEXCEPT
//	{
//		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
//		return last <= rhs.first;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const range& rhs) const VERA_NOEXCEPT
//	{
//		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
//		return rhs.last <= first;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const range& rhs) const VERA_NOEXCEPT
//	{
//		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
//		return first <= rhs.first;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const range& rhs) const VERA_NOEXCEPT
//	{
//		VERA_ASSERT_MSG(step == rhs.step, "cannot compare ranges with different steps");
//		return rhs.first <= first;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR iterator begin() const VERA_NOEXCEPT
//	{
//		return iterator(first, step);
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR iterator end() const VERA_NOEXCEPT
//	{
//		return iterator(last, step);
//	}
//
//private:
//	VERA_CONSTEXPR T make_one() VERA_NOEXCEPT
//	{
//		auto val = T();
//		return ++val;
//	}
//
//private:
//	T first;
//	T last;
//	T step;
//};

template <class T>
class basic_range
{
	template <class T2>
	friend class ranged_set;

	static_assert(std::is_integral_v<T>, "basic_range requires an integral type");
public:
	using value_type      = T;
	using size_type       = size_t;
	using difference_type = ptrdiff_t;
	using iterator        = priv::basic_range_iterator<basic_range<T>>;

	VERA_CONSTEXPR basic_range() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR basic_range(T first, T last) VERA_NOEXCEPT :
		m_first(first),
		m_last(last)
	{
		VERA_ASSERT_MSG(m_first <= m_last, "cannot have reversed range on unsigned types");
	}

	VERA_NODISCARD VERA_CONSTEXPR T first() const VERA_NOEXCEPT
	{
		return m_first;
	}

	VERA_NODISCARD VERA_CONSTEXPR T last() const VERA_NOEXCEPT
	{
		return m_last;
	}

	VERA_NODISCARD VERA_CONSTEXPR basic_range make_union(const basic_range& rhs) const VERA_NOEXCEPT
	{
		if (empty()) return rhs;
		if (rhs.empty()) return *this;

		return {
			m_first < rhs.m_first ? m_first : rhs.m_first,
			m_last > rhs.m_last ? m_last : rhs.m_last
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR bool intersect(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return m_first < rhs.m_last && rhs.m_first < m_last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(value_type value) const VERA_NOEXCEPT
	{
		return m_first <= value && value < m_last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return m_first <= rhs.m_first && rhs.m_last <= m_last;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return static_cast<size_t>(m_last - m_first);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_first == m_last;
	}

	VERA_CONSTEXPR basic_range& operator+=(const T rhs) VERA_NOEXCEPT
	{
		m_first += rhs;
		m_last  += rhs;

		return *this;
	}

	VERA_CONSTEXPR basic_range& operator-=(const T rhs) VERA_NOEXCEPT
	{
		m_first -= rhs;
		m_last  -= rhs;
	
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR basic_range operator+(const T rhs) const VERA_NOEXCEPT
	{
		return { m_first + rhs, m_last + rhs };
	}

	friend 	VERA_NODISCARD VERA_CONSTEXPR basic_range operator+(const T lhs, const basic_range& rhs) VERA_NOEXCEPT
	{
		return { lhs + rhs.m_first, lhs + rhs.m_last };
	}

	VERA_NODISCARD VERA_CONSTEXPR basic_range operator-(const T rhs) const VERA_NOEXCEPT
	{
		return { m_first - rhs, m_last - rhs };
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return m_first == rhs.m_first && m_last == rhs.m_last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const T rhs) const VERA_NOEXCEPT
	{
		return m_last <= rhs;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const T rhs) const VERA_NOEXCEPT
	{
		return rhs < m_first;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const T rhs) const VERA_NOEXCEPT
	{
		return m_first <= rhs;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const T rhs) const VERA_NOEXCEPT
	{
		return rhs <= m_last;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return m_last <= rhs.m_first;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return rhs.m_last <= m_first;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return m_first <= rhs.m_first;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const basic_range& rhs) const VERA_NOEXCEPT
	{
		return rhs.m_first <= m_first;
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() const VERA_NOEXCEPT
	{
		return iterator(m_first);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() const VERA_NOEXCEPT
	{
		return iterator(m_last);
	}

private:
	T m_first;
	T m_last;
};

VERA_NAMESPACE_END
