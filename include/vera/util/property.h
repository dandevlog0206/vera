#pragma once

#include "../core/coredefs.h"
#include <type_traits>
#include <utility>

VERA_PRIV_NAMESPACE_BEGIN

template <class Property, class Type>
class ClassProxy : public Type {
	ClassProxy() = delete;
	ClassProxy(const ClassProxy&) = delete;
	ClassProxy(ClassProxy&&) = delete;

public:
	template <class Init>
	constexpr ClassProxy(Init&& value, Property& parent) :
		Type(std::forward<Init>(value)), m_parent(parent) {}
	
	constexpr ~ClassProxy() { m_parent.set(*this); }

	constexpr auto operator->() { return this; }

private:
	Property& m_parent;
};

template <class Type>
class ClassProxy_readonly : public Type {
	ClassProxy_readonly() = delete;
	ClassProxy_readonly(const ClassProxy_readonly&) = delete;
	ClassProxy_readonly(ClassProxy_readonly&&) = delete;

public:
	template <class Init>
	constexpr ClassProxy_readonly(Init&& value) :
		Type(std::forward<Init>(value)) {}

	constexpr auto operator->() const { return this; }
};

template <class Type>
class Proxy {
	Proxy(const Proxy&) = delete;
	Proxy(Proxy&&) = delete;

public:
	using value_type = Type;

	constexpr Proxy() : m_value() {}
	template <class Type2>
	Proxy(Type2&& rhs) :
		m_value(std::forward<Type2>(rhs)) {}
	
	template <class Type2 = value_type, std::enable_if_t<std::is_class_v<Type2>, int> = 0>
	constexpr auto operator->() {
		return ::priv::ClassProxy<Proxy<Type>, Type>(m_value, *this);
	}

	constexpr Type get() const { return m_value; }
	constexpr Type operator()() const { return m_value; }
	constexpr operator Type() const { return m_value; }

	constexpr void set(const Type& value) { m_value = value; }
	constexpr void set(Type&& value) { m_value = std::move(value); }
	constexpr void operator()(const Type& value) { m_value = value; }
	constexpr void operator()(Type&& value) { m_value = std::move(value); }
	constexpr void operator=(const Type& value) { m_value = value; }
	constexpr void operator=(Type&& value) { m_value = std::move(value); }

	constexpr const Type& cref() const { return m_value; }
	constexpr const Type& ref() const { return m_value; }
	constexpr Type& ref() { return m_value; }

private:
	Type m_value;
};

template <class Type, class Class>
class Proxy_readonly {
	friend Class;
	template <class Type2, class Property>
	friend class ::priv::ClassProxy;

	Proxy_readonly(const Proxy_readonly&) = delete;
	Proxy_readonly(Proxy_readonly&&) = delete;

public:
	using value_type = Type;

	constexpr Proxy_readonly() : m_value() {}
	template <class Type2>
	constexpr Proxy_readonly(Type2&& rhs) :
		m_value(std::forward<Type2>(rhs)) {}

	template <class Type2 = value_type, std::enable_if_t<std::is_class_v<Type2>, int> = 0>
	auto operator->() const {
		return ::priv::ClassProxy_readonly<value_type>(get());
	}

	constexpr Type get() const { return m_value; }
	constexpr Type operator()() const { return m_value; }
	constexpr operator Type() const { return m_value; }

	constexpr const Type& cref() const { return m_value; }
	constexpr const Type& ref() const { return m_value; }
	
private:
	template <class Type2 = value_type, std::enable_if_t<std::is_class_v<Type2>, int> = 0>
	auto operator->() {
		return ::priv::ClassProxy<Proxy_readonly, value_type>(get(), *this);
	}
	
	constexpr void set(const Type& value) { m_value = value; }
	constexpr void set(Type&& value) { m_value = std::move(value); }
	constexpr void operator()(const Type& value) { m_value = value; }
	constexpr void operator()(Type&& value) { m_value = std::move(value); }
	constexpr void operator=(const Type& value) { m_value = value; }
	constexpr void operator=(Type&& value) { m_value = std::move(value); }
	
	constexpr Type& ref() { return m_value; }

private:
	Type m_value;
};

template <class Type, class Class>
class Proxy_writeonly {
	friend Class;

	Proxy_writeonly(const Proxy_writeonly&) = delete;
	Proxy_writeonly(Proxy_writeonly&&) = delete;

public:
	using value_type = Type;

	constexpr Proxy_writeonly() : m_value() {}
	template <class Type2>
	constexpr Proxy_writeonly(Type2&& rhs)
		: m_value(std::forward<Type2>(rhs)) {}

