#pragma once

#include "object_impl.h"
#include "../spirv/reflection_node.h"
#include "../../include/vera/core/command_buffer_sync.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/util/ranged_set.h"
#include <unordered_map>
#include <deque>
#include <string>

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class PipelineBindPoint VERA_ENUM;

class ShaderParameterDescriptorSet
{
public:
	vk::DescriptorSet     descriptorSet;
	basic_range<uint64_t> stateIdRange;

	void allocateDescriptorSet(
		ref<DescriptorPool>      pool,
		ref<DescriptorSetLayout> layout,
		uint32_t                 elem_count);
};

class ShaderParameterBlockStorage
{
public:
	uint32_t               set;
	uint32_t               binding;
	std::vector<std::byte> data;
};

class ShaderParameterSetState
{
public:
	void writeSampler(
		obj<Sampler> sampler,
		uint32_t     binding,
		uint32_t     array_idx = 0);

	void writeTextureView(
		obj<TextureView> texture_view,
		uint32_t         binding,
		uint32_t         array_idx = 0);

	void writeCombinedTextureSampler(
		obj<Sampler>     sampler,
		obj<TextureView> texture_view,
		uint32_t         binding,
		uint32_t         array_idx = 0);

	void writeBufferView(
		obj<BufferView> buffer_view,
		uint32_t        binding,
		uint32_t        array_idx = 0);

	void writeBuffer(
		obj<Buffer> buffer,
		size_t      offset,
		size_t      range,
		uint32_t    binding,
		uint32_t    array_idx = 0);

	// TODO: implement writeAccelerationStructure

	struct BindingRange
	{
		basic_range<uint32_t> objectRange;
		basic_range<uint32_t> samplerRange;
		basic_range<uint32_t> blockRange;
		basic_range<uint32_t> infoRange;
	};

	std::unordered_map<uint32_t, BindingRange>                  bindingRanges;
	std::vector<obj<CoreObject>>                                objects;
	std::vector<ShaderParameterBlockStorage>                    blockStorages;
	std::vector<vk::DescriptorImageInfo>                        imageInfos;
	std::vector<vk::DescriptorBufferInfo>                       bufferInfos;
	std::vector<vk::BufferView>                                 bufferViewInfos;
	std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> accelStructInfos;
	bool                                                        bindless;
	bool                                                        dirty;
};

class ShaderParameterFrame
{
public:
	CommandBufferSync     sync;
	basic_range<uint64_t> stateIdRange;
};

class ShaderParameterImpl
{
public:
	obj<Device>                               device;
	obj<ProgramReflection>                    programReflection;
	obj<PipelineLayout>                       pipelineLayout;
	obj<DescriptorPool>                       descriptorPool;

	const ReflectionRootNode*                 rootNode;
	std::deque<ShaderParameterFrame>          frames;
	std::vector<ShaderParameterSetState>      setStates;
	std::vector<ShaderParameterDescriptorSet> descriptorSets;
	ShaderParameterBlockStorage               pushConstantStorage;
	uint64_t                                  stateId;

	void bind(CommandBufferImpl& cmd_impl);
	void prepareDescriptorWrite(uint32_t set, uint32_t binding, uint32_t array_idx);
	void submitFrame(CommandBufferImpl& cmd_impl);

private:
	bool checkStateLocked(uint64_t state_id) const;
};

VERA_NAMESPACE_END