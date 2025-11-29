#include "../../include/vera/core/shader_parameter.h"
#include "../impl/command_buffer_impl.h"
#include "../impl/shader_parameter_impl.h"
#include "../impl/program_reflection_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/core/descriptor_pool.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/core/program_reflection.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

static vk::ImageLayout find_vk_image_layout(DescriptorType type)
{
	switch (type) {
	case DescriptorType::SampledTexture:
	case DescriptorType::CombinedTextureSampler:
		return vk::ImageLayout::eShaderReadOnlyOptimal;
	case DescriptorType::StorageTexture:
		return vk::ImageLayout::eGeneral;
	case DescriptorType::InputAttachment:
		return vk::ImageLayout::eShaderReadOnlyOptimal;
	}

	VERA_ERROR_MSG("unsupported descriptor type for image layout lookup");
}

static uint32_t get_element_count(const ReflectionDescriptorNode* node)
{
	VERA_ASSERT(node);

	if (node->type == ReflectionNodeType::DescriptorArray) {
		const auto* array_node = node->as<ReflectionDescriptorArrayNode>();

		if (array_node->elementCount == UINT32_MAX)
			return 32; // initial guess for unsized arrays
		else
			return array_node->elementCount;
	}

	return 1;
}

static uint32_t get_uniform_buffer_size( const ReflectionDescriptorNode* node)
{
	VERA_ASSERT(
		node &&
		node->type == ReflectionNodeType::DescriptorArray ||
		node->type == ReflectionNodeType::DescriptorBlock);

	if (node->type == ReflectionNodeType::DescriptorBlock) {
		const auto* block_node  = node->as<ReflectionDescriptorBlockNode>();
		const auto* struct_node = block_node->block->as<ReflectionStructNode>();
		return struct_node->paddedSize;
	} else /* node->type == ReflectionNodeType::DescriptorArray */ {
		const auto* array_node = node->as<ReflectionDescriptorArrayNode>();
		return get_uniform_buffer_size(array_node->elementNode);
	}
}

obj<ShaderParameter> ShaderParameter::create(
	obj<Device>            device,
	obj<ProgramReflection> program_reflection,
	obj<DescriptorPool>    descriptor_pool)
{
	auto  obj       = createNewCoreObject<ShaderParameter>();
	auto& impl      = getImpl(obj);
	auto& refl_impl = getImpl(program_reflection);

	impl.device            = device;
	impl.programReflection = std::move(program_reflection);
	impl.pipelineLayout    = PipelineLayout::create(impl.device, impl.programReflection);
	impl.descriptorPool    = descriptor_pool;
	impl.rootNode          = refl_impl.rootNode;

	if (!impl.descriptorPool) {
		DescriptorPoolCreateInfo pool_info;
		pool_info.flags =
			DescriptorPoolCreateFlagBits::FreeDescriptorSet |
			DescriptorPoolCreateFlagBits::UpdateAfterBind;

		impl.descriptorPool = DescriptorPool::create(impl.device, pool_info);
	}

	impl.createSetStates();

	if (auto pc_ranges = impl.pipelineLayout->getPushConstantRanges(); !pc_ranges.empty()) {
		size_t pc_size = 0;
	
		for (const auto& pc_range : pc_ranges)
			pc_size = std::max<size_t>(pc_size, pc_range.offset + pc_range.size);

		impl.pushConstantStorage.block.resize(pc_size);
	}

	impl.stateId             = 1;
	impl.completeStateId     = 0;
	impl.pipelineLayoutDirty = false;

	return obj;
}

ShaderParameter::~ShaderParameter() VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	destroyObjectImpl(this);
}

obj<Device> ShaderParameter::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

obj<PipelineLayout> ShaderParameter::getPipelineLayout() VERA_NOEXCEPT
{
	return getImpl(this).pipelineLayout;
}

obj<DescriptorPool> ShaderParameter::getDescriptorPool() VERA_NOEXCEPT
{
	return getImpl(this).descriptorPool;
}

ShaderVariable ShaderParameter::getRootVariable() VERA_NOEXCEPT
{
	auto& impl = getImpl(this);
	return { &impl, impl.rootNode, nullptr, 0, 0 };
}

void ShaderParameter::setBindless(uint32_t set, uint32_t binding, bool enable)
{
	getImpl(this).setBindless(set, binding, enable);
}

