#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

struct SemaphoreImpl
{
	using object_type = class Semaphore;

	ref<Device>   device;

	vk::Semaphore semaphore;
};

VERA_NAMESPACE_END