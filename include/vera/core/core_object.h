#pragma once

#include "assertion.h"
#include <type_traits>
#include <utility>
#include <memory>
#include <atomic>

#define VERA_CORE_OBJECT_INIT(obj_name)                                             \
	using impl_type = struct obj_name ## Impl;                                 \
	using pair_type = ::vr::priv::compressed_pair<obj_name, obj_name ## Impl>; \
	friend class CoreObject;                                                   \
	friend class obj<obj_name>;                                                \
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
	VERA_INLINE void link(WeakNode& node) VERA_NOEXCEPT
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

	VERA_INLINE void exchange(WeakNode& node) VERA_NOEXCEPT
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

	VERA_INLINE void invalidate() VERA_NOEXCEPT
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
	friend class obj;
	template <class Object>
	friend class weak_ref;
	template <class Object>
	friend class ref;
	template <class Object>
	friend class const_ref;

public:
	template <class Object>
	VERA_NODISCARD VERA_INLINE static typename Object::impl_type& getImpl(obj<Object>& obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static const typename Object::impl_type& getImpl(const obj<Object>& obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<const typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static typename Object::impl_type& getImpl(weak_ref<Object>& obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static const typename Object::impl_type& getImpl(const weak_ref<Object>& obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<const typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static typename Object::impl_type& getImpl(ref<Object> obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static const typename Object::impl_type& getImpl(const_ref<Object> obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<const typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static typename Object::impl_type& getImpl(Object* ptr) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(ptr)->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static const typename Object::impl_type& getImpl(const Object* ptr) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<const typename Object::pair_type*>(ptr)->second;
	}

protected:
	VERA_INLINE CoreObject() :
		m_atomic(),
		m_weak_chain(nullptr) {}

	template <class Object>
	VERA_INLINE static obj<Object> createNewObject()
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return obj<Object>(static_cast<Object*>(new typename Object::pair_type));
	}

	template <class Object>
	VERA_INLINE static void destroyObjectImpl(Object* ptr) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		std::destroy_at(&static_cast<typename Object::pair_type*>(ptr)->second);
	}

private:
	template <class Object>
	VERA_INLINE static void destroyObject(Object* ptr) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);
		delete ptr;
	}

private:
	std::atomic<uint64_t> m_atomic;
	priv::WeakNodeLink    m_weak_chain;
};

/*
obj<Object>:
	Acts as a reference-counted smart pointer (similar to std::shared_ptr). It automatically increases the reference
	count when copied and decreases it when destroyed. When the count reaches zero, the object is deleted and all
	associated weak references are invalidated.

weak_ref<Object>:
	Provides a non-owning reference to an object managed by obj<>. It does not affect the reference count. If the
	object is destroyed, the weak reference is automatically invalidated (set to null), preventing dangling pointers.

ref<Object>:
	A lightweight, non-owning pointer wrapper. It can be constructed from obj<> or weak_ref<>, but does not manage
	lifetime or reference counting. It simply provides convenient access to the underlying object.
*/

template <class Object>
class ref;

template <class Object>
class obj
{
	template <class Obj>
	friend class const_ref;
public:
	VERA_INLINE obj() VERA_NOEXCEPT :
		m_ptr(nullptr) {}
	
	VERA_INLINE obj(std::nullptr_t) VERA_NOEXCEPT :
		m_ptr(nullptr) {}

	VERA_INLINE obj(Object* ptr) VERA_NOEXCEPT :
		m_ptr(ptr)
	{
		increase();
	}

	VERA_INLINE obj(const obj& rhs) VERA_NOEXCEPT :
		m_ptr(rhs.m_ptr)
	{
		increase();
	}

	VERA_INLINE obj(obj&& rhs) VERA_NOEXCEPT :
		m_ptr(std::exchange(rhs.m_ptr, nullptr)) {}

	VERA_INLINE ~obj() VERA_NOEXCEPT
	{
		decrease();
	}

	VERA_INLINE obj& operator=(const obj& rhs) VERA_NOEXCEPT
	{
		if (std::addressof(rhs) == this) return *this;

		decrease();
		m_ptr = rhs.m_ptr;
		increase();

		return *this;
	}

	VERA_INLINE obj& operator=(obj&& rhs) VERA_NOEXCEPT
	{
		if (std::addressof(rhs) == this) return *this;

		decrease();
		m_ptr = std::exchange(rhs.m_ptr, nullptr);

		return *this;
	}

	VERA_NODISCARD VERA_INLINE uint64_t count() const VERA_NOEXCEPT
	{
		return m_ptr->m_atomic;
	}

	VERA_NODISCARD VERA_INLINE const Object* get() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE Object* get() VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object& unwrap() const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object& unwrap() VERA_NOEXCEPT
	{
		return *reinterpret_cast<Object*>(m_ptr);
	}

	VERA_INLINE void reset() VERA_NOEXCEPT
	{
		decrease();
	}

