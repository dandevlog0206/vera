#pragma once

#include "../core/coredefs.h"
#include "../core/exception.h"
#include "../core/assertion.h"
#include "static_vector.h"

#include <memory>
#include <utility>
#include <iterator>
#include <algorithm>
#include <new>

VERA_NAMESPACE_BEGIN

template <class T, size_t N>
class hybrid_vector
{
public:
	using value_type      = T;
	using pointer         = T*;
	using const_pointer   = const T*;
	using reference       = T&;
	using const_reference = const T&;
	using size_type       = size_t;
	using difference_type = ptrdiff_t;

	using iterator               = pointer;
	using const_iterator         = const_pointer;
	using reverse_iterator       = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	hybrid_vector() noexcept :
		m_stack(),
		m_ptr(N ? m_stack.elem : nullptr),
		m_size(0),
		m_capacity(N),
		m_alloc() {}

	hybrid_vector(size_t count) :
		m_stack(),
		m_ptr(N ? m_stack.elem : nullptr),
		m_size(0),
		m_capacity(N),
		m_alloc()
	{
		VERA_ASSERT_MSG(count <= max_size(), "hybrid_vector exceeded max_size");
		reserve(count);
		std::uninitialized_default_construct_n(m_ptr, count);
		m_size = count;
	}

	hybrid_vector(size_t count, const T& value) :
		m_stack(),
		m_ptr(N ? m_stack.elem : nullptr),
		m_size(0),
		m_capacity(N),
		m_alloc()
	{
		VERA_ASSERT_MSG(count <= max_size(), "hybrid_vector exceeded max_size");
		reserve(count);
		std::uninitialized_fill_n(m_ptr, count, value);
		m_size = count;
	}

	template <class Iter>
	hybrid_vector(Iter first, Iter last) :
		m_stack(),
		m_ptr(N ? m_stack.elem : nullptr),
		m_size(0),
		m_capacity(N),
		m_alloc()
	{
		size_t count = std::distance(first, last);
		VERA_ASSERT_MSG(count <= max_size(), "hybrid_vector exceeded max_size");
		reserve(count);
		std::uninitialized_copy_n(first, count, m_ptr);
		m_size = count;
	}

	hybrid_vector(std::initializer_list<T> ilist) :
		hybrid_vector(ilist.begin(), ilist.end()) {}

	hybrid_vector(const hybrid_vector& rhs) :
		m_stack(),
		m_ptr(N ? m_stack.elem : nullptr),
		m_size(0),
		m_capacity(N),
		m_alloc()
	{
		size_t count = rhs.m_size;
		VERA_ASSERT_MSG(count <= max_size(), "hybrid_vector exceeded max_size");
		if (rhs.is_heap()) {
			// allocate exact capacity from rhs
			allocate_and_copy_from(rhs.m_ptr, count, rhs.m_capacity);
		} else {
			// use stack
			std::uninitialized_copy_n(rhs.m_ptr, count, m_stack.elem);
			m_ptr = m_stack.elem;
			m_capacity = N;
		}
		m_size = count;
	}

	hybrid_vector(hybrid_vector&& rhs) noexcept :
		m_stack(),
		m_ptr(N ? m_stack.elem : nullptr),
		m_size(0),
		m_capacity(N),
		m_alloc()
	{
		if (rhs.is_heap()) {
			// take ownership of heap allocation
			m_ptr = rhs.m_ptr;
			m_capacity = rhs.m_capacity;
			m_size = rhs.m_size;
			// reset rhs to use its stack (or null if N==0)
			rhs.m_ptr = N ? rhs.m_stack.elem : nullptr;
			rhs.m_capacity = N;
			rhs.m_size = 0;
		} else {
			// move elements into our stack
			if (N) {
				std::uninitialized_move_n(rhs.m_ptr, rhs.m_size, m_stack.elem);
				m_ptr = m_stack.elem;
				m_capacity = N;
				m_size = rhs.m_size;
			} else {
				// N==0, nothing to move onto stack; allocate heap if size>0
				if (rhs.m_size) {
					allocate_and_move_from(rhs.m_ptr, rhs.m_size);
				}
			}
			std::destroy_n(rhs.m_ptr, rhs.m_size);
			rhs.m_size = 0;
		}
	}

	~hybrid_vector()
	{
		std::destroy_n(m_ptr, m_size);
		if (is_heap()) {
			m_alloc.deallocate(m_ptr, m_capacity);
		}
	}

