#pragma once

#include "object_impl.h"

#include "detail/shader_reflection.h"
#include "../../include/vera/core/command_buffer_sync.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/util/ranged_set.h"
#include <variant>

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class PipelineBindPoint VERA_ENUM;

struct ShaderStorageSubFrame
{
	obj<DescriptorSet>    descriptorSet;
	std::vector<uint64_t> lastSubFrameIDs;
};

struct ShaderStorageFrameSet
{
	std::vector<ShaderStorageSubFrame> subFrames;
	uint32_t                           set;
	uint32_t                           variableArrayCount;
	uint32_t                           subFrameIndex;
};

struct ShaderStorageFrame
{
	obj<DescriptorPool>                descriptorPool;

	std::vector<ShaderStorageFrameSet> frameSets;

	mutable const_ref<CommandBuffer>   commandBuffer;
	mutable CommandBufferSync          commandBufferSync;

	// resources that need to be alive until command completed
	std::vector<obj<Sampler>>          samplers;
	std::vector<obj<TextureView>>      textureViews;
	std::vector<obj<BufferView>>       bufferViews;
	std::vector<obj<Buffer>>           buffers;
};

struct ShaderStorageBinding
{
	ReflectionType        reflectionType;
	DescriptorType        descriptorType;
	uint32_t              set;
	uint32_t              binding;
	uint32_t              bindingIndex;
	uint32_t              setResourceOffset;
	basic_range<uint32_t> resourceRange;
	ranged_set<uint32_t>  arrayUpdateRange;
	bool                  needUpdate;
};

struct ShaderStorageSet
{
	std::vector<ShaderStorageBinding> bindings;
	uint32_t                          set;
	uint64_t                          lastSubFrameID;
	bool                              needUpdate;
};

struct ShaderStorageData
{
};

struct ShaderStorageBlockData : ShaderStorageData
{
	std::vector<uint8_t> block;
};

struct ShaderStorageImageData : ShaderStorageData
{
	obj<Sampler>     sampler;
	obj<TextureView> textureView;
	TextureLayout    textureLayout;
};

struct ShaderStorageBufferViewData : ShaderStorageData
{
	obj<BufferView> bufferView;
};

struct ShaderStorageBufferData : ShaderStorageData
{
	obj<Buffer> buffer;
	size_t      offset;
	size_t      range;
};

struct ShaderStorageBufferBlockData : ShaderStorageBlockData
{
	obj<Buffer> buffer;
};

struct ShaderStoragePushConstantData : ShaderStorageBlockData
{
	ShaderStageFlags stageFlags;
};

class ShaderStoragePadding
{
public:
	ShaderStoragePadding() VERA_NOEXCEPT
	{
		memset(this, 0, sizeof(ShaderStoragePadding));
	}
	
	ShaderStoragePadding(const ShaderStoragePadding& rhs) VERA_NOEXCEPT
	{
		memcpy(this, &rhs, sizeof(ShaderStoragePadding));
	}

	~ShaderStoragePadding()
	{
	}

	ShaderStoragePadding& operator=(const ShaderStoragePadding& rhs) VERA_NOEXCEPT
	{
		memcpy(this, &rhs, sizeof(ShaderStoragePadding));
		return *this;
	}

	union {
		ShaderStorageImageData        imageData;
		ShaderStorageBufferViewData   bufferViewData;
		ShaderStorageBufferData       bufferData;
		ShaderStorageBufferBlockData  bufferBlockData;
		ShaderStoragePushConstantData pushConstantData;
	};
};

struct ShaderStorageResource
{
	ShaderStoragePadding          data;
	const ReflectionResourceDesc* resourceDesc;
	uint32_t                      resourceID;
	uint32_t                      set;
	uint32_t                      bindingIndex;
	uint32_t                      arrayIndex;
	uint64_t                      lastSubFrameID;
};

struct ShaderStorageImpl
{
	obj<Device>                        device;
	obj<PipelineLayout>                pipelineLayout;

	ShaderReflection                   reflection;
	PipelineBindPoint                  pipelineBindPoint;
	std::vector<basic_range<uint32_t>> resourceRanges;
	std::vector<basic_range<uint32_t>> setResourceRanges;
	basic_range<uint32_t>              pcRange;
	std::vector<ShaderStorageResource> resources;
	std::vector<ShaderStorageSet>      sets;
	std::vector<ShaderStorageFrame>    frames;
	CommandBufferSync                  currentSync;
	uint32_t                           frameIndex;
	uint32_t                           prevFrameIndex;
	uint64_t                           frameID;
	uint32_t                           subFrameIndex;
	uint64_t                           subFrameID;
};

template <class ShaderStorageDataType>
const ShaderStorageDataType& get_storage_data(const ShaderStorageResource& resource)
{
	const auto& data = reinterpret_cast<const ShaderStorageData&>(resource.data);
	return static_cast<const ShaderStorageDataType&>(data);
}

template <class ShaderStorageDataType>
ShaderStorageDataType& get_storage_data(ShaderStorageResource& resource)
{
	auto& data = reinterpret_cast<ShaderStorageData&>(resource.data);
	return static_cast<ShaderStorageDataType&>(data);
}

VERA_NAMESPACE_END