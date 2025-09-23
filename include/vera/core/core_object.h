#pragma once

#include "assertion.h"
#include <type_traits>
#include <utility>
#include <memory>
#include <atomic>

#define VERA_CORE_OBJECT(obj_name)                                             \
	using impl_type = struct obj_name ## Impl;                                 \
	using pair_type = ::vr::priv::compressed_pair<obj_name, obj_name ## Impl>; \
	friend class CoreObject;                                                   \
	friend class ref<obj_name>;                                                \
	friend pair_type;                                                          \
	obj_name() = default;

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

typedef class WeakNode* WeakNodePtr;
typedef class WeakNodeLink* WeakNodeLinkPtr;

template <class Type1, class Type2>
struct compressed_pair : public Type1
{
	compressed_pair() = default;

	Type2 second;
};

class WeakNodeLink
{
public:
	WeakNodePtr next;
};

class WeakNode : public WeakNodeLink
{
public:
	inline void link(WeakNode& node) noexcept
	{
		if (!ptr) return;

		VERA_ASSERT_MSG(!node.ptr && !node.prev && !node.next, "node must be empty");

		if (next) {
			next->prev = &node;
			node.next  = next;
		}

		next      = &node;
		node.prev = this;
		node.ptr  = ptr;
	}

	inline void exchange(WeakNode& node) noexcept
	{
		if (!ptr) return;

		if (next)
			next->prev = &node;
		
		VERA_ASSERT(prev);
		prev->next = &node;

		VERA_ASSERT_MSG(!node.ptr && !node.prev && !node.next, "node must be empty");

		node.next = std::exchange(next, nullptr);
		node.prev = std::exchange(prev, nullptr);
		node.ptr  = std::exchange(ptr, nullptr);
	}

	inline void invalidate() noexcept
	{
		if (!ptr) return;

		if (next)
			next->prev = prev;

		VERA_ASSERT(prev);
		prev->next = next;

		next = nullptr;
		prev = nullptr;
		ptr  = nullptr;
	}

	WeakNodeLinkPtr prev;
	void*           ptr;
};

VERA_PRIV_NAMESPACE_END

class CoreObject
{
	VERA_NOCOPY(CoreObject)
	VERA_NOMOVE(CoreObject)

	template <class Object>
	friend class ref;
	template <class Object>
	friend class weak_ref;

public:
	template <class Object>
	static typename Object::impl_type& getImpl(ref<Object>& obj)
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	static const typename Object::impl_type& getImpl(const ref<Object>& obj)
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<const typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	static typename Object::impl_type& getImpl(Object* ptr)
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(ptr)->second;
	}

	template <class Object>
	static const typename Object::impl_type& getImpl(const Object* ptr)
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<const typename Object::pair_type*>(ptr)->second;
	}

protected:
	CoreObject() :
		m_atomic(),
		m_weak_chain(nullptr) {}

	template <class Object>
	static ref<Object> createNewObject()
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return ref<Object>(static_cast<Object*>(new typename Object::pair_type));
	}

	template <class Object>
	static void destroyObjectImpl(Object* ptr)
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		std::destroy_at(&static_cast<typename Object::pair_type*>(ptr)->second);
	}

private:
	template <class Object>
	static void destroyObject(Object* ptr)
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);
		delete ptr;
	}

private:
	std::atomic<uint64_t> m_atomic;
	priv::WeakNodeLink    m_weak_chain;
};

template <class Object>
class ref
{
public:
	ref() :
		m_ptr(nullptr) {}
	
	ref(std::nullptr_t) :
		m_ptr(nullptr) {}

	ref(Object* ptr) :
		m_ptr(ptr)
	{
		increase();
	}

	ref(const ref& rhs) :
		m_ptr(rhs.m_ptr)
	{
		increase();
	}

	ref(ref&& rhs) noexcept :
		m_ptr(std::exchange(rhs.m_ptr, nullptr)) {}

	~ref()
	{
		decrease();
	}

	ref& operator=(const ref& rhs)
	{
		if (std::addressof(rhs) == this) return *this;

		decrease();
		m_ptr = rhs.m_ptr;
		increase();

		return *this;
	}

