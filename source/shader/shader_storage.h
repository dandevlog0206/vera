#pragma once

#include "../../include/vera/core/resource_layout.h"

VERA_NAMESPACE_BEGIN

class Sampler;
class Texture;
class Buffer;

enum class ShaderStorageType VERA_ENUM
{
	ResourceArray,
	Sampler,
	Texture,
	CombinedImageSampler,
	Buffer,
	BufferBlock,
	PushConstant
};

struct ShaderStorage
{
	ShaderStorageType storageType;
	ResourceType      resourceType;
	ShaderStageFlags  shaderStageFlags;
};

struct ResourceArrayStorage : ShaderStorage
{
	uint32_t                    elementCount;
	std::vector<ShaderStorage*> elements;
};

struct BlockStorage : ShaderStorage
{
	std::vector<uint8_t> blockStorage;
};

struct SamplerStorage : ShaderStorage
{
	ref<Sampler> sampler;
};

struct TextureStorage : ShaderStorage
{
	ref<Texture> texture;
};

struct CombinedImageSamplerStorage : ShaderStorage
{
	ref<Sampler> sampler;
	ref<Texture> texture;
};

struct BufferStorage : ShaderStorage
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

VERA_NAMESPACE_END