bool ShaderParameter::isBindless(uint32_t set, uint32_t binding) const VERA_NOEXCEPT
{
	return getImpl(this).isBindless(set, binding);
}

void ShaderParameter::reset()
{
}

///////////////////////////////////////////////////////////////////////////////

void ShaderParameterDescriptorSet::allocateDescriptorSet(
	ref<DescriptorPool>      pool,
	ref<DescriptorSetLayout> layout,
	uint32_t                 variable_count
) {
	auto vk_device = get_vk_device(pool->getDevice());
	auto vk_pool   = get_vk_descriptor_pool(pool);
	auto flags     = layout->enumerateBindings().back().flags;

	vk::DescriptorSetVariableDescriptorCountAllocateInfo var_count_info{};
	vk::DescriptorSetAllocateInfo                        alloc_info{};
	alloc_info.descriptorPool     = vk_pool;
	alloc_info.pSetLayouts        = &get_vk_descriptor_set_layout(layout);
	alloc_info.descriptorSetCount = 1;

	if (flags.has(DescriptorSetLayoutBindingFlagBits::VariableDescriptorCount)) {
		var_count_info.descriptorSetCount = 1;
		var_count_info.pDescriptorCounts  = &variable_count;
		alloc_info.pNext                  = &var_count_info;
	}

	auto result = vk_device.allocateDescriptorSets(&alloc_info, &descriptorSet);

	if (result != vk::Result::eSuccess)
		throw Exception("Failed to allocate descriptor sets for ShaderParameter");

	variableCount = variable_count;
}

void ShaderParameterDescriptorSet::destroy(ref<DescriptorPool> pool)
{
	auto vk_device = get_vk_device(pool->getDevice());
	auto vk_pool = get_vk_descriptor_pool(pool);

	if (pool->getFlags().has(DescriptorPoolCreateFlagBits::FreeDescriptorSet))
		vk_device.freeDescriptorSets(vk_pool, 1, &descriptorSet);

	descriptorSet = nullptr;
	variableCount = 0;
	stateIdRange  = {};
}

///////////////////////////////////////////////////////////////////////////////

void vr::ShaderParameterSetState::writeSampler(obj<Sampler> sampler, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_state = bindingStates.at(binding);

	uint32_t sampler_off =
		binding_state.descriptorType == DescriptorType::CombinedTextureSampler ? 2 * array_idx : array_idx;

	VERA_ASSERT(array_idx < binding_state.infoRange.size());
	VERA_ASSERT(sampler_off < binding_state.objectRange.size());

	uint32_t info_idx    = binding_state.infoRange.first() + array_idx;
	uint32_t sampler_idx = binding_state.objectRange.first() + sampler_off;

	imageInfos[info_idx].sampler = get_vk_sampler(sampler);
	objects[sampler_idx]         = std::move(sampler);
}

void vr::ShaderParameterSetState::writeTextureView(obj<TextureView> texture_view, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_state = bindingStates.at(binding);

	uint32_t texture_off =
		binding_state.descriptorType == DescriptorType::CombinedTextureSampler ? 2 * array_idx + 1 : array_idx;

	VERA_ASSERT(array_idx < binding_state.infoRange.size());
	VERA_ASSERT(texture_off < binding_state.objectRange.size());

	uint32_t info_idx   = binding_state.infoRange.first() + array_idx;
	uint32_t object_idx = binding_state.objectRange.first() + texture_off;

	imageInfos[info_idx].imageView   = get_vk_image_view(texture_view);
	imageInfos[info_idx].imageLayout = find_vk_image_layout(binding_state.descriptorType);
	objects[object_idx]              = std::move(texture_view);
}

void vr::ShaderParameterSetState::writeBufferView(obj<BufferView> buffer_view, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_state = bindingStates.at(binding);

	VERA_ASSERT(array_idx < binding_state.infoRange.size());
	VERA_ASSERT(array_idx < binding_state.objectRange.size());

	uint32_t info_idx   = binding_state.infoRange.first() + array_idx;
	uint32_t object_idx = binding_state.objectRange.first() + array_idx;

	bufferViewInfos[info_idx] = get_vk_buffer_view(buffer_view);
	objects[object_idx]       = std::move(buffer_view);
}

