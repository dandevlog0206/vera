#pragma once

#include "core_object.h"
#include "format.h"
#include <vector>
#include <string_view>

VERA_NAMESPACE_BEGIN

class Context;

struct DeviceCreateInfo
{
	std::vector<std::string_view> deviceLayers          = {};
	std::vector<std::string_view> deviceExtensions      = {};

	uint32_t                      deviceID              = 0;

	Format                        colorFormat           = Format::RGBA8Unorm;
	Format                        depthFormat           = Format::D32Float;

	bool                          enablePipelineCache   = true;
	std::string_view              pipelineCacheFilePath = {};
};

class Device : protected CoreObject
{
	VERA_CORE_OBJECT(Device)
public:
	static ref<Device> create(ref<Context> context, const DeviceCreateInfo& info);
	~Device();

	void waitIdle() const;
};

VERA_NAMESPACE_END