	hybrid_vector& operator=(const hybrid_vector& rhs)
	{
		if (this == std::addressof(rhs)) return *this;

		if (rhs.m_size == 0) {
			clear();
			return *this;
		}

		if (rhs.m_size <= m_capacity && !is_heap()) {
			// both using stack or we have enough capacity on current storage
			if (rhs.m_size < m_size) {
				std::copy_n(rhs.m_ptr, rhs.m_size, m_ptr);
				std::destroy_n(m_ptr + rhs.m_size, m_size - rhs.m_size);
			} else if (rhs.m_size > m_size) {
				std::copy_n(rhs.m_ptr, m_size, m_ptr);
				std::uninitialized_copy_n(rhs.m_ptr + m_size, rhs.m_size - m_size, m_ptr + m_size);
			} else {
				std::copy_n(rhs.m_ptr, m_size, m_ptr);
			}
			m_size = rhs.m_size;
			return *this;
		}

		// general case: allocate new storage of rhs.m_capacity if rhs uses heap, else use rhs.m_size
		if (rhs.is_heap()) {
			auto* new_mem = m_alloc.allocate(rhs.m_capacity);
			std::uninitialized_copy_n(rhs.m_ptr, rhs.m_size, new_mem);
			cleanup_and_replace_heap(new_mem, rhs.m_size, rhs.m_capacity);
		} else {
			// rhs uses stack; we can try to fit into our stack, otherwise allocate
			if (rhs.m_size <= N) {
				// fit into stack
				if (is_heap()) {
					// free our heap first
					std::destroy_n(m_ptr, m_size);
					m_alloc.deallocate(m_ptr, m_capacity);
					m_ptr = m_stack.elem;
					m_capacity = N;
				}
				std::uninitialized_copy_n(rhs.m_ptr, rhs.m_size, m_ptr);
				m_size = rhs.m_size;
			} else {
				// shouldn't happen since rhs didn't use heap and has size>N
				allocate_and_copy_from(rhs.m_ptr, rhs.m_size, rhs.m_size);
			}
		}

		return *this;
	}

	hybrid_vector& operator=(hybrid_vector&& rhs) noexcept
	{
		if (this == std::addressof(rhs)) return *this;

		// destroy current
		std::destroy_n(m_ptr, m_size);
		if (is_heap()) {
			m_alloc.deallocate(m_ptr, m_capacity);
		}

		// move-from rhs
		if (rhs.is_heap()) {
			m_ptr = rhs.m_ptr;
			m_capacity = rhs.m_capacity;
			m_size = rhs.m_size;

			// reset rhs to use its stack
			rhs.m_ptr = N ? rhs.m_stack.elem : nullptr;
			rhs.m_capacity = N;
			rhs.m_size = 0;
		} else {
			// rhs on stack -> move elements into our stack (or allocate if we don't have stack)
			if (N) {
				std::uninitialized_move_n(rhs.m_ptr, rhs.m_size, m_stack.elem);
				m_ptr = m_stack.elem;
				m_capacity = N;
				m_size = rhs.m_size;
			} else {
				allocate_and_move_from(rhs.m_ptr, rhs.m_size);
			}
			std::destroy_n(rhs.m_ptr, rhs.m_size);
			rhs.m_size = 0;
		}

		return *this;
	}

	template <class... Args>
	T& emplace_back(Args&&... params)
	{
		ensure_capacity_for(1);
		new (m_ptr + m_size) T(std::forward<Args>(params)...);
		return m_ptr[m_size++];
	}

	void push_back(const T& value)
	{
		ensure_capacity_for(1);
		new (m_ptr + m_size) T(value);
		++m_size;
	}

	void push_back(T&& value)
	{
		ensure_capacity_for(1);
		new (m_ptr + m_size) T(std::move(value));
		++m_size;
	}

	void pop_back()
	{
		VERA_ASSERT_MSG(m_size > 0, "call pop_back() on empty hybrid_vector");
		std::destroy_at(m_ptr + --m_size);
		// optionally shrink to stack if a lot freed; not implemented here to keep behavior simple
	}

	template <class... Args>
	T& emplace(const_iterator where, Args&&... params)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid hybrid_vector position");
		size_t pos = std::distance(cbegin(), where);
		ensure_capacity_for(1);

