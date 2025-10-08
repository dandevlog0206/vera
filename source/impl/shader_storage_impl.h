#pragma once

#include "object_impl.h"

#include "../../include/vera/core/command_buffer_sync.h"
#include <map>

VERA_NAMESPACE_BEGIN

class ShaderStorageData
{
public:
	virtual ~ShaderStorageData() {};
};

class ShaderStoragePushConstantData : public ShaderStorageData
{
public:
	std::vector<uint8_t> block;
};

class ShaderStorageSamplerData : public ShaderStorageData
{
public:
	obj<Sampler> sampler;
};

class ShaderStorageCombinedImageSamplerData : public ShaderStorageData
{
public:
	obj<Sampler> sampler;
	obj<Texture> texture;
};

class ShaderStorageTextureData : public ShaderStorageData
{
public:
	obj<TextureView> textureView;
};

class ShaderStorageTexelBufferData : public ShaderStorageData
{
public:
	obj<BufferView> bufferView;
};

class ShaderStorageBufferData : public ShaderStorageData
{
public:
	obj<Buffer> buffer;
	size_t      offset;
	size_t      size;
};

class ShaderStorageBufferBlockData : public ShaderStorageData
{
public:
	obj<Buffer>          buffer;
	std::vector<uint8_t> block;
};

class ShaderStorageSamplerArrayData : public ShaderStorageData
{
public:
	std::vector<ShaderStorageSamplerData> elements;
};

class ShaderStorageCombinedImageSamplerArrayData : public ShaderStorageData
{
public:
	std::vector<ShaderStorageCombinedImageSamplerData> elements;
};

class ShaderStorageTextureArrayData : public ShaderStorageData
{
public:
	std::vector<ShaderStorageTextureData> elements;
};

class ShaderStorageTexelBufferArrayData : public ShaderStorageData
{
public:
	std::vector<ShaderStorageTexelBufferData> elements;
};

class ShaderStorageBufferArrayData : public ShaderStorageData
{
public:
	std::vector<ShaderStorageBufferData> elements;
};

class ShaderStorageBufferBlockArrayData : public ShaderStorageData
{
public:
	std::vector<ShaderStorageBufferBlockData> elements;
};

struct ShaderStorageResource
{
	obj<ResourceBinding> resourceBinding;
	uint32_t             poolIndex;
};

struct ShaderStorageFrame
{
	using StorageResources = std::vector<ShaderStorageResource*>;

	StorageResources  storageResources;
	CommandBufferSync commandSync;
};

struct ShaderStorageImpl
{
	using ShaderStorageFrames  = std::vector<ShaderStorageFrame>;
	using ShaderStorageDatas   = std::vector<ShaderStorageData*>;
	using ResourceBindingPools = std::vector<obj<ResourceBindingPool>>;

	obj<Device>           device;
	obj<ShaderReflection> reflection;

	ShaderStorageFrames   storageFrames;
	ShaderStorageDatas    storageDatas;
	ResourceBindingPools  resourcePools;
	uint32_t              frameIndex;
};

VERA_NAMESPACE_END