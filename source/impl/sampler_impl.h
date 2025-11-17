#pragma once

#include "object_impl.h"

VERA_NAMESPACE_BEGIN

class SamplerImpl
{
public:
	obj<Device>       device    = {};

	vk::Sampler       vkSampler = {};

	size_t            hashValue = {};
	SamplerCreateInfo info      = {};
};

VERA_NAMESPACE_END