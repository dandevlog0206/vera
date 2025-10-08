#pragma once

#include "../core/assertion.h"
#include "../core/exception.h"
#include <vector>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

template <class RingVectorType>
class ring_vector_const_iterator
{
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type        = typename RingVectorType::value_type;
	using difference_type   = typename RingVectorType::difference_type;
	using pointer           = const typename RingVectorType::pointer;
	using reference         = const typename RingVectorType::reference;

	VERA_CONSTEXPR ring_vector_const_iterator() VERA_NOEXCEPT :
		m_vector(nullptr),
		m_index(0) {}

	VERA_CONSTEXPR ring_vector_const_iterator(const RingVectorType* vec, size_t idx) VERA_NOEXCEPT :
		m_vector(vec),
		m_index(idx) {}

	VERA_NODISCARD VERA_CONSTEXPR reference operator*() const VERA_NOEXCEPT
	{
		return (*m_vector)[m_index];
	}

	VERA_NODISCARD VERA_CONSTEXPR pointer operator->() const VERA_NOEXCEPT
	{
		return std::addressof((*m_vector)[m_index]);
	}

	VERA_CONSTEXPR ring_vector_const_iterator& operator++() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_index < m_vector->size(), "incrementing iterator past end");
		m_index++;
		return *this;
	}

	VERA_CONSTEXPR ring_vector_const_iterator& operator--() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(0 < m_index, "decrementing iterator past beginning");
		--m_index;
		return *this;
	}

	VERA_CONSTEXPR ring_vector_const_iterator operator++(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		++*this;
		return temp;
	}

	VERA_CONSTEXPR ring_vector_const_iterator operator--(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		--*this;
		return temp;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const ring_vector_const_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_vector == rhs.m_vector, "comparing iterators from different containers");
		return m_index < rhs.m_index;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const ring_vector_const_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(rhs < *this);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const ring_vector_const_iterator& rhs) const VERA_NOEXCEPT
	{
		return rhs < *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const ring_vector_const_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this < rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const ring_vector_const_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_vector == rhs.m_vector, "comparing iterators from different containers");
		return m_index == rhs.m_index;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const ring_vector_const_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_vector == rhs.m_vector, "comparing iterators from different containers");
		return m_index != rhs.m_index;
	}

private:
	const RingVectorType* m_vector;
	size_t m_index;
};

template <class RingVectorType>
class ring_vector_iterator : public ring_vector_const_iterator<RingVectorType>
{
	using base_type = ring_vector_const_iterator<RingVectorType>;
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type        = typename RingVectorType::value_type;
	using difference_type   = typename RingVectorType::difference_type;
	using pointer           = typename RingVectorType::pointer;
	using reference         = typename RingVectorType::reference;
	
	using base_type::base_type;

	VERA_NODISCARD VERA_CONSTEXPR reference operator*() const VERA_NOEXCEPT
	{
		return const_cast<reference>(base_type::operator*());
	}

	VERA_NODISCARD VERA_CONSTEXPR pointer operator->() const VERA_NOEXCEPT
	{
		return const_cast<pointer>(base_type::operator->());
	}

	VERA_CONSTEXPR ring_vector_iterator& operator++() VERA_NOEXCEPT
	{
		base_type::operator++();
		return *this;
	}

	VERA_CONSTEXPR ring_vector_iterator& operator--() VERA_NOEXCEPT
	{
		base_type::operator--();
		return *this;
	}

	VERA_CONSTEXPR ring_vector_iterator operator++(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		++*this;
		return temp;
	}

	VERA_CONSTEXPR ring_vector_iterator operator--(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		--*this;
		return temp;
	}
};

VERA_PRIV_NAMESPACE_END

template <class T>
class ring_vector : protected std::vector<T>
{
	using my_base = std::vector<T>;
public:
	using value_type      = typename my_base::value_type;
	using pointer         = typename my_base::pointer;
	using const_pointer   = typename my_base::const_pointer*;
	using reference       = typename my_base::reference;
	using const_reference = typename my_base::const_reference;
	using size_type       = typename my_base::size_type;
	using difference_type = typename my_base::difference_type;

	using iterator       = priv::ring_vector_iterator<ring_vector<T>>;
	using const_iterator = priv::ring_vector_const_iterator<ring_vector<T>>;

	VERA_CONSTEXPR ring_vector() VERA_NOEXCEPT :
		my_base(),
		m_head(0) {}

	VERA_CONSTEXPR ring_vector(size_t count) VERA_NOEXCEPT :
		my_base(count),
		m_head(0) {}

	VERA_CONSTEXPR ring_vector(size_t count, const T& val) VERA_NOEXCEPT :
		my_base(count, val),
		m_head(0) {}

	template <class Iter>
	VERA_CONSTEXPR ring_vector(Iter first, Iter last) VERA_NOEXCEPT :
		my_base(first, last),
		m_head(0) {}

	VERA_CONSTEXPR ring_vector(std::initializer_list<T> ilist) VERA_NOEXCEPT :
		my_base(ilist),
		m_head(0) {}

	VERA_CONSTEXPR ring_vector(const ring_vector& other) VERA_NOEXCEPT :
		my_base(other),
		m_head(other.m_head) {}

	VERA_CONSTEXPR ring_vector(ring_vector&& other) VERA_NOEXCEPT :
		my_base(std::move(other)),
		m_head(std::exchange(other.m_head, 0)) {}

	~ring_vector() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR ring_vector& operator=(const ring_vector& other) VERA_NOEXCEPT
	{
		my_base::operator=(other);
		m_head = other.m_head;
		return *this;
	}

