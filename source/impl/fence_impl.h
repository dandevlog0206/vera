#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class FenceImpl
{
public:
	obj<Device> device = {};

	vk::Fence   vkFence  = {};
};

VERA_NAMESPACE_END