#pragma once

#include "../core/coredefs.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

template <class ResultEnum>
class ResultMessage
{
public:
	VERA_CONSTEXPR ResultMessage() VERA_NOEXCEPT :
		m_message(nullptr),
		m_result() {}

	VERA_CONSTEXPR ResultMessage(ResultEnum result) VERA_NOEXCEPT :
		m_message(nullptr),
		m_result(result) {}

	VERA_CONSTEXPR ResultMessage(ResultEnum result, const char* message) VERA_NOEXCEPT :
		m_message(message),
		m_result(result) {}

	VERA_NODISCARD VERA_CONSTEXPR std::string_view what() const VERA_NOEXCEPT
	{
		return m_message ? m_message : std::string_view{};
	}

	VERA_NODISCARD VERA_CONSTEXPR ResultEnum result() const VERA_NOEXCEPT
	{
		return m_result;
	}

	VERA_NODISCARD VERA_CONSTEXPR operator ResultEnum() const VERA_NOEXCEPT
	{
		return m_result;
	}

private:
	const char* m_message;
	ResultEnum  m_result;
};

VERA_NAMESPACE_END