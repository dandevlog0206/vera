#pragma once

#include "shader_reflection_impl.h"

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
	vk::DescriptorSet               descriptorSet;
};

struct BlockStorage : ShaderStorageData
{
	std::vector<uint8_t> blockStorage;
};

struct SamplerStorage : ShaderStorageData
{
	obj<Sampler>      sampler;
	vk::DescriptorSet descriptorSet;
	int32_t           elementIndex;
};

struct TextureStorage : ShaderStorageData
{
	obj<Texture>      texture;
	vk::DescriptorSet descriptorSet;
	int32_t           elementIndex;
};

struct CombinedImageSamplerStorage : ShaderStorageData
{
	obj<Sampler>      sampler;
	obj<Texture>      texture;
	vk::DescriptorSet descriptorSet;
	int32_t           elementIndex;
};

struct BufferStorage : ShaderStorageData
{
	obj<Buffer>       buffer;
	vk::DescriptorSet descriptorSet;
	int32_t           elementIndex;
};

struct BufferBlockStorage : BlockStorage
{
	obj<Buffer>       buffer;
	vk::DescriptorSet descriptorSet;
	int32_t           elementIndex;
};

struct PushConstantStorage : BlockStorage
{
};

struct ShaderStorageFrameData
{
	const_ref<Fence>                fence;
	std::vector<ShaderStorageData*> storages;
	vk::DescriptorPool              descriptorPool;
};

struct ShaderStorageImpl
{
	obj<Device>                         device;
	obj<ShaderReflection>               reflection;

	std::vector<ShaderStorageFrameData> frames;
	int32_t                             frameIndex;
	int32_t                             lastFrameIndex;
};

ShaderStorageData* create_resource_block_storage(const ReflectionResourceBlockDesc& desc, int32_t elem_idx);
ShaderStorageData* create_resource_storage(const ReflectionResourceDesc& desc, int32_t elem_idx);

VERA_NAMESPACE_END