#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class Device;

struct FenceImpl
{
	obj<Device> device;

	vk::Fence   fence;
};

VERA_NAMESPACE_END