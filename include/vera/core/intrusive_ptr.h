#pragma once

#include "assertion.h"
#include <atomic>
#include <utility>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

typedef class WeakNode* WeakNodePtr;
typedef class WeakNodeLink* WeakNodeLinkPtr;

class WeakNodeLink
{
public:
	mutable WeakNodePtr next;
};

class WeakNode : public WeakNodeLink
{
public:
	VERA_INLINE void link(WeakNode& node) const VERA_NOEXCEPT
	{
		if (!ptr) return;

		VERA_ASSERT_MSG(!node.ptr && !node.prev && !node.next, "node must be empty");

		if (next) {
			next->prev = &node;
			node.next  = next;
		}

		next      = &node;
		node.prev = const_cast<WeakNode*>(this);
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

	mutable WeakNodeLinkPtr prev;
	void*                   ptr;
};

VERA_PRIV_NAMESPACE_END

/*
obj<Object>:
	Acts as a reference-counted smart pointer (similar to std::shared_ptr). It automatically increases the reference
	count when copied and decreases it when destroyed. When the count reaches zero, the object is deleted and all
	associated weak references are invalidated.

weak_obj<Object>:
	Provides a non-owning reference to an object managed by obj<>. It does not affect the reference count. If the
	object is destroyed, the weak reference is automatically invalidated (set to null), preventing dangling pointers.

ref<Object>:
	A lightweight, non-owning pointer wrapper. It can be constructed from obj<> or weak_obj<>, but does not manage
	lifetime or reference counting. It simply provides convenient access to the underlying object.
*/

template <class Object>
class obj;
template <class Object>
class weak_obj;
template <class Object>
class const_ref;
template <class Object>
class ref;

class ManagedObject abstract
{
	template <class Object>
	friend class obj;
	template <class Object>
	friend class weak_obj;
	template <class Object>
	friend class const_ref;
	template <class Object>
	friend class ref;

protected:
	VERA_INLINE ManagedObject() VERA_NOEXCEPT :
		m_atomic(0),
		m_weak_chain(nullptr) {}

	virtual VERA_INLINE ~ManagedObject() VERA_NOEXCEPT {}

private:
	std::atomic<uint64_t> m_atomic;
	priv::WeakNodeLink    m_weak_chain;
};

template <class Object>
class obj
{
	template <class Target, class T>
	friend VERA_CONSTEXPR obj<Target> obj_cast(obj<T>) VERA_NOEXCEPT;
public:
	VERA_INLINE obj() VERA_NOEXCEPT :
		m_ptr(nullptr) {}
	
	VERA_INLINE obj(std::nullptr_t) VERA_NOEXCEPT :
		m_ptr(nullptr) {}