		if (pos == m_size) {
			new (m_ptr + m_size) T(std::forward<Args>(params)...);
		} else {
			std::uninitialized_move_n(m_ptr + pos, m_size - pos, m_ptr + pos + 1);
			std::destroy_at(m_ptr + pos);
			new (m_ptr + pos) T(std::forward<Args>(params)...);
		}
		++m_size;
		return m_ptr[pos];
	}

	iterator insert(const_iterator where, const T& value)
	{
		return emplace(where, value);
	}

	iterator insert(const_iterator where, T&& value)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid hybrid_vector position");
		size_t pos = std::distance(cbegin(), where);
		ensure_capacity_for(1);

		if (pos == m_size) {
			new (m_ptr + m_size) T(std::move(value));
		} else {
			std::uninitialized_move_n(m_ptr + pos, m_size - pos, m_ptr + pos + 1);
			std::destroy_at(m_ptr + pos);
			new (m_ptr + pos) T(std::move(value));
		}
		++m_size;
		return m_ptr + pos;
	}

	iterator erase(const_iterator where)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where < cend(), "invalid hybrid_vector position");
		size_t pos = std::distance(cbegin(), where);
		std::destroy_at(m_ptr + pos);
		std::move(m_ptr + pos + 1, m_ptr + m_size, m_ptr + pos);
		--m_size;
		return m_ptr + pos;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		VERA_ASSERT_MSG(cbegin() <= first && first <= last && last <= cend(), "invalid hybrid_vector position");
		if (first == last) return const_cast<iterator>(first);

		size_t pos1 = std::distance(cbegin(), first);
		size_t pos2 = std::distance(cbegin(), last);
		size_t diff = pos2 - pos1;

		std::destroy_n(m_ptr + pos1, diff);
		std::move(m_ptr + pos2, m_ptr + m_size, m_ptr + pos1);
		m_size -= diff;
		return m_ptr + pos1;
	}

	void assign(size_t new_size, const T& value)
	{
		resize(new_size, value);
	}

	template <class Iter>
	void assign(Iter first, Iter last)
	{
		size_t new_size = std::distance(first, last);
		reserve(new_size);
		std::uninitialized_copy_n(first, new_size, m_ptr);
		m_size = new_size;
	}

	void resize(size_t new_size)
	{
		if (new_size < m_size) {
			std::destroy_n(m_ptr + new_size, m_size - new_size);
		} else if (new_size > m_size) {
			ensure_capacity_for(new_size - m_size);
			std::uninitialized_default_construct_n(m_ptr + m_size, new_size - m_size);
		} else {
			return;
		}
		m_size = new_size;
	}

	void resize(size_t new_size, const T& value)
	{
		if (new_size < m_size) {
			std::destroy_n(m_ptr + new_size, m_size - new_size);
		} else if (new_size > m_size) {
			ensure_capacity_for(new_size - m_size);
			std::uninitialized_fill_n(m_ptr + m_size, new_size - m_size, value);
		} else {
			return;
		}
		m_size = new_size;
	}

	void reserve(size_t new_capacity)
	{
		if (new_capacity <= m_capacity) return;
		VERA_ASSERT_MSG(new_capacity <= max_size(), "hybrid_vector exceeded max_size");
		auto* new_mem = m_alloc.allocate(new_capacity);
		std::uninitialized_move_n(m_ptr, m_size, new_mem);
		std::destroy_n(m_ptr, m_size);
		if (is_heap()) {
			m_alloc.deallocate(m_ptr, m_capacity);
		}
		m_ptr = new_mem;
		m_capacity = new_capacity;
	}

	void clear() noexcept
	{
		std::destroy_n(m_ptr, m_size);
		m_size = 0;
		// do not free heap allocation here; keep capacity (similar to std::vector)
	}

	void swap(hybrid_vector& rhs) noexcept
	{
		// swap considering stack-vs-heap states
		if (this == std::addressof(rhs)) return;

		if (!is_heap() && !rhs.is_heap()) {
			// both on stack: swap elements up to min size, move remainder, adjust sizes
			size_t min_sz = std::min(m_size, rhs.m_size);
			for (size_t i = 0; i < min_sz; ++i) std::swap(m_ptr[i], rhs.m_ptr[i]);

			if (m_size < rhs.m_size) {
				// move rhs remainder into this stack
				std::uninitialized_move_n(rhs.m_ptr + m_size, rhs.m_size - m_size, m_ptr + m_size);
				std::destroy_n(rhs.m_ptr + m_size, rhs.m_size - m_size);
			} else if (rhs.m_size < m_size) {
				std::uninitialized_move_n(m_ptr + rhs.m_size, m_size - rhs.m_size, rhs.m_ptr + rhs.m_size);
				std::destroy_n(m_ptr + rhs.m_size, m_size - rhs.m_size);
			}
			std::swap(m_size, rhs.m_size);
		} else {
			// ensure both end up valid after swapping pointers/capacity/size
			std::swap_ranges(m_ptr, m_ptr + std::min(m_size, rhs.m_size), rhs.m_ptr);
			std::swap(m_ptr, rhs.m_ptr);
			std::swap(m_size, rhs.m_size);
			std::swap(m_capacity, rhs.m_capacity);
		}
	}

	T* data() noexcept { return m_ptr; }
	const T* data() const noexcept { return m_ptr; }

	iterator begin() noexcept { return m_ptr; }
	const_iterator begin() const noexcept { return m_ptr; }
	iterator end() noexcept { return m_ptr + m_size; }
	const_iterator end() const noexcept { return m_ptr + m_size; }

	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

	const_iterator cbegin() const noexcept { return begin(); }
	const_iterator cend() const noexcept { return end(); }
	const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

	bool empty() const noexcept { return m_size == 0; }
	size_t size() const noexcept { return m_size; }
	size_t max_size() const noexcept { return static_cast<size_t>(-1) / sizeof(T); } // or large sentinel
	size_t capacity() const noexcept { return m_capacity; }

	T& operator[](const size_t pos) noexcept
	{
		VERA_ASSERT_MSG(pos < m_size, "invalid hybrid_vector subscript");
		return m_ptr[pos];
	}

	const T& operator[](const size_t pos) const noexcept
	{
		VERA_ASSERT_MSG(pos < m_size, "invalid hybrid_vector subscript");
		return m_ptr[pos];
	}

	T& at(size_t pos)
	{
		VERA_CHECK_MSG(pos < m_size, "invalid hybrid_vector subscript");
		return m_ptr[pos];
	}

	const T& at(size_t pos) const
	{
		VERA_CHECK_MSG(pos < m_size, "invalid hybrid_vector subscript");
		return m_ptr[pos];
	}

	T& front()
	{
		VERA_ASSERT_MSG(m_size > 0, "front() called on empty hybrid_vector");
		return m_ptr[0];
	}

	const T& front() const
	{
		VERA_ASSERT_MSG(m_size > 0, "front() called on empty hybrid_vector");
		return m_ptr[0];
	}

	T& back()
	{
		VERA_ASSERT_MSG(m_size > 0, "back() called on empty hybrid_vector");
		return m_ptr[m_size - 1];
	}

	const T& back() const
	{
		VERA_ASSERT_MSG(m_size > 0, "back() called on empty hybrid_vector");
		return m_ptr[m_size - 1];
	}