	VERA_NODISCARD VERA_INLINE Object* operator*() VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object* operator*() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE Object* operator->() VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object* operator->() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator==(const obj& rhs) const VERA_NOEXCEPT
	{
		return m_ptr == rhs.m_ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator!=(const obj& rhs) const VERA_NOEXCEPT
	{
		return m_ptr != rhs.m_ptr;
	}

	VERA_NODISCARD VERA_INLINE operator bool() const VERA_NOEXCEPT
	{
		return static_cast<bool>(m_ptr);
	}

private:
	VERA_INLINE void increase() VERA_NOEXCEPT
	{
		if (m_ptr)
			m_ptr->m_atomic.fetch_add(1);
	}

	VERA_INLINE void decrease() VERA_NOEXCEPT
	{
		if (m_ptr && m_ptr->m_atomic.fetch_sub(1) == 1) {
			invalidate_all_weak();
			CoreObject::destroyObject(m_ptr);
		}

		m_ptr = nullptr;
	}

	VERA_INLINE void invalidate_all_weak() VERA_NOEXCEPT
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
	template <class Obj>
	friend class obj;
	template <class Obj>
	friend class const_ref;
public:
	VERA_INLINE weak_ref() VERA_NOEXCEPT = default;
	
	VERA_INLINE weak_ref(std::nullptr_t) VERA_NOEXCEPT :
		m_node() {}

	VERA_INLINE weak_ref(obj<Object>& rhs) VERA_NOEXCEPT
	{
		auto* obj_ptr = rhs.get();

		m_node.next = std::exchange(obj_ptr->m_weak_chain.next, &m_node);
		m_node.prev = &obj_ptr->m_weak_chain;
		m_node.ptr  = obj_ptr;
	}

	VERA_INLINE weak_ref(const weak_ref& rhs) VERA_NOEXCEPT
	{
		rhs.m_node.link(m_node);
	}

	VERA_INLINE weak_ref(weak_ref&& rhs)  VERA_NOEXCEPT
	{
		rhs.m_node.exchange(m_node);
	}

	VERA_INLINE ~weak_ref() VERA_NOEXCEPT
	{
		m_node.invalidate();
	}

	VERA_INLINE weak_ref& operator=(const weak_ref& rhs) VERA_NOEXCEPT
	{
		if (std::addressof(rhs) == this) return *this;

		m_node.invalidate();
		rhs.m_node.link(m_node);

		return *this;
	}

	VERA_INLINE weak_ref& operator=(weak_ref&& rhs)  VERA_NOEXCEPT
	{
		if (std::addressof(rhs) == this) return *this;
	
		m_node.invalidate();
		rhs.m_node.exchange(m_node);

		return *this;
	}

	VERA_NODISCARD VERA_INLINE const Object* get() const VERA_NOEXCEPT
	{
		return m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE Object* get() VERA_NOEXCEPT
	{
		return m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object& unwrap() const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE Object& unwrap() VERA_NOEXCEPT
	{
		return *reinterpret_cast<Object*>(m_node.ptr);
	}

	VERA_INLINE void reset() VERA_NOEXCEPT
	{
		m_node.invalidate();
	}

	VERA_NODISCARD VERA_INLINE Object* operator*() VERA_NOEXCEPT
	{
		return m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object* operator*() const VERA_NOEXCEPT
	{
		return m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE Object* operator->() VERA_NOEXCEPT
	{
		return m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object* operator->() const VERA_NOEXCEPT
	{
		return m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator==(const weak_ref& rhs) const VERA_NOEXCEPT
	{
		return m_node.ptr == rhs.m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator!=(const weak_ref& rhs) const VERA_NOEXCEPT
	{
		return m_node.ptr != rhs.m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE operator bool() const VERA_NOEXCEPT
	{
		return static_cast<bool>(m_node.ptr);
	}

private:
	priv::WeakNode m_node;
};

template <class Object>
class const_ref
{
public:
	VERA_INLINE	explicit const_ref() VERA_NOEXCEPT :
		m_ptr(nullptr) {}
	
	VERA_INLINE explicit const_ref(std::nullptr_t) VERA_NOEXCEPT :
		m_ptr(nullptr) {}

	VERA_INLINE	explicit const_ref(const Object* ptr) VERA_NOEXCEPT :
		m_ptr(ptr) {}

	VERA_INLINE	explicit const_ref(const obj<Object> obj) VERA_NOEXCEPT :
		m_ptr(obj.m_ptr) {}

	VERA_INLINE explicit const_ref(const weak_ref<Object> wref) VERA_NOEXCEPT :
		m_ptr(wref.m_ptr) {}

	VERA_NODISCARD VERA_INLINE const Object* get() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object& unwrap() const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const Object*>(m_ptr);
	}

	void reset() VERA_NOEXCEPT
	{
		m_ptr = nullptr;
	}

	VERA_NODISCARD VERA_INLINE const Object* operator*() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object* operator->() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator==(const const_ref& rhs) const VERA_NOEXCEPT
	{
		return m_ptr == rhs.m_ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator!=(const const_ref& rhs) const VERA_NOEXCEPT
	{
		return m_ptr != rhs.m_ptr;
	}

	VERA_NODISCARD VERA_INLINE operator bool() const VERA_NOEXCEPT
	{
		return static_cast<bool>(m_ptr);
	}

protected:
	const Object* m_ptr;
};

template <class Object>
class ref : public const_ref<Object>
{
	using my_base = const_ref<Object>;
public:
	VERA_INLINE ref() VERA_NOEXCEPT :
		my_base() {}

	VERA_INLINE	ref(std::nullptr_t) VERA_NOEXCEPT :
		my_base() {}

	VERA_INLINE	ref(Object* ptr) VERA_NOEXCEPT :
		my_base(ptr) {}

	VERA_INLINE	ref(obj<Object> obj) VERA_NOEXCEPT :
		my_base(obj.get()) {}

	VERA_INLINE	ref(weak_ref<Object> wref) VERA_NOEXCEPT :
		my_base(wref.get()) {}

	VERA_NODISCARD VERA_INLINE Object* get() VERA_NOEXCEPT
	{
		return const_cast<Object*>(this->m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object& unwrap() VERA_NOEXCEPT
	{
		return *const_cast<Object*>(this->m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* operator*() VERA_NOEXCEPT
	{
		return const_cast<Object*>(this->m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* operator->() VERA_NOEXCEPT
	{
		return const_cast<Object*>(this->m_ptr);
	}
};

VERA_NAMESPACE_END