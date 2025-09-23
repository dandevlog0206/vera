#pragma once

#include "object_impl.h"

#include <unordered_map>

VERA_NAMESPACE_BEGIN

class Context;
class Pipeline;
class Shader;
class Sampler;
class PipelineLayout;
class ResourceLayout;

struct DeviceImpl
{
	using pipeline_map_type        = std::unordered_map<size_t, ref<Pipeline>>;
	using shader_map_type          = std::unordered_map<size_t, ref<Shader>>;
	using sampler_map_type         = std::unordered_map<size_t, ref<Sampler>>;
	using pipeline_layout_map_type = std::unordered_map<size_t, ref<PipelineLayout>>;
	using resource_layout_map_type = std::unordered_map<size_t, ref<ResourceLayout>>;

	ref<Context>                       context;

	vk::PhysicalDeviceProperties       physicalDeviceProperties;
	vk::PhysicalDeviceMemoryProperties memoryProperties;
	vk::PhysicalDevice                 physicalDevice;
	vk::Device                         device;
	vk::Queue                          graphicsQueue;
	vk::Queue                          computeQueue;
	vk::Queue                          transferQueue;
	vk::PipelineCache                  pipelineCache;
	
	vk::SampleCountFlagBits            sampleCount;
	Format                             colorFormat;
	Format                             depthFormat;
	int32_t                            graphicsQueueFamilyIndex;
	int32_t                            transferQueueFamilyIndex;
	int32_t                            computeQueueFamilyIndex;
	std::string                        pipelineCacheFilePath;

	bool                               ExtDescriptorIndexing;

	pipeline_map_type                  pipelineMap;
	shader_map_type                    shaderMap;
	sampler_map_type                   samplerMap;
	pipeline_layout_map_type           pipelineLayoutMap;
	resource_layout_map_type           resourceLayoutMap;

	ref<Sampler>                       defaultSampler;
};

VERA_NAMESPACE_END