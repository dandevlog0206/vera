#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class SemaphoreImpl
{
public:
	obj<Device>   device        = {};

	vk::Semaphore semaphore     = {};
};

class TimelineSemaphoreImpl
{
public:
	obj<Device>   device        = {};

	vk::Semaphore semaphore     = {};
};

VERA_NAMESPACE_END