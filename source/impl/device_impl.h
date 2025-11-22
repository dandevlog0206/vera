#pragma once

#include "object_impl.h"

#include "../../include/vera/core/device.h"
#include <unordered_map>
#include <bitset>

VERA_NAMESPACE_BEGIN

class DeviceImpl
{
public:
	using DeviceProperties             = vk::PhysicalDeviceProperties;
	using DeviceMemoryProperties       = vk::PhysicalDeviceMemoryProperties;
	using DescriptorIndexingProperties = vk::PhysicalDeviceDescriptorIndexingProperties;

	using ShaderCacheType              = std::unordered_map<hash_t, ref<Shader>>;
	using ShaderReflectionCacheType    = std::unordered_map<hash_t, ref<ShaderReflection>>;
	using ProgramReflectionCacheType   = std::unordered_map<hash_t, ref<ProgramReflection>>;
	using DescriptorSetLayoutCacheType = std::unordered_map<hash_t, ref<DescriptorSetLayout>>;
	using PipelineLayoutCacheType      = std::unordered_map<hash_t, ref<PipelineLayout>>;
	using PipelineCacheType            = std::unordered_map<hash_t, ref<Pipeline>>;
	using SamplerCacheType             = std::unordered_map<hash_t, ref<Sampler>>;

	using DeviceMemoryTypes  = std::vector<DeviceMemoryType>;
	using DeviceFeatureTypes = std::vector<uint8_t>;

	obj<Context>                 context                          = {};

	vk::PhysicalDevice           vkPhysicalDevice                 = {};
	DeviceProperties             vkDeviceProperties               = {};
	DeviceMemoryProperties       vkDeviceMemoryProperties         = {};
	DescriptorIndexingProperties vkDescriptorIndexingProperties   = {};
	vk::Device                   vkDevice                         = {};
	vk::Queue                    vkGraphicsQueue                  = {};
	vk::Queue                    vkComputeQueue                   = {};
	vk::Queue                    vkTransferQueue                  = {};
	vk::PipelineCache            vkPipelineCache                  = {};
	vk::SampleCountFlagBits      vkSampleCount                    = {};
	
	Format                       colorFormat                      = {};
	Format                       depthFormat                      = {};
	int32_t                      graphicsQueueFamilyIndex         = {};
	int32_t                      transferQueueFamilyIndex         = {};
	int32_t                      computeQueueFamilyIndex          = {};
	std::string                  pipelineCacheFilePath            = {};
	DeviceFeatureTypes           enabledFeatures                  = {};
	DeviceMemoryTypes            memoryTypes                      = {};

	ShaderCacheType              shaderCache                      = {};
	ShaderReflectionCacheType    shaderReflectionCache            = {};
	ProgramReflectionCacheType   programReflectionCache           = {};
	DescriptorSetLayoutCacheType descriptorSetLayoutCache         = {};
	PipelineLayoutCacheType      pipelineLayoutCache              = {};
	PipelineCacheType            pipelineCache                    = {};
	SamplerCacheType             samplerCache                     = {};

	obj<Sampler>                 defaultSampler                   = {};
	obj<Texture>                 defaultTexture                   = {};

	VERA_NODISCARD bool isFeatureEnabled(DeviceFeatureType feature) const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t findMemoryTypeIndex(MemoryPropertyFlags flags, std::bitset<32> type_mask) VERA_NOEXCEPT;

	template <class CoreObject>
	obj<CoreObject> findCachedObject(hash_t hash_value)
	{
		static_assert("unsupported CoreObject type for caching");
	}

	template <> obj<Shader> findCachedObject<Shader>(hash_t hash_value);
	template <> obj<ShaderReflection> findCachedObject<ShaderReflection>(hash_t hash_value);
	template <> obj<ProgramReflection> findCachedObject<ProgramReflection>(hash_t hash_value);
	template <> obj<DescriptorSetLayout> findCachedObject<DescriptorSetLayout>(hash_t hash_value);
	template <> obj<PipelineLayout> findCachedObject<PipelineLayout>(hash_t hash_value);
	template <> obj<Pipeline> findCachedObject<Pipeline>(hash_t hash_value);
	template <> obj<Sampler> findCachedObject<Sampler>(hash_t hash_value);

	template <class CoreObject>
	void registerCachedObject(hash_t hash_value, ref<CoreObject> object)
	{
		VERA_ASSERT_MSG(hash_value, "cannot cache object with invalid hash value");

		if constexpr (std::is_same_v<CoreObject, Shader>){
			if (!shaderCache.emplace(hash_value, object).second)
				throw Exception("Failed to register shader: hash collision(hash= {:016x})", hash_value);
		} else if constexpr (std::is_same_v<CoreObject, ShaderReflection>) {
			if (!shaderReflectionCache.emplace(hash_value, object).second)
				throw Exception("Failed to register shader layout: hash collision(hash= {:016x})", hash_value);
		} else if constexpr (std::is_same_v<CoreObject, ProgramReflection>) {
			if (!programReflectionCache.emplace(hash_value, object).second)
				throw Exception("Failed to register program layout: hash collision(hash= {:016x})", hash_value);
		} else if constexpr (std::is_same_v<CoreObject, DescriptorSetLayout>) {
			if (!descriptorSetLayoutCache.emplace(hash_value, object).second)
				throw Exception("Failed to register descriptor set layout: hash collision(hash= {:016x})", hash_value);
		} else if constexpr (std::is_same_v<CoreObject, PipelineLayout>) {
			if (!pipelineLayoutCache.emplace(hash_value, object).second)
				throw Exception("Failed to register pipeline layout: hash collision(hash= {:016x})", hash_value);
		} else if constexpr (std::is_same_v<CoreObject, Pipeline>) {
			if (!pipelineCache.emplace(hash_value, object).second)
				throw Exception("Failed to register pipeline: hash collision(hash= {:016x})", hash_value);
		} else if constexpr (std::is_same_v<CoreObject, Sampler>) {
			if (!samplerCache.emplace(hash_value, object).second)
				throw Exception("Failed to register sampler: hash collision(hash= {:016x})", hash_value);
		} else {
			static_assert("unsupported CoreObject type for caching");
		}
	}

	template <class CoreObject>
	void unregisterCachedObject(hash_t hash_value) VERA_NOEXCEPT
	{
		size_t erased;

		VERA_ASSERT_MSG(hash_value, "cannot cache object with invalid hash value");

		if constexpr (std::is_same_v<CoreObject, Shader>){
			erased = shaderCache.erase(hash_value);
		} else if constexpr (std::is_same_v<CoreObject, ShaderReflection>) {
			erased = shaderReflectionCache.erase(hash_value);
		} else if constexpr (std::is_same_v<CoreObject, ProgramReflection>) {
			erased = programReflectionCache.erase(hash_value);
		} else if constexpr (std::is_same_v<CoreObject, DescriptorSetLayout>) {
			erased = descriptorSetLayoutCache.erase(hash_value);
		} else if constexpr (std::is_same_v<CoreObject, PipelineLayout>) {
			erased = pipelineLayoutCache.erase(hash_value);
		} else if constexpr (std::is_same_v<CoreObject, Pipeline>) {
			erased = pipelineCache.erase(hash_value);
		} else if constexpr (std::is_same_v<CoreObject, Sampler>) {
			erased = samplerCache.erase(hash_value);
		} else {
			static_assert("unsupported CoreObject type for caching");
		}

		VERA_ASSERT_MSG(erased == 1, "failed to unregister cached object");
		(void)erased;
	}
};

VERA_NAMESPACE_END