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
	obj<Sampler> sampler;
};

struct TextureStorage : ShaderStorageData
{
	obj<Texture> texture;
};

struct CombinedImageSamplerStorage : ShaderStorageData
{
	obj<Sampler>      sampler;
	obj<Texture>      texture;
	vk::DescriptorSet descriptorSet;
};

struct BufferStorage : ShaderStorageData
{
	obj<Buffer> buffer;
};

struct BufferBlockStorage : BlockStorage
{
	obj<Buffer> buffer;
};

struct PushConstantStorage : BlockStorage
{
};

struct ShaderStorageImpl
{
	obj<Device>                     device;
	obj<ShaderReflection>           reflection;

	vk::DescriptorPool              descriptorPool;

	std::vector<ShaderStorageData*> storages;
};

VERA_NAMESPACE_END