	VERA_CONSTEXPR ring_vector& operator=(ring_vector&& other) VERA_NOEXCEPT
	{
		my_base::operator=(std::move(other));
		m_head = std::exchange(other.m_head, 0);
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& at(size_t idx) const VERA_NOEXCEPT {
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");
		return my_base::at((m_head + idx) % my_base::size());
	}

	VERA_NODISCARD VERA_CONSTEXPR T& at(size_t idx) VERA_NOEXCEPT {
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");
		return my_base::at((m_head + idx) % my_base::size());
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& operator[](size_t idx) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");
		return my_base::operator[]((m_head + idx) % my_base::size());
	}

	VERA_NODISCARD VERA_CONSTEXPR T& operator[](size_t idx) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");
		return my_base::operator[]((m_head + idx) % my_base::size());
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& front() const VERA_NOEXCEPT
	{
		return my_base::operator[](m_head);
	}

	VERA_NODISCARD VERA_CONSTEXPR T& front() VERA_NOEXCEPT
	{
		return my_base::operator[](m_head);
	}

	VERA_NODISCARD VERA_CONSTEXPR const T& back() const VERA_NOEXCEPT
	{
		auto size = my_base::size();

		return my_base::operator[]((m_head + size - 1) % size);
	}

	VERA_NODISCARD VERA_CONSTEXPR T& back() VERA_NOEXCEPT
	{
		auto size = my_base::size();
		
		return my_base::operator[]((m_head + size - 1) % size);
	}

	VERA_CONSTEXPR void rotate_forward() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");
		m_head = (m_head + 1) % my_base::size();
	}

	VERA_CONSTEXPR void rotate_forward(size_t n) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");
		m_head = (m_head + n) % my_base::size();
	}

	VERA_CONSTEXPR void rotate_backward() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");

		auto size = my_base::size();

		m_head = (m_head + size - 1) % size;
	}

	VERA_CONSTEXPR void rotate_backward(size_t n) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(!my_base::empty(), "ring_vector is empty");

		auto size = my_base::size();

		m_head = (m_head + size - (n % size)) % size;
	}

	VERA_CONSTEXPR void push_front(const T& value)
	{
		my_base::emplace(my_base::cbegin() + m_head, value);
	}

	VERA_CONSTEXPR void push_front(T&& value)
	{
		my_base::emplace(my_base::cbegin() + m_head, std::move(value));
	}

	VERA_CONSTEXPR void push_back(const T& value)
	{
		my_base::emplace(my_base::cbegin() + m_head, value);
		m_head += 1;
	}

	VERA_CONSTEXPR void push_back(T&& value)
	{
		my_base::emplace(my_base::cbegin() + m_head, std::move(value));
		m_head += 1;
	}

	VERA_CONSTEXPR void pop_front() VERA_NOEXCEPT
	{
		my_base::erase(my_base::cbegin() + m_head);
	}

	VERA_CONSTEXPR void pop_back() VERA_NOEXCEPT
	{
		auto size = my_base::size();

		m_head = (size + m_head - 1) % size;
		my_base::erase(my_base::cbegin() + m_head);
	}

	VERA_CONSTEXPR void resize(size_t new_size) VERA_NOEXCEPT
	{
		auto size = my_base::size();

		if (size < new_size) {
			auto diff = new_size - size;

			my_base::insert(my_base::cbegin() + m_head, diff, T{});
			m_head += diff;
		} else if (new_size < size){
			auto diff = size - new_size;

			if (m_head < diff) {
				auto back_first = my_base::cend() - (diff - m_head);
				auto back_last  = my_base::cend();

				my_base::erase(back_first, back_last);
			}

			auto new_head    = std::max<difference_type>(m_head - diff, 0);
			auto front_first = my_base::cbegin() + new_head;
			auto front_last  = my_base::cbegin() + m_head;

			my_base::erase(front_first, front_last);

			m_head = new_head;
		}
	}

	VERA_CONSTEXPR void resize(size_t new_size, const T& value) VERA_NOEXCEPT
	{
		auto size = my_base::size();

		if (size < new_size) {
			auto diff = new_size - size;

			my_base::insert(my_base::cbegin() + m_head, diff, value);
			m_head += diff;
		} else if (new_size < size){
			auto diff = size - new_size;

			if (m_head < diff) {
				auto back_first = my_base::cend() - (diff - m_head);
				auto back_last  = my_base::cend();

				my_base::erase(back_first, back_last);
			}

			auto new_head    = std::max<difference_type>(m_head - diff, 0);
			auto front_first = my_base::cbegin() + new_head;
			auto front_last  = my_base::cbegin() + m_head;

			my_base::erase(front_first, front_last);

			m_head = new_head;
		}
	}

	VERA_CONSTEXPR void clear() VERA_NOEXCEPT
	{
		my_base::clear();
		m_head = 0;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t capacity() const VERA_NOEXCEPT
	{
		return my_base::capacity();
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t max_size() const VERA_NOEXCEPT
	{
		return my_base::max_size();
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return my_base::size();
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return my_base::empty();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator begin() const VERA_NOEXCEPT
	{
		return const_iterator(this, 0);
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator end() const VERA_NOEXCEPT
	{
		return const_iterator(this, my_base::size());
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() VERA_NOEXCEPT
	{
		return iterator(this, 0);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() VERA_NOEXCEPT
	{
		return iterator(this, my_base::size());
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cbegin() const VERA_NOEXCEPT
	{
		return begin();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cend() const VERA_NOEXCEPT
	{
		return end();
	}

private:
	size_t m_head;
};

VERA_NAMESPACE_END