void vr::ShaderParameterSetState::writeBuffer(obj<Buffer> buffer, size_t offset, size_t range, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_state = bindingStates.at(binding);

	VERA_ASSERT(array_idx < binding_state.infoRange.size());
	VERA_ASSERT(array_idx < binding_state.objectRange.size());

	uint32_t info_idx   = binding_state.infoRange.first() + array_idx;
	uint32_t object_idx = binding_state.objectRange.first() + array_idx;

	bufferInfos[info_idx].buffer = get_vk_buffer(buffer);
	bufferInfos[info_idx].offset = offset;
	bufferInfos[info_idx].range  = range;
	objects[object_idx]          = std::move(buffer);
}

void ShaderParameterSetState::resize(uint32_t new_variable_count)
{
	if (new_variable_count <= variableCount)
	variableCount = new_variable_count;
}

///////////////////////////////////////////////////////////////////////////////

void ShaderParameterImpl::bind(cref<CommandBuffer> cmd_buffer)
{
	prepareFrame(cmd_buffer);

	if (pipelineLayoutDirty)
		recreatePipelineLayout();

	auto& cmd_impl           = CoreObject::getImpl(cmd_buffer);
	auto  vk_device          = get_vk_device(cmd_impl.device);
	auto  vk_cmd_buffer      = cmd_impl.vkCommandBuffer;
	auto  pc_ranges          = pipelineLayout->getPushConstantRanges();
	auto  vk_pipeline_layout = get_vk_pipeline_layout(pipelineLayout);
	bool  has_dirty          = false;

	for (auto& set_state : setStates) {
		if (set_state.dirty) {
			updateDescriptorSet(set_state);
			has_dirty = true;
		}

		uint32_t curr_idx = set_state.currentSetIdx;
		auto&    curr_set = set_state.descriptorSets[curr_idx];

		vk_cmd_buffer.bindDescriptorSets(
			to_vk_pipeline_bind_point(programReflection->getPipelineBindPoint()),
			vk_pipeline_layout,
			0,
			1,
			&curr_set.descriptorSet,
			0,
			nullptr);
	}

	for (const auto& pc_range : pc_ranges) {
		vk_cmd_buffer.pushConstants(
			vk_pipeline_layout,
			to_vk_shader_stage_flags(pc_range.stageFlags),
			pc_range.offset,
			pc_range.size,
			pushConstantStorage.block.data() + pc_range.offset);
	}

	if (has_dirty) {
		for (auto& set_state : setStates) {
			auto& curr_set = set_state.descriptorSets[set_state.currentSetIdx];
			curr_set.stateIdRange = { curr_set.stateIdRange.first(), stateId + 1 };
		}

		stateId++;
	}
}

void ShaderParameterImpl::prepareDescriptorWrite(uint32_t set, uint32_t binding, uint32_t array_idx)
{
	// if set equals UINT32_MAX, that is push constant
	if (set == UINT32_MAX) return;

	if (auto& set_state = setStates[set]; !set_state.dirty) {
		auto& binding_state = set_state.bindingStates.at(binding);

		if (set_state.variableCount == 0) { // set has no variable count
			VERA_ASSERT(array_idx < binding_state.infoRange.size());

			if (!binding_state.bindless)
				set_state.dirty = true;
		} else if (set_state.variableCount <= array_idx) {
			set_state.resize(array_idx + 1);
			set_state.dirty = true;
		}
	}
}

void ShaderParameterImpl::setBindless(uint32_t set, uint32_t binding, bool enable)
{
	if (set >= setStates.size())
		throw Exception("set index out of range");

	auto& set_state      = setStates[set];
	auto& binding_states = set_state.bindingStates;
	auto  it             = binding_states.find(binding);

	if (it == binding_states.end())
		throw Exception("binding not found");

	if (auto& binding_state = it->second; enable != binding_state.bindless) {
		binding_state.bindless             = enable;
		set_state.descriptorSetLayoutDirty = true;
		set_state.dirty                    = true;
		pipelineLayoutDirty                = true;
	}
}

bool ShaderParameterImpl::isBindless(uint32_t set, uint32_t binding) const VERA_NOEXCEPT
{
	if (set >= setStates.size())
		return false;

	auto& binding_states = setStates[set].bindingStates;
	auto  it             = binding_states.find(binding);

	return it == binding_states.end() ? false : it->second.bindless;
}

