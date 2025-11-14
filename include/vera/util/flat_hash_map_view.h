#pragma once

#include "flat_hash_map.h"
#include <utility>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN



VERA_PRIV_NAMESPACE_END

template <class FlatHashMapType>
class flat_hash_map_view
{
	using table_type    = typename FlatHashMapType::Table;
	using entry_type    = typename table_type::Entry;
	using entry_pointer = typename table_type::EntryPointer;
	using hash_policy   = typename FlatHashMapType::HashPolicy;

public:
	using key_type        = typename FlatHashMapType::key_type;
	using mapped_type     = typename FlatHashMapType::mapped_type;
	using value_type      = typename FlatHashMapType::value_type;
	using size_type       = size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using pointer         = value_type *;
    using const_pointer   = const value_type *;
	using iterator        = typename table_type::const_iterator;

	flat_hash_map_view() :
		m_entries(nullptr),
		m_num_slots_minus_one(0),
		m_hash_policy(),
		m_max_lookups(0) {}

	flat_hash_map_view(const FlatHashMapType& hash_map) :
		m_entries(hash_map.entries),
		m_num_slots_minus_one(hash_map.num_slots_minus_one),
		m_hash_policy(hash_map.hash_policy),
		m_max_lookups(hash_map.max_lookups)

	const mapped_type& at(const key_type& key) const
	{
		auto found = this->find(key);
		if (found == this->end())
			throw std::out_of_range("Argument passed to at() was not in the map.");
		return found->second;
	}

	iterator find(const key_type & key) const
	{
		return const_cast<sherwood_v3_table *>(this)->find(key);
	}

	bool empty() const
	{
		return m_entries == nullptr;
	}

	iterator begin() const
	{
		for (EntryPointer it = entries;; ++it)
			if (it->has_value())
				return { it };
	}

	iterator end() const
	{
		return { entries + static_cast<ptrdiff_t>(num_slots_minus_one + max_lookups) };
	}

	iterator cbegin() const
	{
		return begin();
	}

	iterator cend() const
	{
		return end();
	}

private:
    entry_pointer m_entries;
    size_t        m_num_slots_minus_one;
    hash_policy   m_hash_policy;
    int8_t        m_max_lookups;
};

VERA_NAMESPACE_END
