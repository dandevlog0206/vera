#pragma once

#include "shader_reflection_impl.h"

#include "../../include/vera/core/command_buffer_sync.h"
#include <map>

VERA_NAMESPACE_BEGIN

struct ShaderStorageArrayResource
{
	using BlockData = std::vector<uint8_t>;

	obj<ResourceBinding>   resourceBinding;

	std::vector<BlockData> blockDatas;
	uint32_t               poolIndex;
};

struct ShaderStorageResource
{
	using BlockData = std::vector<uint8_t>;

	obj<ResourceBinding> resourceBinding;
	BlockData            blockData;
	uint32_t             poolIndex;
};

struct ShaderStorageFrame
{
	using StorageResources   = std::vector<ShaderStorageResource*>;
	using CommandBufferSyncs = std::vector<CommandBufferSync>;

	StorageResources   storageResources;
	CommandBufferSyncs commandSyncs;
};

struct ShaderStorageImpl
{
	using ShaderStorageFrames  = std::vector<ShaderStorageFrame>;
	using ResourceBindingPools = std::vector<obj<ResourceBindingPool>>;

	obj<Device>                 device;
	obj<ShaderReflection>       reflection;

	ShaderStorageFrames  storageFrames;
	ResourceBindingPools resourcePools;
};

VERA_NAMESPACE_END