void ShaderParameterImpl::createSetStates()
{
	setStates.reserve(rootNode->setCount);

	for (uint32_t set_idx = 0; set_idx < rootNode->setCount; ++set_idx) {
		auto&    new_set_state      = setStates.emplace_back();
		auto     set_layout         = pipelineLayout->getDescriptorSetLayout(set_idx);
		uint32_t object_offset      = 0;
		uint32_t block_offset       = 0;
		uint32_t image_offset       = 0;
		uint32_t buffer_offset      = 0;
		uint32_t buffer_view_offset = 0;
		uint32_t as_offset          = 0;
		uint32_t elem_count         = 0;

		for (const auto* binding : rootNode->enumerateDescriptorSet(set_idx)) {
			ShaderParameterSetState::BindingState binding_state = {};

			elem_count = get_element_count(binding);

			switch (binding->descriptorType) {
			case DescriptorType::Sampler: {
				binding_state.objectRange = { object_offset, object_offset + elem_count };
				binding_state.infoRange   = { image_offset, image_offset + elem_count };
				object_offset += elem_count;
				image_offset  += elem_count;
			} break;
			case DescriptorType::CombinedTextureSampler: {
				binding_state.objectRange  = { object_offset, object_offset + 2 * elem_count };
				binding_state.infoRange    = { image_offset, image_offset + elem_count };
				object_offset += 2 * elem_count;
				image_offset  += elem_count;
			} break;
			case DescriptorType::SampledTexture:
			case DescriptorType::StorageTexture:
			case DescriptorType::InputAttachment: {
				binding_state.objectRange = { object_offset, object_offset + elem_count };
				binding_state.infoRange   = { image_offset, image_offset + elem_count };
				object_offset += elem_count;
				image_offset  += elem_count;
			} break;
			case DescriptorType::UniformTexelBuffer:
			case DescriptorType::StorageTexelBuffer: {
				binding_state.objectRange = { object_offset, object_offset + elem_count };
				binding_state.infoRange   = { buffer_view_offset, buffer_view_offset + elem_count };
				object_offset      += elem_count;
				buffer_view_offset += elem_count;
			} break;
			case DescriptorType::UniformBuffer:
			case DescriptorType::StorageBuffer:
			case DescriptorType::UniformBufferDynamic:
			case DescriptorType::StorageBufferDynamic: {
				if (binding->type == ReflectionNodeType::DescriptorBlock) {
					binding_state.blockRange = { block_offset, block_offset + elem_count };
					block_offset            += elem_count;
				}

				binding_state.objectRange = { object_offset, object_offset + elem_count };
				binding_state.infoRange   = { buffer_offset, buffer_offset + elem_count };
				object_offset += elem_count;
				buffer_offset += elem_count;
			} break;
			case DescriptorType::AccelerationStructure:
			case DescriptorType::AccelerationStructureNV: {
				binding_state.objectRange = { object_offset, object_offset + elem_count };
				binding_state.infoRange   = { as_offset, as_offset + elem_count };
				object_offset += elem_count;
				as_offset     += elem_count;
			} break;
			}

			auto layout_binding = set_layout->getBinding(binding->binding);
			auto is_bindless    = layout_binding.flags.has(DescriptorSetLayoutBindingFlagBits::UpdateAfterBind);

			binding_state.descriptorType = binding->descriptorType;
			binding_state.blockStride    = 0;
			binding_state.bindless       = is_bindless;

			new_set_state.bindingStates.insert(std::make_pair(binding->binding, binding_state));
			new_set_state.objects.resize(object_offset);
			new_set_state.blockStorages.resize(block_offset);
			new_set_state.imageInfos.resize(image_offset);
			new_set_state.bufferInfos.resize(buffer_offset);
			new_set_state.bufferViewInfos.resize(buffer_view_offset);
			new_set_state.accelStructInfos.resize(as_offset);

			// write default objects
			for (const auto& [binding, binding_state] : new_set_state.bindingStates) {
				switch (binding_state.descriptorType) {
				case DescriptorType::Sampler:
				case DescriptorType::CombinedTextureSampler: {
					for (uint32_t i = 0; i < binding_state.infoRange.size(); ++i)
						new_set_state.writeSampler(device->getDefaultSampler(), binding, i);
				} break;
				case DescriptorType::UniformBuffer: {
					// TODO: use dynamic uniform buffer later
				} break;
				case DescriptorType::StorageBuffer: {

				} break;
				}
			}
		}

		if (auto& last_binding = set_layout->enumerateBindings().back();
			last_binding.flags.has(DescriptorSetLayoutBindingFlagBits::VariableDescriptorCount)) {
			new_set_state.variableCount = elem_count;
		}

		new_set_state.descriptorSets.resize(1);

		new_set_state.descriptorSetLayout      = std::move(set_layout);
		new_set_state.set                      = set_idx;
		new_set_state.currentSetIdx            = 0;
		new_set_state.descriptorSetLayoutDirty = false;
		new_set_state.dirty                    = true;
	}
}

