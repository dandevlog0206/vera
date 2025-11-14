#pragma once

#include "object_impl.h"
#include "../../include/vera/util/version.h"

VERA_NAMESPACE_BEGIN

class ContextImpl
{
public:
	vk::Instance               instance                 = {};

	std::string                applicationName          = {};
	Version                    applicationVersion       = {};

	vk::DebugUtilsMessengerEXT debugUtilsMessenger      = {};
	uint32_t                   debugUtilsMessengerLevel = {};
	bool                       enableDeviceFault        = {};
};

VERA_NAMESPACE_END