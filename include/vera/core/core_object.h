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
	friend class CoreObject;                                                       \
	friend class obj<obj_name>;                                                    \
	friend pair_type;                                                              \
	template <class Target, class T>                                               \
	friend VERA_CONSTEXPR obj<Target> obj_cast(obj<T> source) VERA_NOEXCEPT;       \
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
	VERA_NODISCARD VERA_INLINE static typename Object::impl_type& getImpl(weak_obj<Object>& obj) VERA_NOEXCEPT
	{
		static_assert(std::is_base_of_v<CoreObject, Object>);

		return static_cast<typename Object::pair_type*>(obj.get())->second;
	}

	template <class Object>
	VERA_NODISCARD VERA_INLINE static const typename Object::impl_type& getImpl(const weak_obj<Object>& obj) VERA_NOEXCEPT
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
	VERA_INLINE CoreObject() = default;

	template <class Object>
	VERA_INLINE static obj<Object> createNewCoreObject()
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
};

VERA_NAMESPACE_END