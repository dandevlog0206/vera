#pragma once

#include "coredefs.h"
#include <string_view>

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

	static void setStream(std::ostream& os);
	static void enableColor(bool value);
	static void setColor(LogType type, Color fg_color, Color bg_color);
};

VERA_NAMESPACE_END