void ShaderParameterImpl::prepareFrame(cref<CommandBuffer> cmd_buffer)
{
	for (auto& frame : frames) {
		if (frame.commandBuffer == cmd_buffer) {
			if (frame.stateIdRange.empty())
				frame.stateIdRange = { stateId };
			else
				frame.stateIdRange = { frame.stateIdRange.first(), stateId + 1 };

			// TODO: optimize sync update
			frame.sync         = cmd_buffer->getSync();
			return;
		}
	}

	auto& new_frame = frames.emplace_back();
	new_frame.commandBuffer = cmd_buffer;
	new_frame.sync          = cmd_buffer->getSync();
	new_frame.stateIdRange  = { stateId };
}

void ShaderParameterImpl::recreatePipelineLayout()
{
	if (!pipelineLayoutDirty) return;

	PipelineLayoutCreateInfo layout_info;

	for (auto& set_state : setStates) {
		if (!set_state.descriptorSetLayoutDirty) {
			layout_info.descriptorSetLayouts.push_back(set_state.descriptorSetLayout);
			continue;
		}

		DescriptorSetLayoutCreateInfo set_layout_info;

		for (const auto& [binding, binding_state] : set_state.bindingStates) {
			auto layout_binding = pipelineLayout->getDescriptorSetLayout(set_state.set)->getBinding(binding);

			if (binding_state.bindless) {
				set_layout_info.flags += DescriptorSetLayoutCreateFlagBits::UpdateAfterBindPool;
				layout_binding.flags  += DescriptorSetLayoutBindingFlagBits::UpdateAfterBind;
			} else {
				layout_binding.flags -= DescriptorSetLayoutBindingFlagBits::UpdateAfterBind;
			}

			set_layout_info.bindings.push_back(layout_binding);
		}

		auto new_set_layout = DescriptorSetLayout::create(device, set_layout_info);

		layout_info.descriptorSetLayouts.push_back(new_set_layout);
		set_state.descriptorSetLayout      = std::move(new_set_layout);
		set_state.descriptorSetLayoutDirty = false;
	}

	auto pc_ranges = pipelineLayout->getPushConstantRanges();

	layout_info.pushConstantRanges.assign(VERA_SPAN(pc_ranges));

	pipelineLayout      = PipelineLayout::create(device, layout_info);
	pipelineLayoutDirty = false;
}

