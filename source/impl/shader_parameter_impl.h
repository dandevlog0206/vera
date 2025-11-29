#pragma once

#include "object_impl.h"
#include "../spirv/reflection_node.h"
#include "../../include/vera/core/command_sync.h"
#include <map>

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class PipelineBindPoint VERA_ENUM;

class CommandBufferImpl;

class ShaderParameterDescriptorSet
{
public:
	vk::DescriptorSet     descriptorSet;
	uint32_t              variableCount;
	basic_range<uint64_t> stateIdRange;

	void allocateDescriptorSet(
		ref<DescriptorPool>      pool,
		ref<DescriptorSetLayout> layout,
		uint32_t                 variable_count);

	void destroy(ref<DescriptorPool> pool);
};

class ShaderParameterBlockStorage
{
public:
	std::vector<std::byte> block;
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

	void resize(uint32_t new_variable_count);

	struct BindingState
	{
		basic_range<uint32_t> objectRange;
		basic_range<uint32_t> blockRange;
		basic_range<uint32_t> infoRange;
		DescriptorType        descriptorType;
		uint32_t              blockStride; // for uniform/storage buffer array
		bool                  bindless;
	};

	obj<DescriptorSetLayout>                                    descriptorSetLayout;
	std::vector<ShaderParameterDescriptorSet>                   descriptorSets;
	std::map<uint32_t, BindingState>                            bindingStates;
	std::vector<obj<CoreObject>>                                objects;
	std::vector<ShaderParameterBlockStorage>                    blockStorages;
	std::vector<vk::DescriptorImageInfo>                        imageInfos;
	std::vector<vk::DescriptorBufferInfo>                       bufferInfos;
	std::vector<vk::BufferView>                                 bufferViewInfos;
	std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> accelStructInfos;
	uint32_t                                                    set;
	uint32_t                                                    currentSetIdx;
	uint32_t                                                    variableCount;
	bool                                                        descriptorSetLayoutDirty;
	bool                                                        dirty;
};

class ShaderParameterFrame
{
public:
	cref<CommandBuffer>   commandBuffer;
	CommandSync           sync;
	basic_range<uint64_t> stateIdRange;
};

class ShaderParameterImpl
{
	friend class ShaderParameter;
public:
	obj<Device>                          device;
	obj<ProgramReflection>               programReflection;
	obj<PipelineLayout>                  pipelineLayout;
	obj<DescriptorPool>                  descriptorPool;

	const ReflectionRootNode*            rootNode;
	std::vector<ShaderParameterFrame>    frames;
	std::vector<ShaderParameterSetState> setStates;
	ShaderParameterBlockStorage          pushConstantStorage;
	uint64_t                             stateId;
	uint64_t                             completeStateId;
	bool                                 pipelineLayoutDirty;

	void bind(cref<CommandBuffer> cmd_buffer);
	void prepareDescriptorWrite(uint32_t set, uint32_t binding, uint32_t array_idx);
	void setBindless(uint32_t set, uint32_t binding, bool enable);
	bool isBindless(uint32_t set, uint32_t binding) const VERA_NOEXCEPT;

private:
	void createSetStates();

	void prepareFrame(cref<CommandBuffer> cmd_buffer);
	void recreatePipelineLayout();
	void updateDescriptorSet(ShaderParameterSetState& set_state);
	bool checkStateLocked(const basic_range<uint64_t>& state_range);
};

VERA_NAMESPACE_END