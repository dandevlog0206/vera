#pragma once

#include "coredefs.h"
#include <exception>
#include <format>

#define VERA_CHECK(expression) \
	(static_cast<bool>(expression)) ? (void)(0) : \
	vr::priv::__check_impl(#expression, nullptr, __FILE__, __LINE__)

#define VERA_CHECK_MSG(expression, msg) \
	(static_cast<bool>(expression)) ? (void)(0) : \
	vr::priv::__check_impl(#expression, msg, __FILE__, __LINE__)

VERA_NAMESPACE_BEGIN

class Exception : public std::exception
{
public:
	VERA_INLINE Exception() {}

	VERA_INLINE Exception(std::string_view msg) :
		m_msg(msg) {}

	template <class... Args>
	VERA_INLINE Exception(const std::format_string<Args...> fmt, Args&&... params) :
		m_msg(std::format(fmt, std::forward<Args>(params)...)) {}

	const char* what() const override
	{
		return m_msg.c_str();
	}

protected:
	std::string m_msg;
};

VERA_PRIV_NAMESPACE_BEGIN

extern void __check_impl(
	const char* expression,
	const char* message,
	const char* file,
	uint32_t    line);

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END