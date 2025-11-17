#pragma once

#include "object_impl.h"

#include "../../include/vera/core/device_memory.h"

VERA_NAMESPACE_BEGIN

enum class MemoryResourceType VERA_ENUM
{
	Buffer,
	Texture
};

struct MemoryResourceBind
{
	MemoryResourceType resourceType;
	size_t             size;
	size_t             offset;
	void*              resourcePtr;
};

class DeviceMemoryImpl
{
public:
	obj<Device>                     device        = {};

	vk::DeviceMemory                vkMemory        = {};

	MemoryPropertyFlags             propertyFlags = {};
	std::vector<MemoryResourceBind> resourceBind  = {};
	size_t                          allocated     = {};
	uint32_t                        typeIndex     = {};
	void*                           mapPtr        = {};
};

VERA_NAMESPACE_END