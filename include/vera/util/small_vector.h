#pragma once

#include "../core/coredefs.h"
#include "../core/exception.h"
#include "../core/assertion.h"
#include "static_vector.h"

#include <vector>
#include <memory>
#include <utility>
#include <iterator>
#include <algorithm>
#include <new>

VERA_NAMESPACE_BEGIN

template <class T, size_t N = 16, class Allocator = std::allocator<T>>
using small_vector = std::vector<T, Allocator>; // temporary placeholder

//VERA_PRIV_NAMESPACE_BEGIN
//
//template <size_t N, class Allocator>
//class small_vector_storage : public Allocator
//{
//public:
//	using alloc_traits = std::allocator_traits<Allocator>;
//
//	using value_type      = typename alloc_traits::value_type;
//	using pointer         = typename alloc_traits::pointer;
//	using const_pointer   = typename alloc_traits::const_pointer;
//	using reference       = typename alloc_traits::reference;
//	using const_reference = typename alloc_traits::const_reference;
//	using size_type       = typename alloc_traits::size_type;
//	using difference_type = typename alloc_traits::difference_type;
//
//	small_vector_storage() VERA_NOEXCEPT :
//		m_size(0),
//		m_capacity(N) {}
//
//	~small_vector_storage()
//	{
//		if (!is_using_inline_storage())
//			alloc_traits::deallocate(*this, m_storage.ptr, m_capacity);
//	}
//
//	template <class Allocator2>
//	VERA_CONSTEXPR void copy_from(hybrid_vector_storage<Allocator2> rhs)
//	{
//		if (rhs.is_using_inline_storage()) {
//			// copy inline storage
//			m_storage = rhs.m_storage;
//		} else {
//			// allocate and copy heap storage
//			m_size        = rhs.size();
//			m_capacity    = rhs.capacity();
//			m_storage.ptr = std::allocator_traits<Allocator>::allocate(
//				static_cast<Allocator&>(m_storage),
//				other_capacity);
//			
//			std::uninitialized_copy_n(
//				rhs.m_storage.ptr,
//				m_size,
//				m_storage.ptr);
//		}
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_pointer data() const VERA_NOEXCEPT
//	{
//		if (is_using_inline_storage())
//			return reinterpret_cast<const_pointer>(m_storage.buf);
//		else
//			return m_storage.ptr;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR pointer data() VERA_NOEXCEPT
//	{
//		if (is_using_inline_storage())
//			return reinterpret_cast<pointer>(m_storage.buf);
//		else
//			return m_storage.ptr;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR size_type capacity() const VERA_NOEXCEPT
//	{
//		if (is_using_inline_storage())
//			return N;
//		else
//			return m_capacity;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR size_type size() const VERA_NOEXCEPT
//	{
//		return m_size;
//	}
//
//	VERA_NODISCARD bool is_using_inline_storage() const VERA_NOEXCEPT
//	{
//		return m_capacity <= N;
//	}
//
//private:
//	union storage_t
//	{
//		alignas(alignof(value_type)) std::byte buf[sizeof(value_type) * N];
//		pointer ptr;
//	};
//
//	storage_t m_storage;
//	size_type m_size;
//	size_type m_capacity;
//};
//
//VERA_PRIV_NAMESPACE_END
//
//template <class T, size_t N, class Allocator = std::allocator<T>>
//class small_vector
//{
//	using storage_type = priv::small_vector_storage<N, Allocator>;
//	using alloc_traits = std::allocator_traits<Allocator>;
//
//public:
//	using value_type      = T;
//	using allocator_type  = Allocator;
//	using pointer         = typename alloc_traits::pointer;
//	using const_pointer   = typename alloc_traits::const_pointer;
//	using reference       = T&;
//	using const_reference = const T&;
//	using size_type       = size_t;
//	using difference_type = ptrdiff_t;
//
//	using iterator               = pointer;
//	using const_iterator         = const_pointer;
//	using reverse_iterator       = std::reverse_iterator<iterator>;
//	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
//
//	VERA_CONSTEXPR small_vector() VERA_NOEXCEPT = default;
//
//	VERA_CONSTEXPR small_vector(const small_vector& other)
//	{
//		m_storage.copy_from(other.m_storage);
//		std::uninitialized_copy(other.begin(), other.end(), begin());
//	}
//
//	VERA_CONSTEXPR ~small_vector()
//	{
//		clear();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR reference operator[](size_type pos)
//	{
//		VERA_ASSERT(pos < size());
//		return data()[pos];
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_reference operator[](size_type pos) const
//	{
//		VERA_ASSERT(pos < size());
//		return data()[pos];
//	}
//	
//	VERA_NODISCARD VERA_CONSTEXPR reference at(size_type pos)
//	{
//		if (pos >= size()) {
//			throw std::out_of_range("small_vector::at");
//		}
//		return data()[pos];
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_reference at(size_type pos) const
//	{
//		if (pos >= size()) {
//			throw std::out_of_range("small_vector::at");
//		}
//		return data()[pos];
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR reference front()
//	{
//		VERA_ASSERT(!empty());
//		return data()[0];
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_reference front() const
//	{
//		VERA_ASSERT(!empty());
//		return data()[0];
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR reference back()
//	{
//		VERA_ASSERT(!empty());
//		return data()[size() - 1];
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_reference back() const
//	{
//		VERA_ASSERT(!empty());
//		return data()[size() - 1];
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR pointer data() VERA_NOEXCEPT
//	{
//		return m_storage.data();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_pointer data() const VERA_NOEXCEPT
//	{
//		return m_storage.data();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR iterator begin() VERA_NOEXCEPT
//	{
//		return data();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_iterator begin() const VERA_NOEXCEPT
//	{
//		return data();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_iterator cbegin() const VERA_NOEXCEPT
//	{
//		return data();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR iterator end() VERA_NOEXCEPT
//	{
//		return data() + size();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_iterator end() const VERA_NOEXCEPT
//	{
//		return data() + size();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR const_iterator cend() const VERA_NOEXCEPT
//	{
//		return data() + size();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
//	{
//		return m_storage.size() == 0;
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR size_type size() const VERA_NOEXCEPT
//	{
//		return m_storage.size();
//	}
//
//	VERA_NODISCARD VERA_CONSTEXPR size_type capacity() const VERA_NOEXCEPT
//	{
//		return m_storage.capacity();
//	}
//
//	VERA_CONSTEXPR void clear() VERA_NOEXCEPT
//	{
//		std::destroy(begin(), end());
//		m_storage.m_size = 0;
//	}
//
//	template <class... Args>
//	VERA_CONSTEXPR reference emplace_back(Args&&... args)
//	{
//		if (size() == capacity()) {
//			grow(size() + 1);
//		}
//		alloc_traits::construct(get_allocator(), data() + size(), std::forward<Args>(args)...);
//		return data()[m_storage.m_size++];
//	}
//
//	VERA_CONSTEXPR void push_back(const T& value)
//	{
//		emplace_back(value);
//	}
//
//	VERA_CONSTEXPR void push_back(T&& value)
//	{
//		emplace_back(std::move(value));
//	}
//
//	VERA_CONSTEXPR void pop_back()
//	{
//		VERA_ASSERT(!empty());
//		alloc_traits::destroy(get_allocator(), data() + size() - 1);
//		--m_storage.m_size;
//	}
//
//private:
//	VERA_CONSTEXPR Allocator& get_allocator() VERA_NOEXCEPT
//	{
//		return static_cast<Allocator&>(m_storage);
//	}
//
//	VERA_CONSTEXPR const Allocator& get_allocator() const VERA_NOEXCEPT
//	{
//		return static_cast<const Allocator&>(m_storage);
//	}
//
//	VERA_CONSTEXPR void grow(size_type min_capacity)
//	{
//		size_type new_capacity = capacity() + capacity() / 2;
//		if (new_capacity < min_capacity) {
//			new_capacity = min_capacity;
//		}
//
//		pointer new_storage_ptr = alloc_traits::allocate(get_allocator(), new_capacity);
//		
//		// Move elements
//		std::uninitialized_move(begin(), end(), new_storage_ptr);
//
//		// Destroy old elements
//		std::destroy(begin(), end());
//
//		// Deallocate old storage if it was on the heap
//		if (!m_storage.is_using_inline_storage()) {
//			alloc_traits::deallocate(get_allocator(), m_storage.data(), capacity());
//		}
//
//		m_storage.m_storage.ptr = new_storage_ptr;
//		m_storage.m_capacity = new_capacity;
//	}
//
//	storage_type m_storage;
//};

VERA_NAMESPACE_END