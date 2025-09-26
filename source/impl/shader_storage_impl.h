#pragma once

#include "object_impl.h"

#include "../../include/vera/core/resource_layout.h"

VERA_NAMESPACE_BEGIN

enum class ShaderStorageDataType VERA_ENUM
{
	ResourceArray,
	Sampler,
	Texture,
	CombinedImageSampler,
	Buffer,
	BufferBlock,
	PushConstant
};

struct ShaderStorageData
{
	ShaderStorageDataType storageType;
	ResourceType          resourceType;
	ShaderStageFlags      shaderStageFlags;
};

struct ResourceArrayStorage : ShaderStorageData
{
	uint32_t                        elementCount;
	std::vector<ShaderStorageData*> elements;
};

struct BlockStorage : ShaderStorageData
{
	std::vector<uint8_t> blockStorage;
};

struct SamplerStorage : ShaderStorageData
{
	ref<Sampler> sampler;
};

struct TextureStorage : ShaderStorageData
{
	ref<Texture> texture;
};

struct CombinedImageSamplerStorage : ShaderStorageData
{
	ref<Sampler>      sampler;
	ref<Texture>      texture;
	vk::DescriptorSet descriptorSet;
};

struct BufferStorage : ShaderStorageData
{
	ref<Buffer> buffer;
};

struct BufferBlockStorage : BlockStorage
{
	ref<Buffer> buffer;
};

struct PushConstantStorage : BlockStorage
{
};

struct ShaderStorageImpl
{
	ref<Device>                     device;

	vk::DescriptorPool              descriptorPool;

	std::vector<ShaderStorageData*> storages;
};

VERA_NAMESPACE_END