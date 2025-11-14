#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class ContextImpl
{
public:
	vk::Instance               instance                 = {};

	vk::DebugUtilsMessengerEXT debugUtilsMessenger      = {};
	uint32_t                   debugUtilsMessengerLevel = {};
};

VERA_NAMESPACE_END