	ref& operator=(ref&& rhs) noexcept
	{
		if (std::addressof(rhs) == this) return *this;

		decrease();
		m_ptr = std::exchange(rhs.m_ptr, nullptr);

		return *this;
	}

	uint64_t count() const
	{
		return m_ptr->m_atomic;
	}

	const Object* get() const
	{
		return m_ptr;
	}

	Object* get()
	{
		return m_ptr;
	}

	const Object& unwrap() const
	{
		return *reinterpret_cast<const Object*>(m_ptr);
	}

	Object* unwrap()
	{
		return *reinterpret_cast<Object*>(m_ptr);
	}

	void reset()
	{
		decrease();
	}

	Object* operator*()
	{
		return m_ptr;
	}

	const Object* operator*() const
	{
		return m_ptr;
	}

	Object* operator->()
	{
		return m_ptr;
	}

	const Object* operator->() const
	{
		return m_ptr;
	}

	bool operator==(const ref& rhs) const
	{
		return m_ptr == rhs.m_ptr;
	}

	bool operator!=(const ref& rhs) const
	{
		return m_ptr != rhs.m_ptr;
	}

	operator bool() const
	{
		return static_cast<bool>(m_ptr);
	}

private:
	void increase()
	{
		if (m_ptr)
			m_ptr->m_atomic.fetch_add(1);
	}

	void decrease()
	{
		if (m_ptr && m_ptr->m_atomic.fetch_sub(1) == 1) {
			invalidate_all_weak();
			CoreObject::destroyObject(m_ptr);
		}

		m_ptr = nullptr;
	}

	void invalidate_all_weak()
	{
		if (!m_ptr || !m_ptr->m_weak_chain.next) return;

		auto* next_node = std::exchange(m_ptr->m_weak_chain.next, nullptr);

		while (next_node) {
			next_node->ptr  = nullptr;
			next_node->prev = nullptr;
			next_node       = std::exchange(next_node->next, nullptr);
		}
	}

private:
	Object* m_ptr;
};

template <class Object>
class weak_ref
{
	template <class Object2>
	friend class ref;
public:
	weak_ref() = default;
	
	weak_ref(std::nullptr_t) :
		m_node() {}

	weak_ref(ref<Object>& rhs)
	{
		auto* obj_ptr = rhs.get();

		m_node.next = std::exchange(obj_ptr->m_weak_chain.next, &m_node);
		m_node.prev = &obj_ptr->m_weak_chain;
		m_node.ptr  = obj_ptr;
	}

	weak_ref(const weak_ref& rhs)
	{
		rhs.m_node.link(m_node);
	}

	weak_ref(weak_ref&& rhs) noexcept
	{
		rhs.m_node.exchange(m_node);
	}

	~weak_ref()
	{
		m_node.invalidate();
	}

	weak_ref& operator=(const weak_ref& rhs)
	{
		if (std::addressof(rhs) == this) return *this;

		m_node.invalidate();
		rhs.m_node.link(m_node);

		return *this;
	}

	weak_ref& operator=(weak_ref&& rhs) noexcept
	{
		if (std::addressof(rhs) == this) return *this;
	
		m_node.invalidate();
		rhs.m_node.exchange(m_node);

		return *this;
	}

	const Object* get() const
	{
		return m_node.ptr;
	}

	Object* get()
	{
		return m_node.ptr;
	}

	const Object& unwrap() const
	{
		return *reinterpret_cast<const Object*>(m_node.ptr);
	}

	Object& unwrap()
	{
		return *reinterpret_cast<Object*>(m_node.ptr);
	}

	void reset()
	{
		m_node.invalidate();
	}

	Object* operator*()
	{
		return m_node.ptr;
	}

	const Object* operator*() const
	{
		return m_node.ptr;
	}

	Object* operator->()
	{
		return m_node.ptr;
	}

	const Object* operator->() const
	{
		return m_node.ptr;
	}

	bool operator==(const weak_ref& rhs) const
	{
		return m_node.ptr == rhs.m_node.ptr;
	}

	bool operator!=(const weak_ref& rhs) const
	{
		return m_node.ptr != rhs.m_node.ptr;
	}

	operator bool() const
	{
		return static_cast<bool>(m_node.ptr);
	}

private:
	priv::WeakNode m_node;
};

VERA_NAMESPACE_END