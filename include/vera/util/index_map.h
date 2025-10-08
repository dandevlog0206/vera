#pragma once

#include "../core/assertion.h"
#include <stdexcept>
#include <limits>
#include <memory>
#include <bit>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

template <class MapType>
class index_map_const_iterator
{
	friend MapType;
public:
	using iterator_category = std::bidirectional_iterator_tag;
	using key_type          = typename MapType::key_type;
	using mapped_type       = typename MapType::mapped_type;
	using value_type        = typename MapType::value_type;
	using size_type         = size_t;
	using difference_type   = std::ptrdiff_t;
	using pointer           = const value_type*;
	using reference         = const value_type&;

	static VERA_CONSTEXPR key_type magic_number = std::numeric_limits<key_type>::max();

	VERA_CONSTEXPR index_map_const_iterator() VERA_NOEXCEPT :
		m_ptr(nullptr),
		m_end(nullptr) {}

	VERA_CONSTEXPR index_map_const_iterator(const value_type* ptr, const value_type* end) VERA_NOEXCEPT :
		m_ptr(ptr),
		m_end(end) {}

	VERA_NODISCARD VERA_CONSTEXPR reference operator*() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_ptr <= m_end, "iterator out of range");
		return *m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR pointer operator->() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_ptr <= m_end, "iterator out of range");
		return m_ptr;
	}

	VERA_CONSTEXPR index_map_const_iterator& operator++() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_ptr < m_end, "iterator out of range");

		while (m_ptr < m_end && (++m_ptr)->first == magic_number);
		
		return *this;
	}

	VERA_CONSTEXPR index_map_const_iterator operator++(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		++*this;
		
		return temp;
	}

	VERA_CONSTEXPR index_map_const_iterator& operator--() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_ptr > m_end, "iterator out of range");
		
		while (m_ptr > m_end && (--m_ptr)->first == magic_number);
		
		return *this;
	}

	VERA_CONSTEXPR index_map_const_iterator operator--(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		--*this;
		
		return temp;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const index_map_const_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_end == rhs.m_end, "comparing iterators from different containers");

		return m_ptr == rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const index_map_const_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_end == rhs.m_end, "comparing iterators from different containers");

		return m_ptr != rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const index_map_const_iterator& rhs) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_end == rhs.m_end, "comparing iterators from different containers");

		return m_ptr != rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const index_map_const_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(rhs < *this);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const index_map_const_iterator& rhs) const VERA_NOEXCEPT
	{
		return rhs < *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const index_map_const_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this < rhs);
	}

private:
	const value_type* m_ptr;
	const value_type* m_end;
};

template <class MapType>
class index_map_iterator : public index_map_const_iterator<MapType>
{
	using base_type = index_map_const_iterator<MapType>;

public:
	using iterator_category = std::bidirectional_iterator_tag;
	using key_type          = typename MapType::key_type;
	using mapped_type       = typename MapType::mapped_type;
	using value_type        = typename MapType::value_type;
	using size_type         = size_t;
	using difference_type   = std::ptrdiff_t;
	using pointer           = value_type*;
	using reference         = value_type&;

	static VERA_CONSTEXPR key_type magic_number = std::numeric_limits<key_type>::max();

	using base_type::base_type;

	VERA_NODISCARD VERA_CONSTEXPR const reference operator*() const VERA_NOEXCEPT
	{
		return base_type::operator*();
	}

	VERA_NODISCARD VERA_CONSTEXPR reference operator*() VERA_NOEXCEPT
	{
		return const_cast<reference>(base_type::operator*());
	}

	VERA_NODISCARD VERA_CONSTEXPR const pointer operator->() const VERA_NOEXCEPT
	{
		return base_type::operator->();
	}

	VERA_NODISCARD VERA_CONSTEXPR pointer operator->() VERA_NOEXCEPT
	{
		return const_cast<pointer>(base_type::operator->());
	}
};

VERA_PRIV_NAMESPACE_END

