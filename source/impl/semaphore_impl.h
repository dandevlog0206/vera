#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class SemaphoreImpl
{
public:
	obj<Device>   device      = {};

	vk::Semaphore vkSemaphore = {};
};

class TimelineSemaphoreImpl
{
public:
	obj<Device>   device      = {};

	vk::Semaphore vkSemaphore = {};
};

VERA_NAMESPACE_END