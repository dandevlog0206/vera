#pragma once

#include "object_impl.h"

#include "../../include/vera/core/device.h"
#include <unordered_map>
#include <bitset>

VERA_NAMESPACE_BEGIN

class Context;
class Pipeline;
class Shader;
class Sampler;
class PipelineLayout;
class ResourceLayout;

struct DeviceImpl
{
	using PipelineCacheType        = std::unordered_map<hash_t, ref<Pipeline>>;
	using ShaderCacheType          = std::unordered_map<hash_t, ref<Shader>>;
	using SamplerCacheType         = std::unordered_map<hash_t, ref<Sampler>>;
	using PipelineLayoutCacheType  = std::unordered_map<hash_t, ref<PipelineLayout>>;
	using PipelineLayoutCacheType2 = std::unordered_map<hash_t, ref<PipelineLayout>>;
	using ResourceLayoutCacheType  = std::unordered_map<hash_t, ref<ResourceLayout>>;

	using DeviceProperties             = vk::PhysicalDeviceProperties;
	using DeviceMemoryProperties       = vk::PhysicalDeviceMemoryProperties;
	using DescriptorIndexingProperties = vk::PhysicalDeviceDescriptorIndexingProperties;

	using DeviceMemoryTypes = std::vector<DeviceMemoryType>;

	obj<Context>                 context;

	vk::PhysicalDevice           physicalDevice;
	DeviceProperties             deviceProperties;
	DeviceMemoryProperties       deviceMemoryProperties;
	DescriptorIndexingProperties descriptorIndexingProperties;
	vk::Device                   device;
	vk::Queue                    graphicsQueue;
	vk::Queue                    computeQueue;
	vk::Queue                    transferQueue;
	vk::PipelineCache            pipelineCache;
	vk::SampleCountFlagBits      sampleCount;
	
	Format                       colorFormat;
	Format                       depthFormat;
	int32_t                      graphicsQueueFamilyIndex;
	int32_t                      transferQueueFamilyIndex;
	int32_t                      computeQueueFamilyIndex;
	std::string                  pipelineCacheFilePath;
	DeviceMemoryTypes            memoryTypes;

	PipelineCacheType            pipelineCacheMap;
	ShaderCacheType              shaderCacheMap;
	SamplerCacheType             samplerCacheMap;
	PipelineLayoutCacheType      pipelineLayoutCacheMap;         // hash with layout itself
	PipelineLayoutCacheType2     pipelineLayoutCacheMapByShader; // hash with shaders
	ResourceLayoutCacheType      resourceLayoutCacheMap;

	obj<Sampler>                 defaultSampler;
	obj<Texture>                 defaultTexture;
};

static uint32_t find_memory_type_idx(const DeviceImpl& impl, MemoryPropertyFlags flags, std::bitset<32> type_mask)
{
	for (uint32_t i = 0; i < impl.memoryTypes.size(); ++i)
		if (type_mask[i] && impl.memoryTypes[i].propertyFlags.has(flags))
			return i;
	
	VERA_ASSERT_MSG(false, "failed to find memory type index");
	return UINT32_MAX;
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