private:
	// returns true if current storage is heap-allocated
	bool is_heap() const noexcept { return m_ptr != (N ? m_stack.elem : nullptr); }

	// ensure capacity to append 'extra' elements
	void ensure_capacity_for(size_t extra)
	{
		size_t required = m_size + extra;
		if (required <= m_capacity) return;
		size_t new_cap = m_capacity ? m_capacity * 2 : 1;
		if (new_cap < required) new_cap = required;
		if (new_cap > max_size()) new_cap = required; // allow required if it exceeds doubling but below max_size check earlier
		reserve(new_cap);
	}

	// allocate new heap of capacity and copy elements from src (count elements)
	void allocate_and_copy_from(const T* src, size_t count, size_t new_capacity)
	{
		auto* new_mem = m_alloc.allocate(new_capacity);
		std::uninitialized_copy_n(src, count, new_mem);
		// cleanup current storage
		std::destroy_n(m_ptr, m_size);
		if (is_heap()) {
			m_alloc.deallocate(m_ptr, m_capacity);
		}
		m_ptr = new_mem;
		m_capacity = new_capacity;
		m_size = count;
	}

	// allocate heap and move from src (count elements)
	void allocate_and_move_from(T* src, size_t count)
	{
		size_t new_capacity = count;
		auto* new_mem = m_alloc.allocate(new_capacity);
		std::uninitialized_move_n(src, count, new_mem);
		std::destroy_n(src, count);
		if (is_heap()) {
			m_alloc.deallocate(m_ptr, m_capacity);
		}
		m_ptr = new_mem;
		m_capacity = new_capacity;
		m_size = count;
	}

	// cleanup existing and replace with provided heap pointer (ownership transferred)
	void cleanup_and_replace_heap(T* new_mem, size_t new_size, size_t new_capacity)
	{
		std::destroy_n(m_ptr, m_size);
		if (is_heap()) {
			m_alloc.deallocate(m_ptr, m_capacity);
		}
		m_ptr = new_mem;
		m_size = new_size;
		m_capacity = new_capacity;
	}

private:
	priv::uninitialized_storage<T, N> m_stack;
	T*                                m_ptr;
	size_t                            m_size;
	size_t                            m_capacity;
	std::allocator<T>                 m_alloc;
};

VERA_NAMESPACE_END