template <class UIntKey, class T>
class index_map
{
public:
	using this_type       = index_map<UIntKey, T>;
	using key_type        = UIntKey;
	using mapped_type     = T;
	using value_type      = std::pair<key_type, mapped_type>;
	using pointer         = value_type*;
	using const_pointer   = const value_type*;
	using reference       = value_type&;
	using const_reference = const value_type&;
	using size_type       = size_t;
	using difference_type = std::ptrdiff_t;
	using iterator        = priv::index_map_iterator<this_type>;
	using const_iterator  = priv::index_map_const_iterator<this_type>;

	static VERA_CONSTEXPR key_type magic_number = std::numeric_limits<key_type>::max();

	VERA_CONSTEXPR index_map() VERA_NOEXCEPT :
		m_bucket(nullptr),
		m_capacity(0),
		m_size(0),
		m_mask(0) {}

	VERA_CONSTEXPR index_map(size_t elem_count) :
		index_map()
	{
		size_t bucket_size = 1ull << std::bit_width(elem_count);

		VERA_ASSERT_MSG(bucket_size - 1 <= std::numeric_limits<size_t>::max(), "bucket size is too large");
		VERA_ASSERT_MSG((bucket_size & (bucket_size - 1)) == 0 && 1 < bucket_size, "bucket size must be power of 2");
		VERA_ASSERT_MSG(1 < bucket_size, "bucket size must be greater than 1");

		if (bucket_size == 0) return;

		m_bucket   = alloc_bucket(bucket_size);
		m_capacity = bucket_size;
		m_mask     = make_mask(bucket_size);

		memset(m_bucket, magic_number, bucket_size * sizeof(value_type));
	}

	template <class Iter>
	VERA_CONSTEXPR index_map(Iter first, Iter last) :
		index_map()
	{
		insert(first, last);
	}

	VERA_CONSTEXPR index_map(const this_type& rhs) :
		index_map()
	{
		if (rhs.m_size == 0) return;

		m_bucket   = alloc_bucket(rhs.m_capacity);
		m_capacity = rhs.m_capacity;
		m_size     = rhs.m_size;
		m_mask     = rhs.m_mask;

		memset(m_bucket, magic_number, m_capacity * sizeof(value_type));

		for (size_t i = 0; i < m_capacity; ++i)
			if (rhs.m_bucket[i].first != magic_number)
				std::construct_at(m_bucket + i, rhs.m_bucket[i]);
	}

	VERA_CONSTEXPR index_map(this_type&& rhs) VERA_NOEXCEPT :
		m_bucket(std::exchange(rhs.m_bucket, nullptr)),
		m_capacity(std::exchange(rhs.m_capacity, 0)),
		m_size(std::exchange(rhs.m_size, 0)),
		m_mask(std::exchange(rhs.m_mask, 0)) {}

	VERA_CONSTEXPR ~index_map()
	{
		destroy_all();
	}

	VERA_CONSTEXPR this_type& operator=(const this_type& rhs)
	{
		// TODO: implement properly lator
		this_type temp(rhs);
		swap(temp);
		
		return *this;
	}

