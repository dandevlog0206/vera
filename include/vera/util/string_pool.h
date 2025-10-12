#pragma once

#include "../core/assertion.h"
#include <string_view>
#include <string>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

struct string_pool_node
{
	string_pool_node* m_next;
	char              m_string[];
};

class string_pool_iterator
{
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = const char*;
	using difference_type = std::ptrdiff_t;

	VERA_CONSTEXPR string_pool_iterator() VERA_NOEXCEPT :
	m_ptr(nullptr) {}

	VERA_CONSTEXPR string_pool_iterator(priv::string_pool_node* ptr) VERA_NOEXCEPT :
		m_ptr(ptr) {}

	VERA_NODISCARD VERA_CONSTEXPR const char* operator*() const VERA_NOEXCEPT
	{
		return m_ptr->m_string;
	}

	VERA_INLINE string_pool_iterator& operator++() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(m_ptr, "incrementing iterator past end");
		
		m_ptr = m_ptr->m_next;

		return *this;
	}

	VERA_INLINE string_pool_iterator operator++(int) VERA_NOEXCEPT
	{
		auto temp = *this;
		
		++*this;
		
		return temp;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const string_pool_iterator& rhs) const VERA_NOEXCEPT
	{
		return m_ptr == rhs.m_ptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const string_pool_iterator& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

private:
	priv::string_pool_node* m_ptr;
};

VERA_PRIV_NAMESPACE_END

// A simple and fast string pool for storing strings with contiguous memory
class string_pool
{
	using node_t     = priv::string_pool_node;
	using node_ptr_t = priv::string_pool_node*;

public:
	using value_type = const char*;

	using iterator = priv::string_pool_iterator;
	using const_iterator = priv::string_pool_iterator;

	VERA_CONSTEXPR string_pool() VERA_NOEXCEPT :
		m_first(nullptr),
		m_last(nullptr) {}
	
	VERA_CONSTEXPR string_pool(const string_pool&) VERA_NOEXCEPT = delete;
	
	VERA_CONSTEXPR string_pool(string_pool&& rhs) VERA_NOEXCEPT :
		m_first(std::exchange(rhs.m_first, nullptr)),
		m_last(std::exchange(rhs.m_last, nullptr)) {}

	~string_pool() VERA_NOEXCEPT
	{
		clear();
	}

	VERA_CONSTEXPR string_pool& operator=(const string_pool&) VERA_NOEXCEPT = delete;

	VERA_CONSTEXPR string_pool& operator=(string_pool&& rhs) VERA_NOEXCEPT
	{
		if (this != &rhs) {
			clear();

			m_first = std::exchange(rhs.m_first, nullptr);
			m_last  = std::exchange(rhs.m_last, nullptr);
		}

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* operator<<(std::string_view str)
	{
		return push_back(str);
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* operator<<(const std::string& str)
	{
		return push_back(str);
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* operator<<(const char* str)
	{
		return push_back(str);
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* operator[](std::string_view str)
	{
		return intern(str);
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* operator[](const std::string& str)
	{
		return intern(str);
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* operator[](const char* str)
	{
		return intern(str);
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* push_back(std::string_view str)
	{
		if (!m_first) {
			m_first = allocate_node(str);
			m_last  = m_first;

			return m_first->m_string;
		}

		m_last->m_next = allocate_node(str);
		m_last         = m_last->m_next;

		return m_last->m_string;
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* push_back(const std::string& str)
	{
		return push_back(std::string_view(str));
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* push_back(const char* str)
	{
		return push_back(std::string_view(str));
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* intern(std::string_view str)
	{
		for (node_ptr_t node = m_first; node; node = node->m_next)
			if (std::strcmp(node->m_string, str.data()) == 0)
				return node->m_string;

		return push_back(str);
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* intern(const std::string& str)
	{
		return intern(std::string_view(str));
	}

	VERA_NODISCARD VERA_CONSTEXPR const char* intern(const char* str)
	{
		return intern(std::string_view(str));
	}

	VERA_CONSTEXPR void clear() VERA_NOEXCEPT
	{
		while (m_first)
			::operator delete(std::exchange(m_first, m_first->m_next));

		m_last = nullptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR size_t size() const VERA_NOEXCEPT
	{
		size_t count = 0;

		for (node_ptr_t node = m_first; node; node = node->m_next, ++count);

		return count;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_first == nullptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator begin() const VERA_NOEXCEPT
	{
		return const_iterator(m_first);
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator end() const VERA_NOEXCEPT
	{
		return const_iterator(nullptr);
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cbegin() const VERA_NOEXCEPT
	{
		return const_iterator(m_first);
	}

	VERA_NODISCARD VERA_CONSTEXPR const_iterator cend() const VERA_NOEXCEPT
	{
		return const_iterator(nullptr);
	}

private:
	static node_ptr_t allocate_node(std::string_view view)
	{
		auto node = static_cast<node_ptr_t>(::operator new(sizeof(nullptr) + view.size() + 1));

		node->m_next = nullptr;
		std::copy(view.begin(), view.end(), node->m_string);
		node->m_string[view.size()] = '\0';

		return node;
	}

private:
	node_ptr_t m_first;
	node_ptr_t m_last;
};

VERA_NAMESPACE_END