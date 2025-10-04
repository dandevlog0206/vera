#pragma once

#include "logger.h"
#include <exception>
#include <sstream>
#include <format>

#define VERA_CHECK(expression) \
	(static_cast<bool>(expression)) ? (void)(0) : \
	vr::priv::__check_impl(#expression, nullptr, __FILE__, __LINE__)

#define VERA_CHECK_MSG(expression, msg) \
	(static_cast<bool>(expression)) ? (void)(0) : \
	vr::priv::__check_impl(#expression, msg, __FILE__, __LINE__)

#define VERA_ERROR(msg) \
	vr::priv::__error_impl(msg, __FILE__, __LINE__)

VERA_NAMESPACE_BEGIN

class Exception : public std::exception
{
public:
	VERA_INLINE Exception() {}

	template <class... Args>
	VERA_INLINE Exception(const std::format_string<Args...> fmt, Args&&... params) :
		m_msg(std::format(fmt, std::forward<Args>(params)...)) {}

	VERA_INLINE Exception(std::string_view msg) :
		m_msg(msg) {}

	const char* what() const override
	{
		return m_msg.c_str();
	}

protected:
	std::string m_msg;
};

VERA_PRIV_NAMESPACE_BEGIN

static void __check_impl(
	const char* expression,
	const char* message,
	const char* file,
	uint32_t    line
) {
	std::stringstream ss;

	ss << "checking follwing expression failed:" << expression;
	ss << " message: " << message;
	ss << " in file: " << file;
	ss << " line: " << line;

	throw ::vr::Exception(ss.str());
}

static void __error_impl(
	const char* message,
	const char* file,
	uint32_t    line
) {
	std::stringstream ss;

	ss << " an error has been occured. message: " << message;
	ss << " in file: " << file;
	ss << " line: " << line;

	throw ::vr::Exception(message);
}

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END