	VERA_INLINE obj(Object* ptr) VERA_NOEXCEPT :
		m_ptr(static_cast<ManagedObject*>(ptr))
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
		return static_cast<const Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* get() VERA_NOEXCEPT
	{
		return static_cast<Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object& unwrap() const VERA_NOEXCEPT
	{
		return *static_cast<const Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object& unwrap() VERA_NOEXCEPT
	{
		return *static_cast<Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* operator*() VERA_NOEXCEPT
	{
		return static_cast<Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object* operator*() const VERA_NOEXCEPT
	{
		return static_cast<const Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* operator->() VERA_NOEXCEPT
	{
		return static_cast<Object*>(m_ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object* operator->() const VERA_NOEXCEPT
	{
		return static_cast<const Object*>(m_ptr);
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

	VERA_NODISCARD VERA_INLINE const_ref<Object> cref() const VERA_NOEXCEPT
	{
		return const_ref<Object>(get());
	}

	VERA_NODISCARD VERA_INLINE ref<Object> ref() VERA_NOEXCEPT
	{
		return ::vr::ref<Object>(get());
	}

	VERA_NODISCARD VERA_INLINE operator const_ref<Object>() const VERA_NOEXCEPT
	{
		return cref();
	}

	VERA_NODISCARD VERA_INLINE operator ::vr::ref<Object>() VERA_NOEXCEPT
	{
		return ref();
	}

	VERA_INLINE void reset() VERA_NOEXCEPT
	{
		decrease();
	}

private:
	VERA_INLINE void increase() VERA_NOEXCEPT
	{
		if (m_ptr)
			m_ptr->m_atomic.fetch_add(1, std::memory_order_relaxed);
	}

	VERA_INLINE void decrease() VERA_NOEXCEPT
	{
		if (m_ptr && m_ptr->m_atomic.fetch_sub(1, std::memory_order_acq_rel) == 1) {
			invalidate_all_weak();
			delete m_ptr;
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
	ManagedObject* m_ptr;
};

template <class Object>
class weak_obj
{
	template <class Target, class T>
	friend VERA_CONSTEXPR weak_obj<Target> weak_cast(weak_obj<T>) VERA_NOEXCEPT;
public:
	VERA_INLINE weak_obj() VERA_NOEXCEPT = default;
	
	VERA_INLINE weak_obj(std::nullptr_t) VERA_NOEXCEPT :
		m_node() {}

	VERA_INLINE weak_obj(obj<Object>& rhs) VERA_NOEXCEPT
	{
		auto* obj_ptr = rhs.get();

		m_node.next = std::exchange(obj_ptr->m_weak_chain.next, &m_node);
		m_node.prev = &obj_ptr->m_weak_chain;
		m_node.ptr  = obj_ptr;
	}

	VERA_INLINE weak_obj(const weak_obj& rhs) VERA_NOEXCEPT
	{
		rhs.m_node.link(m_node);
	}

	VERA_INLINE weak_obj(weak_obj&& rhs)  VERA_NOEXCEPT
	{
		rhs.m_node.exchange(m_node);
	}

	VERA_INLINE ~weak_obj() VERA_NOEXCEPT
	{
		m_node.invalidate();
	}

	VERA_INLINE weak_obj& operator=(const weak_obj& rhs) VERA_NOEXCEPT
	{
		if (std::addressof(rhs) == this) return *this;

		m_node.invalidate();
		rhs.m_node.link(m_node);

		return *this;
	}

	VERA_INLINE weak_obj& operator=(weak_obj&& rhs)  VERA_NOEXCEPT
	{
		if (std::addressof(rhs) == this) return *this;
	
		m_node.invalidate();
		rhs.m_node.exchange(m_node);

		return *this;
	}

	VERA_NODISCARD VERA_INLINE const Object* get() const VERA_NOEXCEPT
	{
		return reinterpret_cast<const Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* get() VERA_NOEXCEPT
	{
		return reinterpret_cast<Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object& unwrap() const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE Object& unwrap() VERA_NOEXCEPT
	{
		return *reinterpret_cast<Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* operator*() VERA_NOEXCEPT
	{
		return reinterpret_cast<Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object* operator*() const VERA_NOEXCEPT
	{
		return reinterpret_cast<const Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE Object* operator->() VERA_NOEXCEPT
	{
		return reinterpret_cast<Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object* operator->() const VERA_NOEXCEPT
	{
		return reinterpret_cast<const Object*>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE bool operator==(const weak_obj& rhs) const VERA_NOEXCEPT
	{
		return m_node.ptr == rhs.m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator!=(const weak_obj& rhs) const VERA_NOEXCEPT
	{
		return m_node.ptr != rhs.m_node.ptr;
	}

	VERA_NODISCARD VERA_INLINE operator bool() const VERA_NOEXCEPT
	{
		return static_cast<bool>(m_node.ptr);
	}

	VERA_NODISCARD VERA_INLINE operator obj<Object>() const VERA_NOEXCEPT
	{
		return obj<Object>(reinterpret_cast<Object*>(m_node.ptr));
	}

	VERA_NODISCARD VERA_INLINE operator const_ref<Object>() const VERA_NOEXCEPT
	{
		return const_ref<Object>(reinterpret_cast<const Object*>(m_node.ptr));
	}

	VERA_NODISCARD VERA_INLINE operator ref<Object>() VERA_NOEXCEPT
	{
		return ref<Object>(reinterpret_cast<Object*>(m_node.ptr));
	}

	VERA_INLINE void reset() VERA_NOEXCEPT
	{
		m_node.invalidate();
	}

private:
	priv::WeakNode m_node;
};

template <class Object>
class const_ref
{
public:
	VERA_INLINE	const_ref() VERA_NOEXCEPT :
		m_ptr(nullptr) {}
	
	VERA_INLINE const_ref(std::nullptr_t) VERA_NOEXCEPT :
		m_ptr(nullptr) {}

	VERA_INLINE	const_ref(const Object* ptr) VERA_NOEXCEPT :
		m_ptr(ptr) {}

	template <class T>
	VERA_INLINE const_ref(ref<T> rhs) VERA_NOEXCEPT :
		m_ptr(static_cast<const T*>(rhs.m_ptr)) {}

	template <class T>
	VERA_INLINE const_ref(const_ref<T> rhs) VERA_NOEXCEPT :
		m_ptr(static_cast<const T*>(rhs.m_ptr)) {}

	VERA_INLINE	const_ref(const const_ref& rhs) VERA_NOEXCEPT :
		m_ptr(rhs.m_ptr) {}

	VERA_INLINE	const_ref(const_ref&& rhs) VERA_NOEXCEPT :
		m_ptr(std::exchange(rhs.m_ptr, nullptr)) {}

	VERA_INLINE const_ref& operator=(const const_ref& rhs) VERA_NOEXCEPT
	{
		this->m_ptr = rhs.m_ptr;

		return *this;
	}

	VERA_INLINE const_ref& operator=(const_ref&& rhs) VERA_NOEXCEPT
	{
		this->m_ptr = std::exchange(rhs.m_ptr, nullptr);

		return *this;
	}

	VERA_NODISCARD VERA_INLINE const Object* get() const VERA_NOEXCEPT
	{
		return m_ptr;
	}

	VERA_NODISCARD VERA_INLINE const Object& unwrap() const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const Object*>(m_ptr);
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

	void reset() VERA_NOEXCEPT
	{
		m_ptr = nullptr;
	}

protected:
	const Object* m_ptr;
};

template <class Object>
class ref : public const_ref<Object>
{
	template <class Obj>
	friend class ref;

	using my_base = const_ref<Object>;
public:
	VERA_INLINE ref() VERA_NOEXCEPT :
		my_base() {}

	VERA_INLINE	ref(std::nullptr_t) VERA_NOEXCEPT :
		my_base() {}

	VERA_INLINE	ref(Object* ptr) VERA_NOEXCEPT :
		my_base(ptr) {}

	template <class T>
	VERA_INLINE ref(ref<T> rhs) VERA_NOEXCEPT :
		my_base(static_cast<const Object*>(rhs.m_ptr)) {}

	VERA_INLINE ref(const ref& rhs) VERA_NOEXCEPT :
		my_base(rhs.m_ptr) {}

	VERA_INLINE ref(ref&& rhs) VERA_NOEXCEPT :
		my_base(std::exchange(rhs.m_ptr, nullptr)) {}

	VERA_INLINE ref& operator=(const ref& rhs) VERA_NOEXCEPT
	{
		this->m_ptr = rhs.m_ptr;

		return *this;
	}

	VERA_INLINE ref& operator=(ref&& rhs) VERA_NOEXCEPT
	{
		this->m_ptr = std::exchange(rhs.m_ptr, nullptr);

		return *this;
	}

	VERA_NODISCARD VERA_INLINE const Object* get() const VERA_NOEXCEPT
	{
		return this->m_ptr;
	}

	VERA_NODISCARD VERA_INLINE Object* get() VERA_NOEXCEPT
	{
		return const_cast<Object*>(this->m_ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object& unwrap() const VERA_NOEXCEPT
	{
		return *this->m_ptr;
	}

	VERA_NODISCARD VERA_INLINE Object& unwrap() VERA_NOEXCEPT
	{
		return *const_cast<Object*>(this->m_ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object* operator*() const VERA_NOEXCEPT
	{
		return this->m_ptr;
	}

	VERA_NODISCARD VERA_INLINE Object* operator*() VERA_NOEXCEPT
	{
		return const_cast<Object*>(this->m_ptr);
	}

	VERA_NODISCARD VERA_INLINE const Object* operator->() const VERA_NOEXCEPT
	{
		return this->m_ptr;
	}

	VERA_NODISCARD VERA_INLINE Object* operator->() VERA_NOEXCEPT
	{
		return const_cast<Object*>(this->m_ptr);
	}
};

template <class T, class... Args>
VERA_NODISCARD VERA_CONSTEXPR obj<T> make_obj(Args&&... params)
{
	static_assert(std::is_base_of_v<ManagedObject, T>);

	return obj<T>(new T(std::forward<Args>(params)...));
}

template <class Target, class T>
VERA_NODISCARD VERA_CONSTEXPR obj<Target> obj_cast(obj<T> source) VERA_NOEXCEPT
{
	static_assert(std::is_base_of_v<ManagedObject, Target> || std::is_base_of_v<ManagedObject, T>);

	obj<Target> result;
	result.m_ptr = static_cast<Target*>(std::exchange(source.m_ptr, nullptr));

	return result;
}

template <class Target, class T>
VERA_NODISCARD VERA_CONSTEXPR weak_obj<Target> weak_cast(weak_obj<T>& source) VERA_NOEXCEPT
{
	static_assert(std::is_base_of_v<ManagedObject, Target> || std::is_base_of_v<ManagedObject, T>);

	if (!source.m_node.ptr) return nullptr;

	weak_obj<Target> result;

	if (source.m_node.next) {
		source.m_node.next->prev = &result.m_node;
		result.m_node.next       = source.m_node.next;
	}

	source.m_node.next = &result.m_node;
	result.m_node.prev = &source.m_node;
	result.m_node.ptr  = static_cast<Target*>(reinterpret_cast<T*>(source.m_node.ptr));

	return result;
}

// Unsafe version of obj_cast, does not perform any type checking.
// DO NOT USE UNLESS YOU ARE ABSOLUTELY SURE THAT THE CAST IS SAFE.
template <class Target, class T>
VERA_NODISCARD VERA_CONSTEXPR obj<Target> unsafe_obj_cast(ref<T> source) VERA_NOEXCEPT
{
	// unsafe cast, object pointed by source might be already destroyed
	return obj<Target>(source.get());
}

VERA_NAMESPACE_END
