#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class SemaphoreImpl
{
public:
	obj<Device>       device        = {};

	vk::Semaphore     semaphore     = {};
	vk::SemaphoreType semaphoreType = {};
};

VERA_NAMESPACE_END