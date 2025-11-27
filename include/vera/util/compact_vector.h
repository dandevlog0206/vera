#pragma once

#include "../core/exception.h"
#include "../core/assertion.h"
#include <iterator>
#include <memory>
#include <bit>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

template <class T>
struct compressed_ptr
{
	static_assert(sizeof(intptr_t) == 8);

	static constexpr size_t MAX_SIZE = UINT16_MAX;

	compressed_ptr() :
		m_ptr(0),
		m_size(0) {}

	compressed_ptr(T* ptr, size_t size) :
		m_ptr(reinterpret_cast<intptr_t>(ptr)),
		m_size(size) {}

	void swap(compressed_ptr& rhs) noexcept
	{
		std::swap(
			reinterpret_cast<intptr_t&>(*this),
			reinterpret_cast<intptr_t&>(rhs));
	}

	void set_ptr(T* ptr)
	{
		m_ptr = reinterpret_cast<intptr_t>(ptr);
	}

	void set_size(size_t size)
	{
		VERA_ASSERT(size < 0x10000);
		m_size = size;
	}

	T* get_ptr()
	{
		return reinterpret_cast<T*>(m_ptr);
	}

	const T* get_ptr() const
	{
		return reinterpret_cast<const T*>(m_ptr);
	}

	size_t get_size() const
	{
		return m_size;
	}

	T* reset()
	{
		T* old_ptr = get_ptr();

		reinterpret_cast<intptr_t&>(*this) = 0;

		return old_ptr;
	}

	T* reset(T* ptr, size_t new_size)
	{
		T* old_ptr = get_ptr();
		
		m_ptr  = reinterpret_cast<intptr_t>(ptr);
		m_size = new_size;
		
		return old_ptr;
	}

private:
	intptr_t m_ptr  : 48;
	size_t   m_size : 16;
};

VERA_PRIV_NAMESPACE_END

