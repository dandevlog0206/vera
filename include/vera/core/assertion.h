#pragma once

#include "coredefs.h"
#include <cstdint>

#ifdef _DEBUG
#	define VERA_ASSERT(expression) \
		(static_cast<bool>(expression)) ? (void)(0) : \
		vr::priv::__assert_impl(#expression, nullptr, __FILE__, __LINE__)

#	define VERA_ASSERT_MSG(expression, msg) \
		(static_cast<bool>(expression)) ? (void)(0) : \
		vr::priv::__assert_impl(#expression, msg, __FILE__, __LINE__)

#   define VERA_ERROR_MSG(msg) \
		vr::priv::__error_impl(msg, __FILE__, __LINE__)
#else
#	define VERA_ASSERT(expression) (void)(0)
#	define VERA_ASSERT_MSG(expression, msg) (void)(0)
#   define VERA_ERROR_MSG(msg) (void)(0)
#endif

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

// implemented in source/core/exception.cpp
VERA_NORETURN extern void __assert_impl(
	const char* expression,
	const char* message,
	const char* file,
	uint32_t    line);

VERA_NORETURN extern void __error_impl(
	const char* message,
	const char* file,
	uint32_t    line);

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END