	constexpr void set(const Type& value) { m_value = value; }
	constexpr void set(Type&& value) { m_value = std::move(value); }
	constexpr void operator()(const Type& value) { m_value = value; }
	constexpr void operator()(Type&& value) { m_value = std::move(value); }
	constexpr void operator=(const Type& value) { m_value = value; }
	constexpr void operator=(Type&& value) { m_value = std::move(value); }

private:
	constexpr Type get() const { return m_value; }
	constexpr Type operator()() const { return m_value; }
	constexpr operator Type() const { return m_value; }

	constexpr const Type& cref() const { return m_value; }
	constexpr const Type& ref() const { return m_value; }
	constexpr Type& ref() { return m_value; }

private:
	Type m_value;
};

VERA_PRIV_NAMESPACE_END

#define __N_ARGS_IMPL__(_1, _2, N, ...) N
#define __N_ARGS__(...) __N_ARGS_IMPL__(__VA_ARGS__, 2, 1)

#define __CAT2_IMPL(a,b) a ## b
#define __CAT2(a,b) __CAT2_IMPL(a,b)

#define __CAT3_IMPL(a,b,c) a ## b ## c
#define __CAT3(a,b, c) __CAT3_IMPL(a, b, c)

#define __safe_offsetof(type, member) ((size_t)(std::addressof(((type*)0)->member)))
#define __PROPERTY_GET_ADDRESS_OF(__prop_name) (std::intptr_t)(this) - __safe_offsetof(__this_class_type, __prop_name)

#define __PROPERTY_REQUIRES_GETTER \
	template <class Class = __this_class_type>
// 	template <class Class = __this_class_type, class Property = this_type, std::enable_if_t<priv::has_get<Class, Property>::type::value, int> = 0>
// 
#define __PROPERTY_REQUIRES_SETTER \
	template <class Class = __this_class_type>
// 	template <class Class = __this_class_type, class Property = this_type, std::enable_if_t<priv::has_set<Class, Property>::type::value, int> = 0>

#define __PROPERTY_IMPL_(__prop_type, __prop_name) \
VERA_NO_UNIQUE_ADRESS struct __property_##__prop_name { \
	using value_type = __prop_type; \
	using this_type = __property_##__prop_name; \
	struct type_tag {}; \
	__property_##__prop_name() = default; \
	__property_##__prop_name(const this_type&) = delete; \
	__property_##__prop_name(const value_type& init) { set(init); } \
	template <class Type = value_type, std::enable_if_t<std::is_class_v<Type>, int> = 0> \
	auto operator->() { \
		return ::priv::ClassProxy<decltype(*this), value_type>(get(), *this); \
	} \
	value_type get() const { \
		auto _this = __PROPERTY_GET_ADDRESS_OF(__prop_name); \
		return reinterpret_cast<__this_class_type*>(_this)->__property_get(type_tag{}); \
	} \
	value_type operator()() const { return get(); }\
	operator value_type() const { return get(); }\
	void set(const value_type& value) { \
		auto _this = __PROPERTY_GET_ADDRESS_OF(__prop_name); \
		reinterpret_cast<__this_class_type*>(_this)->__property_set(type_tag{}, value); \
	} \
	void operator()(const value_type& value) { set(value); } \
	void operator=(const value_type& value) { set(value); } \
} __prop_name

#define __PROPERTY_IMPL_readonly(__prop_type, __prop_name) \
VERA_NO_UNIQUE_ADRESS struct __property_##__prop_name { \
	friend __this_class_type; \
	template <class Type2, class Property> \
	friend class ::priv::ClassProxy; \
	using value_type = __prop_type; \
	using this_type = __property_##__prop_name; \
	struct type_tag {}; \
	__property_##__prop_name() = default; \
	__property_##__prop_name(const this_type&) = delete; \
	template <class Type = value_type, std::enable_if_t<std::is_class_v<Type>, int> = 0> \
	auto operator->() const { \
		return ::priv::ClassProxy_readonly<value_type>(get()); \
	} \
	value_type get() const { \
		auto _this = __PROPERTY_GET_ADDRESS_OF(__prop_name); \
		return reinterpret_cast<__this_class_type*>(_this)->__property_get(type_tag{}); \
	} \
	value_type operator()() const { return get(); } \
	operator value_type() const { return get(); } \
