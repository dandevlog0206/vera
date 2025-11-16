#pragma once

#include "object_impl.h"

#include <unordered_map>
#include <bitset>

VERA_NAMESPACE_BEGIN

class DeviceImpl
{
public:
	using DeviceProperties              = vk::PhysicalDeviceProperties;
	using DeviceMemoryProperties        = vk::PhysicalDeviceMemoryProperties;
	using DescriptorIndexingProperties  = vk::PhysicalDeviceDescriptorIndexingProperties;

	using ShaderCacheType               = std::unordered_map<hash_t, ref<Shader>>;
	using ShaderLayoutCacheType         = std::unordered_map<hash_t, ref<ShaderLayout>>;
	using DescriptorSetLayoutCacheType  = std::unordered_map<hash_t, ref<DescriptorSetLayout>>;
	using PipelineLayoutCacheType       = std::unordered_map<hash_t, ref<PipelineLayout>>;
	using PipelineLayoutCacheType2      = std::unordered_map<hash_t, ref<PipelineLayout>>;
	using PipelineCacheType             = std::unordered_map<hash_t, ref<Pipeline>>;
	using SamplerCacheType              = std::unordered_map<hash_t, ref<Sampler>>;

	using DeviceMemoryTypes  = std::vector<DeviceMemoryType>;
	using DeviceFeatureTypes = std::vector<uint8_t>;

	obj<Context>                 context                          = {};

	vk::PhysicalDevice           physicalDevice                   = {};
	DeviceProperties             deviceProperties                 = {};
	DeviceMemoryProperties       deviceMemoryProperties           = {};
	DescriptorIndexingProperties descriptorIndexingProperties     = {};
	vk::Device                   device                           = {};
	vk::Queue                    graphicsQueue                    = {};
	vk::Queue                    computeQueue                     = {};
	vk::Queue                    transferQueue                    = {};
	vk::PipelineCache            pipelineCache                    = {};
	vk::SampleCountFlagBits      sampleCount                      = {};
	
	Format                       colorFormat                      = {};
	Format                       depthFormat                      = {};
	int32_t                      graphicsQueueFamilyIndex         = {};
	int32_t                      transferQueueFamilyIndex         = {};
	int32_t                      computeQueueFamilyIndex          = {};
	std::string                  pipelineCacheFilePath            = {};
	DeviceFeatureTypes           enabledFeatures                  = {};
	DeviceMemoryTypes            memoryTypes                      = {};

	ShaderCacheType              shaderCacheMap                   = {};
	ShaderLayoutCacheType        shaderLayoutCacheMap             = {};
	DescriptorSetLayoutCacheType descriptorSetLayoutCacheMap      = {};
	PipelineLayoutCacheType      pipelineLayoutCacheMap           = {}; // hash with layout itself
	PipelineLayoutCacheType2     pipelineLayoutCacheMapWithShader = {}; // hash with shaders
	PipelineCacheType            pipelineCacheMap                 = {};
	SamplerCacheType             samplerCacheMap                  = {};

	obj<Sampler>                 defaultSampler                   = {};
	obj<Texture>                 defaultTexture                   = {};

	VERA_NODISCARD bool isFeatureEnabled(DeviceFeatureType feature) const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t findMemoryTypeIndex(MemoryPropertyFlags flags, std::bitset<32> type_mask) VERA_NOEXCEPT;

	void registerShader(hash_t hash_value, ref<Shader> shader);
	void registerShaderLayout(hash_t hash_value, ref<ShaderLayout> shader_layout);
	void registerDescriptorSetLayout(hash_t hash_value, ref<DescriptorSetLayout> descriptor_set_layout);
	void registerPipelineLayout(hash_t hash_value, ref<PipelineLayout> pipeline_layout);
	void registerPipelineLayoutWithShaders(hash_t hash_value, ref<PipelineLayout> pipeline_layout);
	void registerPipeline(hash_t hash_value, ref<Pipeline> pipeline);
	void registerSampler(hash_t hash_value, ref<Sampler> sampler);

	void unregisterShader(hash_t hash_value) VERA_NOEXCEPT;
	void unregisterShaderLayout(hash_t hash_value) VERA_NOEXCEPT;
	void unregisterDescriptorSetLayout(hash_t hash_value) VERA_NOEXCEPT;
	void unregisterPipelineLayout(hash_t hash_value) VERA_NOEXCEPT;
	void unregisterPipelineLayoutWithShaders(hash_t hash_value) VERA_NOEXCEPT;
	void unregisterPipeline(hash_t hash_value) VERA_NOEXCEPT;
	void unregisterSampler(hash_t hash_value) VERA_NOEXCEPT;
};

VERA_NAMESPACE_END