#pragma once

#include "../core/assertion.h"
#include <initializer_list>
#include <type_traits>

VERA_NAMESPACE_BEGIN

template <typename T>
class array_view
{
public:
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
	VERA_CONSTEXPR array_view(T(&ptr)[C]) VERA_NOEXCEPT :
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
		m_size(static_cast<uint32_t>(v.size())) {}

	template <class V,
		typename std::enable_if_t<std::is_convertible_v<decltype(std::declval<V>().begin()), const T*> &&
		std::is_convertible_v<decltype(std::declval<V>().size()), std::size_t> &&
		std::is_lvalue_reference_v<V>, int> = 0>
	VERA_CONSTEXPR array_view(V&& v) VERA_NOEXCEPT :
		m_ptr(v.begin()),
		m_size(static_cast<uint32_t>(v.size())) {}

	VERA_NODISCARD VERA_CONSTEXPR const T& operator[](size_t idx) const VERA_NOEXCEPT
	{
		VERA_ASSERT(m_ptr && idx < m_size);
		return *(m_ptr + idx);
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* begin() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* end() const VERA_NOEXCEPT
	{
		return m_ptr + m_size;
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
		return !m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return m_size;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T* data() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

private:
	const T* m_ptr;
	size_t   m_size;
};

VERA_NAMESPACE_END