private:\
	template <class Type = value_type, std::enable_if_t<std::is_class_v<Type>, int> = 0> \
	auto operator->() { \
		return ::priv::ClassProxy<decltype(*this), value_type>(get(), *this); \
	} \
	__PROPERTY_REQUIRES_SETTER \
	void set(const value_type& value) { \
		auto _this = __PROPERTY_GET_ADDRESS_OF(__prop_name); \
		reinterpret_cast<Class*>(_this)->__property_set(type_tag{}, value); \
	} \
	__PROPERTY_REQUIRES_SETTER \
	void operator()(const value_type& value) { set(value); }\
	__PROPERTY_REQUIRES_SETTER \
	 void operator=(const value_type& value) { set(value); } \
} __prop_name

#define __PROPERTY_IMPL_writeonly(__prop_type, __prop_name) \
VERA_NO_UNIQUE_ADRESS struct __property_##__prop_name { \
	friend __this_class_type; \
	using value_type = __prop_type; \
	using this_type = __property_##__prop_name; \
	struct type_tag {}; \
	__property_##__prop_name() = default; \
	__property_##__prop_name(const this_type&) = delete; \
	void set(const value_type& value) { \
		auto _this = __PROPERTY_GET_ADDRESS_OF(__prop_name); \
		reinterpret_cast<__this_class_type*>(_this)->__property_set(type_tag{}, value); \
	} \
	void operator()(const value_type& value) { set(value); } \
	void operator=(const value_type& value) { set(value); } \
private:\
	template <class Type = value_type, std::enable_if_t<std::is_class_v<Type>, int> = 0> \
	auto operator->() { \
		return ::priv::ClassProxy<decltype(*this), value_type>(get(), *this); \
	} \
	__PROPERTY_REQUIRES_GETTER \
	value_type get() const { \
		auto _this = __PROPERTY_GET_ADDRESS_OF(__prop_name); \
		return reinterpret_cast<Class*>(_this)->__property_get(type_tag{}); \
	} \
	__PROPERTY_REQUIRES_GETTER \
	value_type operator()() const { return get(); } \
	__PROPERTY_REQUIRES_GETTER \
	operator value_type() const { return get(); } \
} __prop_name

#define __PROPERTY_IMPL_default(__prop_type, __prop_name) ::priv::Proxy<__prop_type> __prop_name
#define __PROPERTY_IMPL_defaultreadonly(__prop_type, __prop_name) ::priv::Proxy_readonly<__prop_type, __this_class_type> __prop_name
#define __PROPERTY_IMPL_defaultwriteonly(__prop_type, __prop_name) ::priv::Proxy_writeonly<__prop_type, __this_class_type> __prop_name
#define __PROPERTY_IMPL_readonlydefault(__prop_type, __prop_name) __PROPERTY_IMPL_defaultreadonly(__prop_type, __prop_name)
#define __PROPERTY_IMPL_writeonlydefault(__prop_type, __prop_name) __PROPERTY_IMPL_defaultwriteonly(__prop_type, __prop_name)

#define __CALL_PROPERTY1(type, name, ...) __CAT2(__PROPERTY_IMPL_, __VA_ARGS__)(type, name)
#define __CALL_PROPERTY2(type, name, ...) __CAT3(__PROPERTY_IMPL_, __VA_ARGS__)(type, name)
#define __CALL_PROPERTY(type, name, ...) __CAT2(__CALL_PROPERTY, __N_ARGS__(__VA_ARGS__))(type, name, __VA_ARGS__)

#define PROPERTY_INIT(cls) using __this_class_type = cls
#define PROPERTY(type, __prop_name, ...) __CALL_PROPERTY(type, __prop_name, __VA_ARGS__)

#define PROPERTY_GET_DECL(__prop_name) \
	using __property_has_get_##__prop_name = int; \
	decltype(__prop_name)::value_type __property_get(__property_##__prop_name::type_tag) const

#define PROPERTY_SET_DECL(__prop_name) \
	using __property_has_set_##__prop_name = int; \
	void __property_set(__property_##__prop_name::type_tag, const decltype(__prop_name)::value_type& value)

#define PROPERTY_GET_SET_DECL(__prop_name) \
	using __property_has_get_##__prop_name = int; \
	using __property_has_set_##__prop_name = int; \
	decltype(__prop_name)::value_type __property_get(__property_##__prop_name::type_tag) const; \
	void __property_set(__property_##__prop_name::type_tag, const decltype(__prop_name)::value_type& value)

#define PROPERTY_GET_IMPL(__class_name, __prop_name) \
	decltype(__class_name::__prop_name)::value_type __class_name::__property_get(__property_##__prop_name::type_tag) const

#define PROPERTY_SET_IMPL(__class_name, __prop_name) \
	void __class_name::__property_set(__property_##__prop_name::type_tag, const decltype(__class_name::__prop_name)::value_type& value)