void ShaderParameterImpl::updateDescriptorSet(ShaderParameterSetState& set_state)
{
	auto vk_device = get_vk_device(device);
	auto vk_pool   = get_vk_descriptor_pool(descriptorPool);

	uint32_t curr_idx = set_state.currentSetIdx;
	uint32_t next_idx = (curr_idx + 1) % set_state.descriptorSets.size();
	auto&    next_set = set_state.descriptorSets[next_idx];

	if (curr_idx == next_idx) {
		if (checkStateLocked(next_set.stateIdRange)) {
			auto  at      = set_state.descriptorSets.cbegin() + curr_idx + 1;
			auto& new_set = *set_state.descriptorSets.emplace(at);

			new_set.allocateDescriptorSet(
				descriptorPool,
				set_state.descriptorSetLayout,
				set_state.variableCount);
			
			set_state.currentSetIdx = curr_idx + 1;
		} else {
			set_state.descriptorSets[curr_idx].allocateDescriptorSet(
				descriptorPool,
				set_state.descriptorSetLayout,
				set_state.variableCount);
		}
	} else if (checkStateLocked(next_set.stateIdRange)) {
		auto  at      = set_state.descriptorSets.cbegin() + curr_idx + 1;
		auto& new_set = *set_state.descriptorSets.emplace(at);

		new_set.allocateDescriptorSet(
			descriptorPool,
			set_state.descriptorSetLayout,
			set_state.variableCount);

		set_state.currentSetIdx = curr_idx + 1;
	} else if (next_set.variableCount < set_state.variableCount) {
		next_set.destroy(descriptorPool);
		next_set.allocateDescriptorSet(
			descriptorPool,
			set_state.descriptorSetLayout,
			set_state.variableCount);

		set_state.currentSetIdx = next_idx;
	} else {
		set_state.currentSetIdx = next_idx;
	}

	// TODO: remove later
	VERA_ASSERT(set_state.descriptorSets.size() < 128); // avoid overflow

	auto& curr_set    = set_state.descriptorSets[set_state.currentSetIdx];
	auto  vk_desc_set = curr_set.descriptorSet;

	small_vector<vk::WriteDescriptorSet, 32> write_infos;

	for (const auto& [binding, binding_state] : set_state.bindingStates) {
		auto& write_info = binding_state.infoRange;

		switch (binding_state.descriptorType) {
		case DescriptorType::Sampler:
		case DescriptorType::CombinedTextureSampler:
		case DescriptorType::SampledTexture:
		case DescriptorType::StorageTexture:
		case DescriptorType::InputAttachment: {
			auto& vk_write_info = write_infos.emplace_back();
			vk_write_info.dstSet          = vk_desc_set;
			vk_write_info.dstBinding      = binding;
			vk_write_info.dstArrayElement = 0;
			vk_write_info.descriptorCount = static_cast<uint32_t>(binding_state.infoRange.size());
			vk_write_info.descriptorType  = to_vk_descriptor_type(binding_state.descriptorType);
			vk_write_info.pImageInfo      = &set_state.imageInfos[write_info.first()];
		} break;
		case DescriptorType::UniformTexelBuffer:
		case DescriptorType::StorageTexelBuffer: {
			auto& vk_write_info = write_infos.emplace_back();
			vk_write_info.dstSet           = vk_desc_set;
			vk_write_info.dstBinding       = binding;
			vk_write_info.dstArrayElement  = 0;
			vk_write_info.descriptorCount  = static_cast<uint32_t>(binding_state.infoRange.size());
			vk_write_info.descriptorType   = to_vk_descriptor_type(binding_state.descriptorType);
			vk_write_info.pTexelBufferView = &set_state.bufferViewInfos[write_info.first()];
		} break;
		case DescriptorType::UniformBuffer:
		case DescriptorType::StorageBuffer:
		case DescriptorType::UniformBufferDynamic:
		case DescriptorType::StorageBufferDynamic: {
			auto& vk_write_info = write_infos.emplace_back();
			vk_write_info.dstSet          = vk_desc_set;
			vk_write_info.dstBinding      = binding;
			vk_write_info.dstArrayElement = 0;
			vk_write_info.descriptorCount = static_cast<uint32_t>(binding_state.infoRange.size());
			vk_write_info.descriptorType  = to_vk_descriptor_type(binding_state.descriptorType);
			vk_write_info.pBufferInfo     = &set_state.bufferInfos[write_info.first()];
		} break;
		case DescriptorType::AccelerationStructure:
		case DescriptorType::AccelerationStructureNV: {
			auto& vk_write_info = write_infos.emplace_back();
			vk_write_info.dstSet          = vk_desc_set;
			vk_write_info.dstBinding      = binding;
			vk_write_info.dstArrayElement = 0;
			vk_write_info.descriptorCount = static_cast<uint32_t>(binding_state.infoRange.size());
			vk_write_info.descriptorType  = to_vk_descriptor_type(binding_state.descriptorType);
			vk_write_info.pNext           = &set_state.accelStructInfos[write_info.first()];
		} break;
		}
	}
	
	vk_device.updateDescriptorSets(
		static_cast<uint32_t>(write_infos.size()),
		write_infos.data(),
		0,
		nullptr);

	curr_set.stateIdRange = { stateId };
	set_state.dirty       = false;
}

bool ShaderParameterImpl::checkStateLocked(const basic_range<uint64_t>& state_range)
{
	if (state_range.empty() || state_range.last() <= completeStateId)
		return false;

	for (auto& frame : frames) {
		if (!state_range.intersect(frame.stateIdRange)) continue;

		switch (frame.sync.getState()) {
		case CommandBufferState::Invalid:
			// TODO: remove this frame
			continue;
		case CommandBufferState::Initial:
			VERA_ERROR_MSG("invalid command buffer state");
		case CommandBufferState::Recording:
		case CommandBufferState::Executable:
			return true;
		case CommandBufferState::Pending:
			return true;
		case CommandBufferState::Complete:
			frame.stateIdRange = {};
			continue;
		}
	}

	completeStateId = state_range.last() - 1;

	return false;
}

VERA_NAMESPACE_END