	VERA_CONSTEXPR this_type& operator=(this_type&& rhs) VERA_NOEXCEPT
	{
		destroy_all();

		m_bucket   = std::exchange(rhs.m_bucket, nullptr);
		m_capacity = std::exchange(rhs.m_capacity, 0);
		m_size     = std::exchange(rhs.m_size, 0);
		m_mask     = std::exchange(rhs.m_mask, 0);

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR const mapped_type& at(key_type key) const
	{
		if (size_t idx = hash(key); idx < m_capacity)
			if (const auto& item = m_bucket[idx]; item.first == key)
				return item.second;

		throw std::out_of_range("key not found in index_map");
	}

	VERA_NODISCARD VERA_CONSTEXPR mapped_type& at(key_type key)
	{
		return const_cast<mapped_type&>(at(key));
	}

	VERA_NODISCARD VERA_CONSTEXPR mapped_type& operator[](key_type key) VERA_NOEXCEPT
	{
		if (!m_bucket) {
			m_bucket   = alloc_bucket(4);
			m_capacity = 4;
			m_size     = 1;
			m_mask     = make_mask(4);

			memset(m_bucket, magic_number, m_capacity * sizeof(value_type));
		
			return *std::construct_at(std::addressof(m_bucket[hash(key)].second));
		}

		size_t idx = hash(key);

		if (m_capacity <= idx) {
			rehash(m_capacity << 1);
			idx = hash(key);
		}

		auto& item = m_bucket[idx];

		if (item.first == key) {
			return item.second;
		} else if (item.first == magic_number) {
			item.first = key;
			m_size    += 1;

			return *std::construct_at(std::addressof(item.second));
		} else /* item.first != key */ {
			size_t new_capacity = make_new_capacity(item.first, key);

			// hash collison occured, rehash to larger capacity
			rehash(new_capacity);

			auto& new_item = m_bucket[hash(key)];

			new_item.first = key;
			m_size        += 1;

			return *std::construct_at(std::addressof(new_item.second));
		}
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator find(key_type key) const VERA_NOEXCEPT
	{
		if (size_t idx = hash(key); idx < m_capacity)
			if (const auto& item = m_bucket[idx]; item.first == key)
				return const_iterator(std::addressof(item), m_bucket + m_capacity);

		return cend();
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator find(key_type key) VERA_NOEXCEPT
	{
		if (size_t idx = hash(key); idx < m_capacity)
			if (const auto& item = m_bucket[idx]; item.first == key)
				return iterator(std::addressof(item), m_bucket + m_capacity);
		
		return end();
	}

	VERA_CONSTEXPR std::pair<iterator, bool> insert(const value_type& value)
	{
		if (!m_bucket) {
			m_bucket   = alloc_bucket(2);
			m_capacity = 2;
			m_size     = 1;
			m_mask     = make_mask(2);
			memset(m_bucket, magic_number, m_capacity * sizeof(value_type));
		
			auto& item = m_bucket[hash(value.first)];
			item.first = value.first;
			
			std::construct_at(std::addressof(item.second), value.second);

			return { iterator(std::addressof(item), m_bucket + m_capacity), true };
		}

		size_t idx = hash(value.first);

		if (m_capacity <= idx) {
			rehash(m_capacity << 1);
			idx = hash(value.first);
		}

		auto& item = m_bucket[idx];

		if (item.first == magic_number) {
			item.first = value.first;
			m_size    += 1;
			
			std::construct_at(std::addressof(item.second), value.second);

			return { iterator(std::addressof(item), m_bucket + m_capacity), true };
		} else if (item.first == value.first) {
			item.second = value.second;

			return { iterator(std::addressof(item), m_bucket + m_capacity), false };
		} else /* item.first != key */ {
			size_t new_capacity = make_new_capacity(item.first, value.first);

			// hash collison occured, rehash to larger capacity
			rehash(new_capacity);

			auto& new_item = m_bucket[hash(value.first)];

			new_item.first = value.first;
			m_size        += 1;
			
			std::construct_at(std::addressof(new_item.second), value.second);

			return { iterator(std::addressof(new_item), m_bucket + m_capacity), true };
		}
	}

	VERA_CONSTEXPR std::pair<iterator, bool> insert(value_type&& value)
	{
		if (!m_bucket) {
			m_bucket   = alloc_bucket(2);
			m_capacity = 2;
			m_size     = 1;
			m_mask     = make_mask(2);
			memset(m_bucket, magic_number, m_capacity * sizeof(value_type));
		
			auto& item = m_bucket[hash(value.first)];
			item.first = value.first;
			
			std::construct_at(std::addressof(item.second), std::move(value.second));

			return { iterator(std::addressof(item), m_bucket + m_capacity), true };
		}

		size_t idx = hash(value.first);

		if (m_capacity <= idx) {
			rehash(m_capacity << 1);
			idx = hash(value.first);
		}

		auto& item = m_bucket[idx];

		if (item.first == magic_number) {
			item.first = value.first;
			m_size    += 1;
			
			std::construct_at(std::addressof(item.second), std::move(value.second));
		
			return { iterator(std::addressof(item), m_bucket + m_capacity), true };
		} else if (item.first == value.first) {
			item.second = std::move(value.second);

			return { iterator(std::addressof(item), m_bucket + m_capacity), false };
		} else /* item.first != key */ {
			size_t new_capacity = make_new_capacity(item.first, value.first);

			// hash collison occured, rehash to larger capacity
			rehash(new_capacity);

			auto& new_item = m_bucket[hash(value.first)];

			new_item.first = value.first;
			m_size        += 1;
			
			std::construct_at(std::addressof(new_item.second), std::move(value.second));

			return { iterator(std::addressof(new_item), m_bucket + m_capacity), true };
		}
	}

	template <class Iter>
	VERA_CONSTEXPR void insert(Iter first, Iter last)
	{
		// TODO: make more efficient
		for (; first != last; ++first)
			insert(*first);
	}

	// TODO: implement emplace lator

	VERA_CONSTEXPR void rehash_for(size_t elem_count)
	{
		if (elem_count == 0) {
			destroy_all();
			m_bucket   = nullptr;
			m_capacity = 0;
			m_size     = 0;
			m_mask     = 0;
			return;
		}

		size_t new_bucket_size = 1ull << std::bit_width(elem_count);
		
		if (m_capacity < new_bucket_size)
			rehash(new_bucket_size);
	}

	VERA_CONSTEXPR void rehash(size_t new_bucket_size)
	{
		if (new_bucket_size == 0) {
			destroy_all();
			m_bucket   = nullptr;
			m_capacity = 0;
			m_size     = 0;
			m_mask     = 0;

			return;
		} else if (m_capacity == new_bucket_size) {
			return;
		}

		VERA_ASSERT_MSG(new_bucket_size - 1 <= std::numeric_limits<size_t>::max(), "bucket size is too large");
		VERA_ASSERT_MSG((new_bucket_size & (new_bucket_size - 1)) == 0, "bucket size must be power of 2");
		VERA_ASSERT_MSG(1 < new_bucket_size, "bucket size must be greater than 1");

		auto* new_bucket = alloc_bucket(new_bucket_size);
		auto  new_mask   = make_mask(new_bucket_size);

		memset(new_bucket, magic_number, new_bucket_size * sizeof(value_type));
		
		for (size_t i = 0; i < m_capacity; ++i) {
			if (m_bucket[i].first != magic_number) {
				auto& item = m_bucket[i];
				auto  idx  = hash(item.first, new_mask);
				
				VERA_ASSERT_MSG(new_bucket[idx].first == magic_number, "rehash failed due to hash collision");
				
				if constexpr (std::is_move_constructible_v<T>)
					std::construct_at(new_bucket + idx, std::move(item));
				else // copy construct
					std::construct_at(new_bucket + idx, item);
			}
		}
		
		destroy_all();
		
		m_bucket   = new_bucket;
		m_capacity = new_bucket_size;
		m_mask     = new_mask;
	}

	VERA_CONSTEXPR bool erase(const_iterator pos) VERA_NOEXCEPT
	{
		auto* raw_ptr = const_cast<value_type*>(pos.m_ptr);

		VERA_ASSERT_MSG(m_bucket < raw_ptr && raw_ptr < m_bucket + m_capacity, "invalidated iterator");

		if (!m_bucket) return false;
		
		raw_ptr->first = magic_number;
		m_size        -= 1;

		if constexpr (!std::is_trivially_destructible_v<T>)
			std::destroy_at(std::addressof(raw_ptr->second));

		return false;
	}

	VERA_CONSTEXPR void erase(const_iterator first, const_iterator last) VERA_NOEXCEPT
	{
		auto* raw_first = const_cast<value_type*>(first.m_ptr);
		auto* raw_last  = const_cast<value_type*>(last.m_ptr);

		VERA_ASSERT_MSG(m_bucket <= raw_first && raw_first <= m_bucket + m_capacity, "invalidated iterator");
		VERA_ASSERT_MSG(m_bucket <= raw_last && raw_last <= m_bucket + m_capacity, "invalidated iterator");
		VERA_ASSERT_MSG(raw_first <= raw_last, "invalid range");

		for (auto* ptr = raw_first; ptr != raw_last; ++ptr) {
			if (ptr->first != magic_number) {
				ptr->first = magic_number;
				m_size    -= 1;

				if constexpr (!std::is_trivially_destructible_v<T>)
					std::destroy_at(std::addressof(ptr->second));
			}
		}
	}

	VERA_CONSTEXPR bool erase(key_type key) VERA_NOEXCEPT
	{
		if (size_t idx = hash(key); idx < m_capacity) {
			if (auto& item = m_bucket[idx]; item.first == key) {
				item.first = magic_number;
				m_size    -= 1;

				if constexpr (!std::is_trivially_destructible_v<T>)
					std::destroy_at(std::addressof(item.second));

				return true;
			}
		}

		return false;
	}

	VERA_CONSTEXPR void clear()
	{
		if (!m_bucket) return;

		for (size_t i = 0; i < m_capacity; ++i) {
			if (m_bucket[i].first != magic_number) {
				m_bucket[i].first = magic_number;
				
				if constexpr (!std::is_trivially_destructible_v<T>)
					std::destroy_at(std::addressof(m_bucket[i].second));
			}
		}

		m_size = 0;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t max_size() const VERA_NOEXCEPT
	{
		return std::numeric_limits<key_type>::max() - 1;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		return m_size;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t capacity() const VERA_NOEXCEPT
	{
		return m_capacity;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_size == 0;
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator begin() const VERA_NOEXCEPT
	{
		auto* first = m_bucket;
		auto* last  = m_bucket + m_capacity;

		while (first < last && first->first == magic_number) ++first;

		return const_iterator(first, last);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator begin() VERA_NOEXCEPT
	{
		auto* first = m_bucket;
		auto* last  = m_bucket + m_capacity;

		while (first < last && first->first == magic_number) ++first;

		return iterator(first, last);
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator end() const VERA_NOEXCEPT
	{
		return const_iterator(m_bucket + m_capacity, m_bucket + m_capacity);
	}

	VERA_NODISCARD VERA_CONSTEXPR iterator end() VERA_NOEXCEPT
	{
		return iterator(m_bucket + m_capacity, m_bucket + m_capacity);
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cbegin() const VERA_NOEXCEPT
	{
		return begin();
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cend() const VERA_NOEXCEPT
	{
		return end();
	}

	VERA_CONSTEXPR void swap(this_type& rhs) VERA_NOEXCEPT
	{
		std::swap(m_bucket, rhs.m_bucket);
		std::swap(m_capacity, rhs.m_capacity);
		std::swap(m_size, rhs.m_size);
		std::swap(m_mask, rhs.m_mask);
	}

private:
	VERA_NODISCARD VERA_CONSTEXPR size_t hash(key_type key) const VERA_NOEXCEPT
	{
		return static_cast<size_t>(key & m_mask);
	}

	static VERA_NODISCARD VERA_CONSTEXPR size_t hash(key_type key, key_type mask) VERA_NOEXCEPT
	{
		return static_cast<size_t>(key & mask);
	}
	
	VERA_CONSTEXPR void destroy_all()
	{
		if (!m_bucket) return;
		
		if constexpr (!std::is_trivially_destructible_v<T>) {
			for (size_t i = 0; i < m_capacity; ++i)
				if (m_bucket[i].first != magic_number)
					std::destroy_at(std::addressof(m_bucket[i].second));
		}

		::operator delete(m_bucket);
	}

	static VERA_NODISCARD VERA_CONSTEXPR value_type* alloc_bucket(size_t bucket_size)
	{
		return reinterpret_cast<value_type*>(::operator new(bucket_size * sizeof(value_type)));
	}

	static VERA_NODISCARD VERA_CONSTEXPR key_type make_mask(size_t bucket_size) VERA_NOEXCEPT
	{
		return static_cast<key_type>(bucket_size - 1);
	}

	static VERA_NODISCARD VERA_CONSTEXPR size_t make_new_capacity(key_type key0, key_type key1) VERA_NOEXCEPT
	{
		return static_cast<size_t>(1 << (std::countr_zero(key0 ^ key1) + 1));
	}

private:
	value_type* m_bucket;
	size_t      m_capacity;
	size_t      m_size;
	key_type    m_mask;
};

VERA_NAMESPACE_END