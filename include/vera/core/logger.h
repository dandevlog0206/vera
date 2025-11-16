#pragma once

#include "coredefs.h"
#include <string_view>
#include <format>

VERA_NAMESPACE_BEGIN

class Logger
{
public:
	enum Color
	{
		Black         = 0,
		Red           = 1,
		Green         = 2,
		Yellow        = 3,
		Blue          = 4,
		Magenta       = 5,
		Cyan          = 6,
		White         = 7,
		Gray          = 60,
		BrightRed     = 61,
		BrightGreen   = 62,
		BrightYellow  = 63,
		BrightBlue    = 64,
		BrightMagenta = 65,
		BrightCyan    = 66,
		BrightWhite   = 67
	};

	enum LogType
	{
		Trace,
		Debug,
		Info,
		Verbose,
		Warning,
		Error,
		Exception,
		Assert
	};

	static void log(LogType type, std::string_view msg);
	
	static void trace(std::string_view msg);
	static void debug(std::string_view msg);
	static void info(std::string_view msg);
	static void verbose(std::string_view msg);
	static void warn(std::string_view msg);
	static void error(std::string_view msg);
	static void exception(std::string_view msg);
	static void assertion(std::string_view msg);

	template <class... Args>
	static _NODISCARD void trace(const std::format_string<Args...> fmt, Args&&... params)
	{
	    auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Trace, str);
	}

	template <class... Args>
	static _NODISCARD void debug(const std::format_string<Args...> fmt, Args&&... params)
	{
		auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Debug, str);
	}

	template <class... Args>
	static _NODISCARD void info(const std::format_string<Args...> fmt, Args&&... params)
	{
		auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Info, str);
	}

	template <class... Args>
	static _NODISCARD void verbose(const std::format_string<Args...> fmt, Args&&... params)
	{
		auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Verbose, str);
	}

	template <class... Args>
	static _NODISCARD void warn(const std::format_string<Args...> fmt, Args&&... params)
	{
		auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Warning, str);
	}

	template <class... Args>
	static _NODISCARD void error(const std::format_string<Args...> fmt, Args&&... params)
	{
		auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Error, str);
	}

	template <class... Args>
	static _NODISCARD void exception(const std::format_string<Args...> fmt, Args&&... params)
	{
		auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Exception, str);
	}

	template <class... Args>
	static _NODISCARD void assertion(const std::format_string<Args...> fmt, Args&&... params)
	{
		auto str = std::vformat(fmt.get(), std::make_format_args(params...));
		log(LogType::Assert, str);
	}

	static void setStream(std::ostream& os);
	static void enableColor(bool value);
	static void setColor(LogType type, Color fg_color, Color bg_color);
};

VERA_NAMESPACE_END