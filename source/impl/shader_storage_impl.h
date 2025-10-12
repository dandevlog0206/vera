#pragma once

#include "object_impl.h"

#include "../../include/vera/core/command_buffer_sync.h"
#include "../../include/vera/util/ring_vector.h"

VERA_NAMESPACE_BEGIN

/*

###################################################################################################
##########   Struct Inheritance Diagram   #########################################################
###################################################################################################


ShaderStorageData
	¦§ ShaderStorageRootData
	¦¢   ¦§ ShaderStorageResourceData
	¦¢   ¦¦ ShaderStorageResourceArrayData
	¦¢
	¦§ ShaderStorageBlockData
	¦¢   ¦§ ShaderStoragePushConstantData
	¦¢   ¦¦ ShaderStorageBufferBlockData
	¦¢
	¦§ ShaderStorageSamplerData
	¦§ ShaderStorageCombinedImageSamplerData
	¦§ ShaderStorageTextureData
	¦§ ShaderStorageTexelBufferData
	¦¦ ShaderStorageBufferData


###################################################################################################
##########   Data Structure Diagram   #############################################################
###################################################################################################


ShaderStorageImpl
	¦§ storageFrames : std::vector<ShaderStorageFrame>
	¦¢   ¦§ ShaderStorageFrame
	¦¢   ¦¢   ¦§ storageResources : std::vector<ShaderStorageResource*>
	¦¢   ¦¢   ¦¢   ¦¦ ShaderStorageResource
	¦¢   ¦¢   ¦¢       ¦§ resourceBinding : obj<ResourceBinding>
	¦¢   ¦¢   ¦¢       ¦¦ poolIndex : uint32_t
	¦¢   ¦¢   ¦§ frameID : uint64_t
	¦¢   ¦¢   ¦¦ commandSync : CommandBufferSync
	¦¢   ¦¢
	¦¢   ¦§ ShaderStorageFrame
	¦¢   ¦¢   ...
	¦¢   ¦§ ShaderStorageFrame
	¦¢   ¦¢   ...
	¦¢
	¦§ storageDatas : std::vector<ShaderStorageRootData*> // contains all shader resources. sampler, texture, buffer etc...
	¦¢   ¦§ ShaderStorageResourceData
	¦¢   ¦¢   ¦¦ frames : ring_vector<ShaderStorageData*> // this is example of uniform buffer. buffer datas per frame in flight.
	¦¢   ¦¢       ¦§ ShaderStorageBlockData
	¦¢   ¦¢       ¦§ ShaderStorageBlockData
	¦¢   ¦¢       ¦§ ShaderStorageBlockData
	¦¢   ¦¢
	¦¢   ¦§ ShaderStorageResourceData
	¦¢   ¦¢   ¦¦ frames : ring_vector<ShaderStorageData*>  // this is example of combined image sampler.
	¦¢   ¦¢       ¦§ ShaderStorageCombinedImageSamplerData // not so changing per frame in flight. so just 1
	¦¢   ¦¢
	¦¢   ¦§ ShaderStorageResourceData
	¦¢   ¦¢   ...
	¦¢   ¦¢
	¦¢   ¦§ ShaderStorageResourceArrayData // this is array of resources.
	¦¢   ¦¢   ¦¦ elements : std::vector<ShaderStorageResourceData>
	¦¢   ¦¢       ¦§ ShaderStorageResourceData // each element is resource data. 
	¦¢   ¦¢       ¦§ ShaderStorageResourceData
	¦¢   ¦¢       ¦§ ShaderStorageResourceData
	¦¢   ¦¢       ¦§ ShaderStorageResourceData
	¦¢   ¦¢       ¦§ ShaderStorageResourceData
	¦¢   ¦¢       ¦§ ShaderStorageResourceData
	¦¢   ¦¢       ¦§ ShaderStorageResourceData
	¦¢   ¦¢       ¦¢   ...
	¦¢   ¦¢
	¦¢   ¦§ ShaderStorageResourceArrayData
	¦¢   ¦¢   ...
	¦¢   ¦§ ShaderStorageResourceArrayData
	¦¢   ¦¢   ...
	¦¢
	¦¦ resourcePools : std::vector<obj<ResourceBindingPool>> // one per frame in flight
		¦§ obj<ResourceBindingPool> // one pool can be shared by multiple frames in flight.
		¦§ obj<ResourceBindingPool>
		¦¦ obj<ResourceBindingPool>



*/

class ShaderStorageData
{
public:
	virtual ~ShaderStorageData() {};
};

class ShaderStorageBlockData : public ShaderStorageData
{
public:
	std::vector<uint8_t> block;
};

class ShaderStoragePushConstantData : public ShaderStorageBlockData
{

};

class ShaderStorageSamplerData : public ShaderStorageData
{
public:
	obj<Sampler> sampler;
};

class ShaderStorageCombinedImageSamplerData : public ShaderStorageData
{
public:
	obj<Sampler>     sampler;
	obj<TextureView> textureView;
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
	size_t      range;
};

class ShaderStorageBufferBlockData : public ShaderStorageBlockData
{
public:
	obj<Buffer> buffer;
};

class ShaderStorageRootData : public ShaderStorageData
{
public:
	virtual ~ShaderStorageRootData() {};
};

class ShaderStorageResourceData : public ShaderStorageRootData
{
public:
	ring_vector<ShaderStorageData*> frames;
};

class ShaderStorageResourceArrayData : public ShaderStorageRootData
{
public:
	std::vector<ShaderStorageResourceData> elements;
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
	uint64_t          frameID;
	CommandBufferSync commandSync;
};

struct ShaderStorageImpl
{
	using ShaderStorageFrames  = std::vector<ShaderStorageFrame>;
	using ShaderStorageDatas   = std::vector<ShaderStorageRootData*>;
	using ResourceBindingPools = std::vector<obj<ResourceBindingPool>>;

	obj<Device>          device;
	obj<PipelineLayout>  pipelineLayout;

	ShaderStorageFrames  storageFrames;
	ShaderStorageDatas   storageDatas;
	ResourceBindingPools resourcePools;
	uint32_t             frameIndex;
	uint64_t             frameId;
};

VERA_NAMESPACE_END