template <class T>
class compact_vector
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

	compact_vector() :
		m_storage() {}

	compact_vector(size_t count) :
		m_storage()
	{
		size_t capacity   = calc_capacity(count);
		T*     new_memory = allocate_uninitialized(capacity);

		std::uninitialized_default_construct_n(new_memory, count);
		m_storage.set_ptr(new_memory);
	}
	
	compact_vector(size_t count, const T& value) :
		m_storage()
	{
		size_t capacity   = calc_capacity(count);
		T*     new_memory = allocate_uninitialized(capacity);

		std::uninitialized_fill_n(new_memory, count, value);
		m_storage.set_ptr(new_memory);
		m_storage.set_size(count);
	}

	template <class Iter>
	compact_vector(Iter first, Iter last) :
		m_storage()
	{
		size_t count      = std::distance(first, last);
		size_t capacity   = calc_capacity(count);
		T*     new_memory = allocate_uninitialized(capacity);
		
		std::uninitialized_copy_n(first, count, new_memory);
		m_storage.set_ptr(new_memory);
		m_storage.set_size(count);
	}

	compact_vector(std::initializer_list<T> ilist) :
		compact_vector(ilist.begin(), ilist.end()) {}
	
	compact_vector(const compact_vector& rhs) :
		m_storage()
	{
		size_t count      = rhs.m_storage.get_size();
		size_t capacity   = calc_capacity(count);
		T*     new_memory = allocate_uninitialized(capacity);
		
		std::uninitialized_copy_n(rhs.m_storage.get_ptr(), count, new_memory);
		m_storage.set_ptr(new_memory);
		m_storage.set_size(count);
	}

	compact_vector(compact_vector&& rhs) noexcept :
		m_storage()
	{
		m_storage.swap(rhs.m_storage);
	}
	
	~compact_vector()
	{
		deallocate_uninitialized(m_storage.get_ptr(), m_storage.get_size());
	}
	
	compact_vector& operator=(const compact_vector& rhs)
	{
		if (this == std::addressof(rhs)) return *this;

		size_t   my_size      = m_storage.get_size();
		size_t   rhs_size     = rhs.m_storage.get_size();
		size_t   my_capacity  = calc_capacity(my_size);
		size_t   rhs_capacity = calc_capacity(rhs_size);
		T*       my_ptr       = m_storage.get_ptr();
		const T* rhs_ptr      = rhs.m_storage.get_ptr();

		if (my_capacity == rhs_capacity) {
			if (rhs_size < my_size) {
				std::copy_n(rhs_ptr, rhs_size, my_ptr);
				std::destroy_n(my_ptr + rhs_size, my_size - rhs_size);
			} else if (my_size < rhs_size) {
				std::copy_n(rhs_ptr, my_size, my_ptr);
				std::uninitialized_copy_n(rhs_ptr + my_size, rhs_size - my_size, my_ptr + my_size);
			} else /* rhs_size == my_size */ {
				std::copy_n(rhs_ptr, my_size, my_ptr);
				return *this;
			}
			m_storage.set_size(rhs_size);
		} else {
			auto* new_memory = allocate_uninitialized(rhs_capacity);

			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			std::uninitialized_copy_n(rhs_ptr, rhs_size, new_memory);
			m_storage.reset(new_memory, rhs_size);
		}

		return *this;
	}
	
	compact_vector& operator=(compact_vector&& rhs) noexcept
	{
		if (this == std::addressof(rhs)) return *this;

		deallocate_uninitialized(m_storage.get_ptr(), m_storage.get_size());
		m_storage = rhs.m_storage;
		rhs.m_storage.reset();
		
		return *this;
	}

	template <class... Args>
	T& emplace_back(Args&&... params)
	{
		size_t my_size      = m_storage.get_size();
		size_t new_size     = size + 1;
		size_t my_capacity  = calc_capacity(size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();

		if (my_capacity == new_capacity) {
			new (my_ptr + my_size) T(std::forward<Args>(params)...);
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(my_ptr, my_size, new_memory);
			new (new_memory + my_size) T(std::forward<Args>(params)...);
			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr() + new_size;
	}

	void push_back(const T& value)
	{
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size + 1;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();

		if (my_capacity == new_capacity) {
			new (my_ptr + my_size) T(value);
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(my_ptr, my_size, new_memory);
			new (new_memory + my_size) T(value);
			std::destroy_n(my_ptr, size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}
	}

	void push_back(T&& value)
	{
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size + 1;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();

		if (my_capacity == new_capacity) {
			new (my_ptr + my_size) T(std::move(value));
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(my_ptr, my_size, new_memory);
			new (new_memory + my_size) T(std::move(value));
			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}
	}

	void pop_back()
	{
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size - 1;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();

		if (my_capacity == new_capacity) {
			std::destroy_at(my_ptr + new_size);
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);
			
			std::uninitialized_move_n(my_ptr, new_size, new_memory);
			std::destroy_at(my_ptr + new_size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}
	}

	template <class... Args>
	T& emplace(const_iterator where, Args&&... params)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid compact_vector position");
		
		size_t pos          = std::distance(cbegin(), where);
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size + 1;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();
		
		if (my_capacity == new_capacity) {
			std::move_backward(my_ptr + pos, my_ptr + my_size, my_ptr + new_size);
			std::destroy_at(my_ptr + pos);
			new (my_ptr + pos) T(std::forward<Args>(params)...);
			m_storage.set_size(new_size);
		} else {
			T* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(my_ptr, pos, new_memory);
			new (new_memory + pos) T(std::forward<Args>(params)...);
			std::uninitialized_move_n(my_ptr + pos, my_size - pos, new_memory + pos + 1);
			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr()[pos];
	}

	iterator insert(const_iterator where, const T& value)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid compact_vector position");

		size_t pos          = std::distance(cbegin(), where);
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size + 1;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();
		
		if (my_capacity == new_capacity) {
			std::move_backward(my_ptr + pos, my_ptr + my_size, my_ptr + new_size);
			std::destroy_at(my_ptr + pos);
			new (my_ptr + pos) T(value);
			m_storage.set_size(new_size);
		} else {
			T* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(my_ptr, pos, new_memory);
			new (new_memory + pos) T(value);
			std::uninitialized_move_n(my_ptr + pos, my_size - pos, new_memory + pos + 1);
			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr() + pos;
	}
	
	iterator insert(const_iterator where, T&& value)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid compact_vector position");

		size_t pos          = std::distance(cbegin(), where);
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size + 1;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();
		
		if (my_capacity == new_capacity) {
			std::move_backward(my_ptr + pos, my_ptr + my_size, my_ptr + new_size);
			std::destroy_at(my_ptr + pos);
			new (my_ptr + pos) T(std::move(value));
			m_storage.set_size(new_size);
		} else {
			T* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(my_ptr, pos, new_memory);
			new (new_memory + pos) T(std::move(value));
			std::uninitialized_move_n(my_ptr + pos, my_size - pos, new_memory + pos + 1);
			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr() + pos;
	}

	iterator insert(const_iterator where, size_t count, const T& value)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid compact_vector position");

		size_t pos          = std::distance(cbegin(), where);
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size + count;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     my_ptr       = m_storage.get_ptr();
		
		if (my_capacity == new_capacity) {
			std::move_backward(my_ptr + pos, my_ptr + my_size, my_ptr + new_size);
			std::destroy_n(my_ptr + pos, count);
			std::uninitialized_fill_n(my_ptr + pos, count, value);
			m_storage.set_size(new_size);
		} else {
			T* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(my_ptr, pos, new_memory);
			std::uninitialized_fill_n(new_memory + pos, count, value);
			std::uninitialized_move_n(my_ptr + pos, my_size - pos, new_memory + pos + 1);
			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr() + pos;
	}
	
	template <class Iter>
	iterator insert(const_iterator where, Iter first, Iter last)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where < cend(), "invalid compact_vector position");

		size_t pos          = std::distance(cbegin(), where);
		size_t count        = std::distance(first, last);
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size + count;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     ptr          = m_storage.get_ptr();

		if (my_capacity == new_capacity) {
			std::move_backward(ptr + pos, ptr + my_size, ptr + new_size);
			std::destroy_n(ptr + pos, count);
			std::uninitialized_copy_n(first, count, ptr + pos);
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(ptr, pos, new_memory);
			std::uninitialized_copy_n(first, count, new_memory + pos);
			std::uninitialized_move_n(ptr + pos, my_size - pos, new_memory + pos + count);
			std::destroy_n(ptr, my_size);
			deallocate_uninitialized(ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr() + pos;
	}

	iterator insert(std::initializer_list<T> ilist)
	{
		return insert(cbegin(), ilist.begin(), ilist.end());
	}

	iterator erase(const_iterator where)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where < cend(), "invalid compact_vector position");

		size_t pos          = std::distance(cbegin(), where);
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size - 1;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     ptr          = m_storage.get_ptr();
		T*     target;

		if (my_capacity == new_capacity) {
			std::move(ptr + pos + 1, ptr + my_size, ptr + pos);
			std::destroy_at(ptr + new_size);
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(ptr, pos, new_memory);
			std::uninitialized_move_n(ptr + pos + 1, my_size - pos - 1, new_memory + pos);
			std::destroy_n(ptr, my_size);
			deallocate_uninitialized(ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr() + pos;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		VERA_ASSERT_MSG(cbegin() <= first && first < last && last <= cend(), "invalid compact_vector position");

		size_t pos1         = std::distance(cbegin(), first);
		size_t pos2         = std::distance(cbegin(), last);
		size_t count         = pos2 - pos1;
		size_t my_size      = m_storage.get_size();
		size_t new_size     = my_size - count;
		size_t my_capacity  = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);
		T*     ptr          = m_storage.get_ptr();

		if (my_capacity == new_capacity) {
			std::move(ptr + pos2, ptr + my_size, ptr + pos1);
			std::destroy_n(ptr + new_size, count);
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);

			std::uninitialized_move_n(ptr, pos1, new_memory);
			std::uninitialized_move_n(ptr + pos2, my_size - pos2, new_memory + pos1);
			std::destroy_n(ptr, my_size);
			deallocate_uninitialized(ptr, my_size);
			m_storage.reset(new_memory, new_size);
		}

		return m_storage.get_ptr() + pos1;
	}

	void assign(size_t new_size, const T& value)
	{
		resize(new_size, value);
	}

	template <class Iter>
	void assign(Iter first, Iter last)
	{
		size_t   my_size      = m_storage.get_size();
		size_t   new_size     = std::distance(first, last);
		size_t   my_capacity  = calc_capacity(my_size);
		size_t   new_capacity = calc_capacity(new_size);
		T*       my_ptr       = m_storage.get_ptr();

		if (my_capacity == new_capacity) {
			if (new_size < my_size) {
				std::copy_n(first, new_size, my_ptr);
				std::destroy_n(my_ptr + new_size, my_size - new_size);
			} else if (my_size < new_size) {
				std::copy_n(first, my_size, my_ptr);
				std::uninitialized_copy_n(first + my_size, new_size - my_size, my_ptr + my_size); // TODO: fix first + my_size
			} else /* rhs_size == my_size */ {
				std::copy_n(first, my_size, my_ptr);
				return *this;
			}
			m_storage.set_size(new_size);
		} else {
			auto* new_memory = allocate_uninitialized(new_capacity);

			std::destroy_n(my_ptr, my_size);
			deallocate_uninitialized(my_ptr, my_size);
			std::uninitialized_copy_n(first, new_size, new_memory);
			m_storage.reset(new_memory, new_size);
		}

		return *this;
	}

	template <class Iter>
	void assign(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}

	void resize(size_t new_size)
	{
		size_t my_size = m_storage.get_size();

		if (my_size == new_size) return;

		size_t old_capacity = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);

		if (old_capacity == new_capacity) {
			if (my_size < new_size) {
				std::uninitialized_default_construct_n(m_storage.get_ptr() + my_size, new_size - my_size);
			} else /* new_size < size */ {
				std::destroy_n(m_storage.get_ptr() + new_size, my_size - new_size);
			}
			m_storage.set_size(new_size);
		} else {
			T* new_memory = allocate_uninitialized(new_capacity);

			if (old_capacity < new_capacity) {
				std::uninitialized_move_n(m_storage.get_ptr(), my_size, new_memory);
				std::uninitialized_default_construct_n(new_memory + my_size, new_size - my_size);
			} else /* new_capacity < old_capacity */ {
				std::uninitialized_move_n(m_storage.get_ptr(), new_size, new_memory);
				std::destroy_n(m_storage.get_ptr() + new_size, my_size - new_size);
			}

			deallocate_uninitialized(m_storage.get_ptr(), my_size);
			m_storage.reset(new_memory, new_size);
		}
	}
	
	void resize(size_t new_size, const T& value)
	{
		size_t my_size = m_storage.get_size();

		if (my_size == new_size) return;

		size_t old_capacity = calc_capacity(my_size);
		size_t new_capacity = calc_capacity(new_size);

		if (old_capacity == new_capacity) {
			if (my_size < new_size) {
				std::uninitialized_fill_n(m_storage.get_ptr() + my_size, new_size - my_size, value);
			} else /* new_size < size */ {
				std::destroy_n(m_storage.get_ptr() + new_size, my_size - new_size);
			}
			m_storage.set_size(new_size);
		} else {
			T* new_memory = allocate_uninitialized(new_capacity);

			if (old_capacity < new_capacity) {
				std::uninitialized_move_n(m_storage.get_ptr(), my_size, new_memory);
				std::uninitialized_fill_n(new_memory + my_size, new_size - my_size, value);
			} else /* new_capacity < old_capacity */ {
				std::uninitialized_move_n(m_storage.get_ptr(), new_size, new_memory);
				std::destroy_n(m_storage.get_ptr() + new_size, my_size - new_size);
			}

			deallocate_uninitialized(m_storage.get_ptr(), my_size);
			m_storage.reset(new_memory, new_size);
		}
	}
	
	void clear() noexcept
	{
		deallocate_uninitialized(m_storage.get_ptr(), m_storage.get_size());
		m_storage.reset();
	}

	void swap(compact_vector& rhs) noexcept
	{
		m_storage.swap(rhs.m_storage);
	}

	T* data() noexcept
	{
		return m_storage.get_ptr();
	}

	const T* data() const noexcept
	{
		return m_storage.get_ptr();
	}

	iterator begin() noexcept
	{
		return m_storage.get_ptr();
	}

	const_iterator begin() const noexcept
	{
		return m_storage.get_ptr();
	}

	iterator end() noexcept
	{
		return m_storage.get_ptr() + m_storage.get_size();
	}

	const_iterator end() const noexcept
	{
		return m_storage.get_ptr() + m_storage.get_size();
	}

	reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	const_reverse_iterator crend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	bool empty() const noexcept
	{
		return m_storage.get_size() == 0;
	}

	size_t size() const noexcept
	{
		return m_storage.get_size();
	}

	size_t max_size() const noexcept
	{
		return priv::compressed_ptr<T>::MAX_SIZE;
	}

	size_t capacity() const noexcept
	{
		return ;
	}

	T& operator[](const size_t pos) noexcept
	{
		VERA_ASSERT_MSG(pos < m_storage.get_size(), "invalid compact_vector subscript");
		return m_storage.get_ptr()[pos];
	}

	const T& operator[](const size_t pos) const noexcept
	{
		VERA_ASSERT_MSG(pos < m_storage.get_size(), "invalid compact_vector subscript");
		return m_storage.get_ptr()[pos];
	}

	T& at(size_t pos)
	{
		VERA_CHECK_MSG(pos < m_storage.get_size(), "invalid compact_vector subscript");
		return m_storage.get_ptr()[pos];
	}

	const T& at(size_t pos) const
	{
		VERA_CHECK_MSG(pos < m_storage.get_size(), "invalid compact_vector subscript");
		return m_storage.get_ptr()[pos];
	}

	T& front()
	{
		VERA_ASSERT_MSG(m_storage.get_size() == 0, "front() called on empty compact_vector");
		return m_storage.get_ptr()[0];
	}

	const T& front() const
	{
		VERA_ASSERT_MSG(m_storage.get_size() == 0, "front() called on empty compact_vector");
		return m_storage.get_ptr()[0];
	}

	T& back()
	{
		VERA_ASSERT_MSG(m_storage.get_size() == 0, "back() called on empty compact_vector");
		return m_storage.get_ptr()[m_storage.get_size() - 1];
	}
	
	const T& back() const
	{
		VERA_ASSERT_MSG(m_storage.get_size() == 0, "back() called on empty compact_vector");
		return m_storage.get_ptr()[m_storage.get_size() - 1];
	}

private:
	static size_t calc_capacity(size_t size)
	{
		if (size == 0) return 0;
		return size <= 4 ? 4 : 1ULL << (0x40 - std::countl_zero(size));
	}

	static T* allocate_uninitialized(size_t count)
	{
		VERA_ASSERT_MSG(count < 0x10000, "compact_vector exeeded max size");

		std::allocator<T>().allocate(count);
	}

	static void deallocate_uninitialized(T* ptr, size_t count)
	{
		std::allocator<T>().deallocate(ptr, 0);
	}

private:
	priv::compressed_ptr<T> m_storage;
};

VERA_NAMESPACE_END