#pragma once

#include "object_impl.h"

#include "../../include/vera/core/command_buffer_sync.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/util/ranged_set.h"
#include <queue>
#include <map>

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class PipelineBindPoint VERA_ENUM;

class ShaderStorageFrame
{
public:
	CommandBufferSync            commandBufferSync;
	std::vector<obj<CoreObject>> descriptorObjects;
	basic_range<uint64_t>        stateIDRange;
};

class ShaderParameterDescriptorSet
{
public:
	obj<DescriptorSet>    descriptorSet;
	basic_range<uint64_t> stateIDRange;
};

class ShaderParameterBindingState
{
public:

	basic_range<uint32_t> imageInfoRange;
	basic_range<uint32_t> bufferInfoRange;
	basic_range<uint32_t> bufferViewRange;
};

class ShaderParameterSetState
{
public:
	using BindingStateMap = std::map<uint32_t, ShaderParameterBindingState>;

	std::vector<ShaderParameterDescriptorSet> descriptorSetFrames;
	BindingStateMap                           bindingStates;
	uint32_t                                  currentSetFrameID;
};

class ShaderStoragePCRange
{
public:
	ShaderStageFlags     stageFlags;
	std::vector<uint8_t> data;
};

class ShaderParameterImpl
{
public:
	obj<Device>                                      device;
	obj<PipelineLayout>                              pipelineLayout;
	obj<DescriptorPool>                              descriptorPool;

	std::vector<ShaderParameterSetState>             setStates;
	std::map<ShaderStageFlags, ShaderStoragePCRange> pcStates;
	std::vector<vk::DescriptorImageInfo>             imageInfos;
	std::vector<vk::DescriptorBufferInfo>            bufferInfos;
	std::vector<vk::BufferView>                      bufferViews;
	uint64_t                                         currentStateID;
	uint64_t                                         completeStateID;

	void bind(CommandBufferImpl& cmd_impl);
	void prepareDescriptorWrite(uint32_t set, uint32_t binding, uint32_t array_idx);
	void submitFrame(CommandBufferImpl& cmd_impl);

private:
	bool checkStateLocked(uint64_t state_id) const;
};

VERA_NAMESPACE_END