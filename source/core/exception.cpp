#include "../../include/vera/core/exception.h"

#include "../../include/vera/core/logger.h"
#include <sstream>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

void __check_impl(
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

VERA_NORETURN void __assert_impl(
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

	exit(1);
}

VERA_NORETURN void __error_impl(
	const char* message,
	const char* file,
	uint32_t    line
) {
	std::stringstream ss;
	ss << "error message: " << message << ", ";
	ss << "file: " << file << ", ";
	ss << "line: " << line;

	Logger::error(ss.str());

#	ifdef _MSC_VER
	__debugbreak();
#	endif

	exit(1);
}

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END