#pragma once

#include "../core/exception.h"
#include <utility>
#include <iterator>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

#pragma warning(push)
#pragma warning(disable : 26495)

template <class T, size_t N>
struct uninitialized_storage
{
	using aligned_t = std::aligned_storage_t<sizeof(T), alignof(T)>;

	uninitialized_storage() {}
	~uninitialized_storage() {}

	union {
		T         elem[N];
		aligned_t _dummy[N];
	};
};

#pragma warning(pop)

VERA_PRIV_NAMESPACE_END

template <class T, size_t N>
class static_vector
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

	static_vector() :
		m_storage(),
		m_size(0) {}

	static_vector(size_t count) :
		m_storage(),
		m_size(count)
	{
		VERA_ASSERT_MSG(count <= N, "static_vector exceeded max_size");
		std::uninitialized_default_construct_n(m_storage.elem, count);
	}
	
	static_vector(size_t count, const T& value) :
		m_storage(),
		m_size(count)
	{
		VERA_ASSERT_MSG(count <= N, "static_vector exceeded max_size");
		std::uninitialized_fill_n(m_storage.elem, count, value);
	}

	template <class Iter>
	static_vector(Iter first, Iter last) :
		m_storage(),
		m_size(std::distance(first, last))
	{
		VERA_ASSERT_MSG(m_size <= N, "static_vector exceeded max_size");
		std::uninitialized_copy_n(first, m_size, m_storage.elem);
	}

	static_vector(std::initializer_list<T> ilist) :
		static_vector(ilist.begin(), ilist.end()) {}
	
	static_vector(const static_vector& rhs) :
		m_storage(),
		m_size(rhs.m_size)
	{
		std::uninitialized_copy_n(rhs.m_storage.elem, m_size, m_storage.elem);
	}

	static_vector(static_vector&& rhs) noexcept :
		m_storage(),
		m_size(std::exchange(rhs.m_size, 0))
	{
		std::uninitialized_move_n(rhs.m_storage.elem, m_size, m_storage.elem);
		std::destroy_n(rhs.m_storage.elem, m_size);
	}
	
	~static_vector()
	{
		std::destroy_n(m_storage.elem, m_size);
	}
	
	static_vector& operator=(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());

		return *this;
	}

	static_vector& operator=(const static_vector& rhs)
	{
		if (this == std::addressof(rhs)) return *this;

		if (rhs.m_size < m_size) {
			std::copy_n(rhs.m_storage.elem, rhs.m_size, m_storage.elem);
			std::destroy_n(m_storage.elem + rhs.m_size, m_size - rhs.m_size);
		} else if (rhs.m_size > m_size) {
			std::copy_n(rhs.m_storage.elem, m_size, m_storage.elem);
			std::uninitialized_copy_n(rhs.m_storage.elem + m_size, rhs.m_size - m_size, m_storage.elem + m_size);
		} else /* rhs.m_size == m_size */ {
			std::copy_n(rhs.m_storage.elem, m_size, m_storage.elem);
			return *this;
		}

		m_size = rhs.m_size;

		return *this;
	}
	
	static_vector& operator=(static_vector&& rhs) noexcept
	{
		if (this == std::addressof(rhs)) return *this;

		if (rhs.m_size < m_size) {
			std::move(rhs.m_storage.elem, rhs.m_storage.elem + rhs.m_size, m_storage.elem);
			std::destroy_n(m_storage.elem + rhs.m_size, m_size - rhs.m_size);
		} else if (rhs.m_size > m_size) {
			VERA_ASSERT_MSG(rhs.m_size <= N, "static_vector exceeded max_size");
			std::move(rhs.m_storage.elem, rhs.m_storage.elem + m_size, m_storage.elem);
			std::uninitialized_move_n(rhs.m_storage.elem + m_size, rhs.m_size - m_size, m_storage.elem + m_size);
			std::destroy_n(rhs.m_storage.elem, rhs.m_size);
		} else /* rhs.m_size == m_size */ {
			std::move(rhs.m_storage.elem, rhs.m_storage.elem + m_size, m_storage.elem);
			std::destroy_n(rhs.m_storage.elem, rhs.m_size);
			return *this;
		}

		m_size = std::exchange(rhs.m_size, 0);
		
		return *this;
	}

	template <class... Args>
	T& emplace_back(Args&&... params)
	{
		VERA_ASSERT_MSG(m_size < N, "static_vector exceeded max_size");
		return *(new (&m_storage.elem[m_size++]) T(std::forward<Args>(params)...));
	}

	void push_back(const T& value)
	{
		VERA_ASSERT_MSG(m_size < N, "static_vector exceeded max_size");
		new (&m_storage.elem[m_size++]) T(value);
	}

	void push_back(T&& value)
	{
		VERA_ASSERT_MSG(m_size < N, "static_vector exceeded max_size");
		new (&m_storage.elem[m_size++]) T(std::move(value));
	}

	void pop_back()
	{
		VERA_ASSERT_MSG(m_size > 0, "call pop_back() on empty static_vector");
		std::destroy_at(&m_storage.elem[--m_size]);
	}

	template <class... Args>
	T& emplace(const_iterator where, Args&&... params)
	{
		VERA_ASSERT_MSG(m_size < N, "static_vector exceeded max_size");
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid static_vector position");
		
		size_t pos = std::distance(cbegin(), where);

		if (pos == m_size) {
			new (&m_storage.elem[m_size++]) T(std::forward<Args>(params)...);
		} else {
			T* first_elem = m_storage.elem + pos;
			T* last_elem  = m_storage.elem + pos - 1;

			new (&m_storage.elem[m_size]) T(std::move(*last_elem));
			std::move_backward(first_elem, last_elem, &m_storage.elem[m_size]);
			std::destroy_at(&m_storage.elem[pos]);
			new (&m_storage.elem[pos]) T(std::forward<Args>(params)...);
			m_size++;
		}

		return m_storage.elem[pos];
	}

	iterator insert(const_iterator where, const T& value)
	{
		VERA_ASSERT_MSG(m_size < N, "static_vector exceeded max_size");
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid static_vector position");

		size_t pos = std::distance(cbegin(), where);
		
		if (pos == m_size) {
			new (&m_storage.elem[m_size]) T(value);
		} else {
			T* first_elem  = m_storage.elem + pos;
			T* last_elem   = m_storage.elem + m_size - 1;
			T* target_elem = m_storage.elem + pos;

			new (&m_storage.elem[m_size]) T(std::move(*last_elem));
			std::move_backward(first_elem, last_elem, m_storage.elem + m_size);
			std::destroy_at(target_elem);
			new (target_elem) T(value);
		}

		m_size++;

		return m_storage.elem + pos;
	}
	
	iterator insert(const_iterator where, T&& value)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid static_vector position");
		VERA_ASSERT_MSG(m_size < N, "static_vector exceeded max_size");

		size_t pos = std::distance(cbegin(), where);
		
		if (pos == m_size) {
			new (&m_storage.elem[m_size]) T(std::move(value));
		} else {
			T* first_elem  = m_storage.elem + pos;
			T* last_elem   = m_storage.elem + m_size - 1;
			T* target_elem = m_storage.elem + pos;
			
			new (&m_storage.elem[m_size]) T(std::move(*last_elem));
			std::move_backward(first_elem, last_elem, m_storage.elem + m_size);
			std::destroy_at(target_elem);
			new (target_elem) T(std::move(value));
		}

		m_size++;

		return m_storage.elem + pos;
	}

	iterator insert(const_iterator where, size_t count, const T& value)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid static_vector position");
		VERA_ASSERT_MSG(m_size + count <= N, "static_vector exceeded max_size");

		size_t pos = std::distance(cbegin(), where);

		if (pos == m_size) {
			std::uninitialized_fill_n(m_storage.elem + m_size, count, value);
		} else {
			T* first_elem = m_storage.elem + pos;
			T* last_elem  = m_storage.elem + m_size - 1;

			std::uninitialized_move_n(last_elem - (count - 1), count, m_storage.elem + m_size);
			std::move_backward(first_elem, last_elem - (count - 1), m_storage.elem + m_size);
			std::destroy_n(first_elem, count);
			std::uninitialized_fill_n(first_elem, count, value);
		}

		m_size += count;

		return m_storage.elem + pos;
	}
	
	template <class Iter>
	iterator insert(const_iterator where, Iter first, Iter last)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where <= cend(), "invalid static_vector position");
		
		size_t pos  = std::distance(cbegin(), where);
		size_t diff = std::distance(first, last);
		
		VERA_ASSERT_MSG(m_size + diff <= N, "static_vector exceeded max_size");
		
		if (pos == m_size) {
			std::uninitialized_copy_n(first, diff, m_storage.elem + m_size);
		} else {
			T* first_elem = m_storage.elem + pos;
			T* last_elem  = m_storage.elem + m_size - 1;
			
			std::uninitialized_move_n(last_elem - (diff - 1), diff, m_storage.elem + m_size);
			std::move_backward(first_elem, last_elem - (diff - 1), m_storage.elem + m_size);
			std::destroy_n(first_elem, diff);
			std::uninitialized_copy_n(first, diff, first_elem);
		}
		
		m_size += diff;

		return m_storage.elem + pos;
	}

	iterator insert(std::initializer_list<T> ilist)
	{
		return insert(cbegin(), ilist.begin(), ilist.end());
	}

	iterator erase(const_iterator where)
	{
		VERA_ASSERT_MSG(cbegin() <= where && where < cend(), "invalid static_vector position");

		size_t pos = std::distance(cbegin(), where);
		std::destroy_at(m_storage.elem + pos);
		std::move(m_storage.elem + pos + 1, m_storage.elem + m_size, m_storage.elem + pos);
		m_size--;

		return m_storage.elem + pos;
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		VERA_ASSERT_MSG(cbegin() <= first && first <= last && last <= cend(), "invalid static_vector position");

		if (first == last) return const_cast<iterator>(first);

		size_t pos1 = std::distance(cbegin(), first);
		size_t pos2 = std::distance(cbegin(), last);
		size_t diff = pos2 - pos1;

		std::destroy_n(m_storage.elem + pos1, diff);
		std::move(m_storage.elem + pos2, m_storage.elem + m_size, m_storage.elem + pos1);
		m_size -= diff;

		return m_storage.elem + pos1;
	}

	void assign(size_t new_size, const T& value)
	{
		resize(new_size, value);
	}

	template <class Iter>
	void assign(Iter first, Iter last)
	{
		size_t new_size = std::distance(first, last);

		if (new_size < m_size)
			std::destroy_n(m_storage.elem + new_size, m_size - new_size);

		std::uninitialized_copy_n(first, new_size, m_storage.elem);

		m_size = new_size;
	}

	template <class Iter>
	void assign(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}

	void resize(size_t new_size)
	{
		if (new_size < m_size) {
			std::destroy_n(m_storage.elem + new_size, m_size - new_size);
		} else if (new_size > m_size) {
			VERA_ASSERT_MSG(new_size <= N, "static_vector exceeded max_size");
			std::uninitialized_default_construct_n(m_storage.elem + m_size, new_size - m_size);
		} else /* new_size == m_size */ {
			return;
		}

		m_size = new_size;
	}
	
	void resize(size_t new_size, const T& value)
	{
		if (new_size < m_size) {
			std::destroy_n(m_storage.elem + new_size, m_size - new_size);
		} else if (new_size > m_size) {
			VERA_ASSERT_MSG(new_size <= N, "static_vector exceeded max_size");
			std::uninitialized_fill_n(m_storage.elem + m_size, new_size - m_size, value);
		} else /* new_size == m_size */ {
			return;
		}

		m_size = new_size;
	}
	
	void clear() noexcept
	{
		std::destroy_n(m_storage.elem, m_size);
		m_size = 0;
	}

	void swap(static_vector& rhs) noexcept
	{
		// TODO: swap static_vector with different capacities

		if (m_size < rhs.m_size)
			swap_impl(*this, rhs);
		else
			swap_impl(rhs, *this);
	}

	T* data() noexcept
	{
		return m_size > 0 ? m_storage.elem : nullptr;
	}

	const T* data() const noexcept
	{
		return m_size > 0 ? m_storage.elem : nullptr;
	}

	iterator begin() noexcept
	{
		return m_storage.elem;
	}

	const_iterator begin() const noexcept
	{
		return m_storage.elem;
	}

	iterator end() noexcept
	{
		return m_storage.elem + m_size;
	}

	const_iterator end() const noexcept
	{
		return m_storage.elem + m_size;
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
		return m_size == 0;
	}

	size_t size() const noexcept
	{
		return m_size;
	}

	size_t max_size() const noexcept
	{
		return N;
	}

	size_t capacity() const noexcept
	{
		return N;
	}

	T& operator[](const size_t pos) noexcept
	{
		VERA_ASSERT_MSG(pos < m_size, "invalid static_vector subscript");
		return m_storage.elem[pos];
	}

	const T& operator[](const size_t pos) const noexcept
	{
		VERA_ASSERT_MSG(pos < m_size, "invalid static_vector subscript");
		return m_storage.elem[pos];
	}

	T& at(size_t pos)
	{
		VERA_CHECK_MSG(pos < m_size, "invalid static_vector subscript");
		return m_storage.elem[pos];
	}

	const T& at(size_t pos) const
	{
		VERA_CHECK_MSG(pos < m_size, "invalid static_vector subscript");
		return m_storage.elem[pos];
	}

	T& front()
	{
		VERA_ASSERT_MSG(m_size > 0, "front() called on empty static_vector");
		return m_storage.elem[0];
	}

	const T& front() const
	{
		VERA_ASSERT_MSG(m_size > 0, "front() called on empty static_vector");
		return m_storage.elem[0];
	}

	T& back()
	{
		VERA_ASSERT_MSG(m_size > 0, "back() called on empty static_vector");
		return m_storage.elem[m_size - 1];
	}

	const T& back() const
	{
		VERA_ASSERT_MSG(m_size > 0, "back() called on empty static_vector");
		return m_storage.elem[m_size - 1];
	}


private:
	void swap_impl(static_vector& small, static_vector& large) noexcept
	{
		for (size_t i = 0; i < small.m_size; i++)
			std::swap(small.m_storage.elem[i], large.m_storage.elem[i]);

		for (size_t i = small.m_size; i < large.m_size; i++) {
			new (&small.m_storage.elem[i]) T(std::move(large.m_storage.elem[i]));
			std::destroy_at(large.m_storage.elem + i);
		}

		std::swap(small.m_size, large.m_size);
	}

protected:
	priv::uninitialized_storage<T, N> m_storage;
	size_t                            m_size;
};

VERA_NAMESPACE_END