#pragma once

#include "../core/assertion.h"
#include <initializer_list>
#include <type_traits>
#include <iterator>

VERA_NAMESPACE_BEGIN

template <typename T>
class array_view
{
public:
	using value_type       = T;
	using reference        = const T&;
	using size_type        = size_t;
	using iterator         = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;

	VERA_CONSTEXPR array_view() VERA_NOEXCEPT :
		m_ptr(nullptr),
		m_size(0) {}

	VERA_CONSTEXPR array_view(std::nullptr_t) VERA_NOEXCEPT :
		m_ptr(nullptr),
		m_size(0) {}

	template <typename B = T,
		typename std::enable_if_t<std::is_convertible_v<B, T> &&
		std::is_lvalue_reference_v<B>, int> = 0>
	VERA_CONSTEXPR array_view(B&& value) VERA_NOEXCEPT :
		m_ptr(std::addressof(value)),
		m_size(1) {}

	VERA_CONSTEXPR array_view(const T* ptr, size_t size) VERA_NOEXCEPT :
		m_ptr(ptr),
		m_size(size) {}

	template <size_t C>
	VERA_CONSTEXPR array_view(const T(&ptr)[C]) VERA_NOEXCEPT :
		m_ptr(ptr),
		m_size(C) {}

	VERA_CONSTEXPR array_view(std::initializer_list<T> ilist) VERA_NOEXCEPT :
		m_ptr(ilist.begin()),
		m_size(ilist.size()) {}

	template <std::size_t C>
	array_view(T(&& ptr)[C]) = delete;

	template <class V,
		typename std::enable_if_t<!std::is_convertible_v<decltype(std::declval<V>().begin()), const T*> &&
		std::is_convertible_v<decltype(std::declval<V>().data()), const T*> &&
		std::is_convertible_v<decltype(std::declval<V>().size()), std::size_t> &&
		std::is_lvalue_reference_v<V>, int> = 0>
	VERA_CONSTEXPR array_view(V&& v) VERA_NOEXCEPT :
		m_ptr(v.data()),
		m_size(v.size()) {}

	template <class V,
		typename std::enable_if_t<std::is_convertible_v<decltype(std::declval<V>().begin()), const T*> &&
		std::is_convertible_v<decltype(std::declval<V>().size()), std::size_t> &&
		std::is_lvalue_reference_v<V>, int> = 0>
	VERA_CONSTEXPR array_view(V&& v) VERA_NOEXCEPT :
		m_ptr(v.begin()),
		m_size(v.size()) {}

	VERA_NODISCARD VERA_CONSTEXPR const T& operator[](size_t idx) const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && idx < m_size);
		return *(m_ptr + idx);
	}

	VERA_NODISCARD VERA_CONSTEXPR array_view subview(size_t offset) const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && offset <= m_size);
		return array_view<T>(m_ptr + offset, m_size - offset);
	}

	VERA_NODISCARD VERA_CONSTEXPR array_view subview(size_t offset, size_t count) const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && offset + count <= m_size);
		return array_view<T>(m_ptr + offset, count);
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& front() const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && m_size);
		return *m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& back() const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && m_size);
		return *(m_ptr + m_size - 1);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_size == 0;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return m_size;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* data() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* begin() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* end() const VERA_NOEXCEPT
	{
		return m_ptr + m_size;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* cbegin() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* cend() const VERA_NOEXCEPT
	{
		return m_ptr + m_size;
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator rbegin() const VERA_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator rend() const VERA_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator crbegin() const VERA_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator crend() const VERA_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

private:
	const T* m_ptr;
	size_t   m_size;
};

VERA_PRIV_NAMESPACE_BEGIN

template <class MyType>
class indirect_array_view_iterator
{
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type        = MyType::value_type;
	using difference_type   = std::ptrdiff_t;
	using pointer           = const MyType::value_type*;
	using reference         = const MyType::value_type&;

	VERA_CONSTEXPR indirect_array_view_iterator() VERA_NOEXCEPT :
		m_ptr(nullptr) {}

	VERA_CONSTEXPR indirect_array_view_iterator(const value_type* const* ptr) VERA_NOEXCEPT :
		m_ptr(ptr) {}

	VERA_NODISCARD VERA_CONSTEXPR reference operator*() const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && *m_ptr);
		return **m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR pointer operator->() const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && *m_ptr);
		return *m_ptr;
	}

	VERA_CONSTEXPR indirect_array_view_iterator& operator++() VERA_NOEXCEPT
	{
		++m_ptr;
		return *this;
	}

	VERA_CONSTEXPR indirect_array_view_iterator operator++(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		++m_ptr;
		return temp;
	}

	VERA_CONSTEXPR indirect_array_view_iterator& operator--() VERA_NOEXCEPT
	{
		--m_ptr;
		return *this;
	}

	VERA_CONSTEXPR indirect_array_view_iterator operator--(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		--m_ptr;
		return temp;
	}

	VERA_CONSTEXPR indirect_array_view_iterator operator+(difference_type n) const VERA_NOEXCEPT
	{
		return indirect_array_view_iterator(m_ptr + n);
	}

	VERA_CONSTEXPR indirect_array_view_iterator operator-(difference_type n) const VERA_NOEXCEPT
	{
		return indirect_array_view_iterator(m_ptr - n);
	}

	VERA_CONSTEXPR difference_type operator-(const indirect_array_view_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr - rhs.m_ptr;
	}

	VERA_CONSTEXPR indirect_array_view_iterator& operator+=(difference_type n) VERA_NOEXCEPT
	{
		m_ptr += n;
		return *this;
	}

	VERA_CONSTEXPR indirect_array_view_iterator& operator-=(difference_type n) VERA_NOEXCEPT
	{
		m_ptr -= n;
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const indirect_array_view_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr < rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const indirect_array_view_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr <= rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const indirect_array_view_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr > rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const indirect_array_view_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr >= rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const indirect_array_view_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr == rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const indirect_array_view_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr != rhs.m_ptr;
	}

