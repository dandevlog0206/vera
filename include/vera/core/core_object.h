#pragma once

#include "intrusive_ptr.h"
#include "enum_types.h"
#include "../util/hash.h"
#include <type_traits>
#include <utility>
#include <memory>
#include <atomic>

#define VERA_CORE_OBJECT_INIT(obj_name)                                            \
	using impl_type = class obj_name ## Impl;                                      \
	using pair_type = ::vr::priv::compressed_pair<obj_name, obj_name ## Impl>;     \
	using CoreObject::getImpl;                                                     \
	friend class CoreObject;                                                       \
	friend class obj<obj_name>;                                                    \
	friend pair_type;                                                              \
	VERA_NODISCARD CoreObjectType getCoreObjectType() const VERA_NOEXCEPT override \
	{                                                                              \
		return CoreObjectType::obj_name;                                           \
	}                                                                              \
	obj_name() = default;

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

template <class Type1, class Type2>
struct compressed_pair : public Type1
{
	compressed_pair() = default;

	Type2 second;
};

VERA_PRIV_NAMESPACE_END

class CoreObject : public ManagedObject
{
	VERA_NOCOPY(CoreObject)
	VERA_NOMOVE(CoreObject)
public:
	VERA_NODISCARD virtual CoreObjectType getCoreObjectType() const VERA_NOEXCEPT
	{
		return CoreObjectType::Unknown;
	}

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
	VERA_NODISCARD VERA_INLINE static typename Object::impl_type& getImpl(wref<Object>& obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static const typename Object::impl_type& getImpl(const wref<Object>& obj) VERA_NOEXCEPT
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
	VERA_NODISCARD VERA_INLINE static const typename Object::impl_type& getImpl(cref<Object> obj) VERA_NOEXCEPT
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
	VERA_INLINE CoreObject() = default;

	template <class Object>
	VERA_INLINE static obj<Object> createNewCoreObject()
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		using pair_type = typename Object::pair_type;
		using impl_type = typename Object::impl_type;

		size_t pair_size = sizeof(pair_type);
		auto*  raw_mem   = reinterpret_cast<std::byte*>(operator new(pair_size));

		::new (raw_mem) Object;
		::new (raw_mem + offsetof(pair_type, second)) impl_type;

		return obj<Object>(reinterpret_cast<Object*>(raw_mem));
	}

	template <class Object>
	VERA_INLINE static void destroyObjectImpl(Object* ptr) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		std::destroy_at(&static_cast<typename Object::pair_type*>(ptr)->second);
	}
};

VERA_NAMESPACE_END