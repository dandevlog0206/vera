#include "../include/vera/core/logger.h"

#include "../include/vera/core/exception.h"
#include <iostream>
#include <sstream>

VERA_NAMESPACE_BEGIN

static std::ostream* g_stream               = &std::cerr;
static bool          g_enable_color         = true;
static uint16_t      g_message_colors[8][2] = {
	{ 30 + Logger::BrightWhite, 40 + Logger::Black }, // trace
	{ 30 + Logger::Green,       40 + Logger::Black }, // debug
	{ 30 + Logger::Green,       40 + Logger::Black }, // info
	{ 30 + Logger::Green,       40 + Logger::Black }, // verbose
	{ 30 + Logger::Yellow,      40 + Logger::Black }, // warning
	{ 30 + Logger::BrightRed,   40 + Logger::Black }, // error
	{ 30 + Logger::BrightRed,   40 + Logger::Black }, // exception
	{ 30 + Logger::Red,         40 + Logger::Black }  // assert
};

static void print_log_type(Logger::LogType type)
{
	std::ostream& os = *g_stream;

	if (!g_enable_color) {
		switch (type) {
		case Logger::Trace:     os << "[Trace]";     break;
		case Logger::Debug:     os << "[Debug]";     break;
		case Logger::Info:      os << "[Info]";      break;
		case Logger::Verbose:   os << "[Verbose]";   break;
		case Logger::Warning:   os << "[Warning]";   break;
		case Logger::Error:     os << "[Error]";     break;
		case Logger::Exception: os << "[Exception]"; break;
		case Logger::Assert:    os << "[Assert]";    break;
		}
		return;
	}

	std::stringstream ss;

	switch (type) {
	case Logger::Trace:
		ss << "\x1b[" << g_message_colors[0][0] << ";" << g_message_colors[0][1] << "m[Trace]\x1b[0m";
		break;
	case Logger::Debug:
		ss << "\x1b[" << g_message_colors[1][0] << ";" << g_message_colors[1][1] << "m[Debug]\x1b[0m";
		break;
	case Logger::Info:
		ss << "\x1b[" << g_message_colors[2][0] << ";" << g_message_colors[2][1] << "m[Info]\x1b[0m";
		break;
	case Logger::Verbose:
		ss << "\x1b[" << g_message_colors[3][0] << ";" << g_message_colors[3][1] << "m[Verbose]\x1b[0m";
		break;
	case Logger::Warning:
		ss << "\x1b[" << g_message_colors[4][0] << ";" << g_message_colors[4][1] << "m[Warning]\x1b[0m";
		break;
	case Logger::Error:
		ss << "\x1b[" << g_message_colors[5][0] << ";" << g_message_colors[5][1] << "m[Error]\x1b[0m";
		break;
	case Logger::Exception:
		ss << "\x1b[" << g_message_colors[6][0] << ";" << g_message_colors[6][1] << "m[Exception]\x1b[0m";
		break;
	case Logger::Assert:
		ss << "\x1b[" << g_message_colors[7][0] << ";" << g_message_colors[7][1] << "m[Assert]\x1b[0m";
		break;
	}

	os << ss.str();
}

void Logger::log(LogType type, std::string_view msg)
{
	print_log_type(type);
	*g_stream << msg << std::endl;
}

void Logger::trace(std::string_view msg)
{
	log(Trace, msg);
}

void Logger::debug(std::string_view msg)
{
	log(Debug, msg);
}

void Logger::info(std::string_view msg)
{
	log(Info, msg);
}

void Logger::verbose(std::string_view msg)
{
	log(Verbose, msg);
}

void Logger::warn(std::string_view msg)
{
	log(Warning, msg);
}

void Logger::error(std::string_view msg)
{
	log(Error, msg);
}

void Logger::exception(std::string_view msg)
{
	log(Exception, msg);
}

void Logger::assertion(std::string_view msg)
{
	log(Assert, msg);
}

void Logger::setStream(std::ostream& os)
{
	g_stream = std::addressof(os);
}

void Logger::enableColor(bool value)
{
	g_enable_color = value;
}

void Logger::setColor(LogType type, Color fg_color, Color bg_color)
{
	g_message_colors[type][0] = fg_color + 30;
	g_message_colors[type][1] = bg_color + 40;
}

VERA_NAMESPACE_END