private:
	const value_type* const* m_ptr;
};

VERA_PRIV_NAMESPACE_END

template <class T>
class indirect_array_view
{
public:
	using value_type       = T;
	using pointer          = const T*;
	using reference        = const T&;
	using size_type        = size_t;
	using iterator         = priv::indirect_array_view_iterator<indirect_array_view<T>>;
	using reverse_iterator = std::reverse_iterator<iterator>;

	VERA_CONSTEXPR indirect_array_view() VERA_NOEXCEPT :
		m_ptr(nullptr),
		m_size(0) {}

	VERA_CONSTEXPR indirect_array_view(std::nullptr_t) VERA_NOEXCEPT :
		m_ptr(nullptr),
		m_size(0) {}

	VERA_CONSTEXPR indirect_array_view(const T* const* ptr, size_t size) VERA_NOEXCEPT :
		m_ptr(ptr),
		m_size(size) {}

	template <size_t C>
	VERA_CONSTEXPR indirect_array_view(const T*(&ptr)[C]) VERA_NOEXCEPT :
		m_ptr(ptr),
		m_size(C) {}

	VERA_CONSTEXPR indirect_array_view(std::initializer_list<const T*> ilist) VERA_NOEXCEPT :
		m_ptr(ilist.begin()),
		m_size(ilist.size()) {}

	template <std::size_t C>
	indirect_array_view(T(&& ptr)[C]) = delete;

	template <class V,
		typename std::enable_if_t<!std::is_convertible_v<decltype(std::declval<V>().begin()), const T* const*> &&
		std::is_convertible_v<decltype(std::declval<V>().data()), const T* const*> &&
		std::is_convertible_v<decltype(std::declval<V>().size()), std::size_t> &&
		std::is_lvalue_reference_v<V>, int> = 0>
	VERA_CONSTEXPR indirect_array_view(V&& v) VERA_NOEXCEPT :
		m_ptr(v.data()),
		m_size(v.size()) {}

	template <class V,
		typename std::enable_if_t<std::is_convertible_v<decltype(std::declval<V>().begin()), const T* const*> &&
		std::is_convertible_v<decltype(std::declval<V>().size()), std::size_t> &&
		std::is_lvalue_reference_v<V>, int> = 0>
	VERA_CONSTEXPR indirect_array_view(V&& v) VERA_NOEXCEPT :
		m_ptr(v.begin()),
		m_size(v.size()) {}

	VERA_NODISCARD VERA_CONSTEXPR const T& operator[](size_t idx) const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && idx < m_size && *(m_ptr + idx));
		return **(m_ptr + idx);
	}

	VERA_NODISCARD VERA_CONSTEXPR indirect_array_view subview(size_t offset) const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && offset <= m_size);
		return indirect_array_view<T>(m_ptr + offset, m_size - offset);
	}

	VERA_NODISCARD VERA_CONSTEXPR indirect_array_view subview(size_t offset, size_t count) const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && offset + count <= m_size);
		return indirect_array_view<T>(m_ptr + offset, count);
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& front() const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && m_size && *m_ptr);
		return **m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& back() const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && m_size && *(m_ptr + m_size - 1));
		return **(m_ptr + m_size - 1);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_size == 0;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return m_size;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* const* data() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() const VERA_NOEXCEPT
	{
		return iterator(m_ptr);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() const VERA_NOEXCEPT
	{
		return iterator(m_ptr + m_size);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator cbegin() const VERA_NOEXCEPT
	{
		return iterator(m_ptr);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator cend() const VERA_NOEXCEPT
	{
		return iterator(m_ptr + m_size);
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator rbegin() const VERA_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator rend() const VERA_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator crbegin() const VERA_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	VERA_NODISCARD VERA_CONSTEXPR reverse_iterator crend() const VERA_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

private:
	const T* const* m_ptr;
	size_t          m_size;
};

VERA_NAMESPACE_END