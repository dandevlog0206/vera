#pragma once

#include "logger.h"
#include <sstream>

#ifdef _DEBUG
#	define VERA_ASSERT(expression) \
		(static_cast<bool>(expression)) ? (void)(0) : \
		vr::priv::__assert_impl(#expression, nullptr, __FILE__, __LINE__)

#	define VERA_ASSERT_MSG(expression, msg) \
		(static_cast<bool>(expression)) ? (void)(0) : \
		vr::priv::__assert_impl(#expression, msg, __FILE__, __LINE__)
#else
#	define VERA_ASSERT(expression) (void)(0)
#	define VERA_ASSERT_MSG(expression, msg) (void)(0)
#endif

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

static void __assert_impl(
	const char* expression,
	const char* message,
	const char* file,
	uint32_t    line
) {
	std::stringstream ss;
	ss << "Assertion failed: " << expression << ", ";
	if (message)
		ss << "message: " << message << ", ";
	ss << "file: " << file << ", ";
	ss << "line: " << line;

	Logger::assertion(ss.str());

#	ifdef _MSC_VER
	__debugbreak();
#	endif
}

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END