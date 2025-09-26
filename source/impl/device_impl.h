#pragma once

#include "object_impl.h"

#include "../../include/vera/core/device.h"
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

	ref<Context>                        context;

	vk::PhysicalDeviceProperties        physicalDeviceProperties;
	vk::PhysicalDeviceMemoryProperties  deviceMemoryProperties;
	vk::PhysicalDevice                  physicalDevice;
	vk::Device                          device;
	vk::Queue                           graphicsQueue;
	vk::Queue                           computeQueue;
	vk::Queue                           transferQueue;
	vk::PipelineCache                   pipelineCache;
	vk::SampleCountFlagBits             sampleCount;
	
	Format                              colorFormat;
	Format                              depthFormat;
	int32_t                             graphicsQueueFamilyIndex;
	int32_t                             transferQueueFamilyIndex;
	int32_t                             computeQueueFamilyIndex;
	std::string                         pipelineCacheFilePath;
	std::vector<DeviceMemoryType>       memoryTypes;

	bool                                extDescriptorIndexing;

	pipeline_map_type                   pipelineMap;
	shader_map_type                     shaderMap;
	sampler_map_type                    samplerMap;
	pipeline_layout_map_type            pipelineLayoutMap;
	resource_layout_map_type            resourceLayoutMap;

	ref<Sampler>                        defaultSampler;
};

static uint32_t get_memory_type_index(DeviceImpl& device_impl, uint32_t type_bits, vk::MemoryPropertyFlags flags)
{
	auto& props = device_impl.deviceMemoryProperties;

	for (uint32_t i = 0; i < props.memoryTypeCount; ++i)
		if (type_bits & (1 << i) && (props.memoryTypes[i].propertyFlags & flags) == flags)
			return i;

	throw Exception("cannot find adequate memory type index");
}

static vk::MemoryHeapFlags to_vk_memory_heap_flags(MemoryHeapFlags flags)
{
	vk::MemoryHeapFlags result;

	if (flags & MemoryHeapFlagBits::DeviceLocal)
		result |= vk::MemoryHeapFlagBits::eDeviceLocal;
	if (flags & MemoryHeapFlagBits::MultiInstance)
		result |= vk::MemoryHeapFlagBits::eMultiInstance;

	return result;
}

static vk::MemoryPropertyFlags to_vk_memory_property_flags(MemoryPropertyFlags flags)
{
	vk::MemoryPropertyFlags result;

	if (flags & MemoryPropertyFlagBits::DeviceLocal)
		result |= vk::MemoryPropertyFlagBits::eDeviceLocal;
	if (flags & MemoryPropertyFlagBits::HostVisible)
		result |= vk::MemoryPropertyFlagBits::eHostVisible;
	if (flags & MemoryPropertyFlagBits::HostCoherent)
		result |= vk::MemoryPropertyFlagBits::eHostCoherent;
	if (flags & MemoryPropertyFlagBits::HostCached)
		result |= vk::MemoryPropertyFlagBits::eHostCached;
	if (flags & MemoryPropertyFlagBits::LazilyAllocated)
		result |= vk::MemoryPropertyFlagBits::eLazilyAllocated;
	if (flags & MemoryPropertyFlagBits::Protected)
		result |= vk::MemoryPropertyFlagBits::eProtected;

	return result;
}

VERA_NAMESPACE_END