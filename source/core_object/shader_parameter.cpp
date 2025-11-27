#include "../../include/vera/core/shader_parameter.h"
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

static uint32_t get_element_count(const ReflectionDescriptorNode* node)
{
	VERA_ASSERT(node);

	if (node->type == ReflectionNodeType::DescriptorArray) {
		const auto* array_node = node->as<ReflectionArrayNode>();

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

	impl.device            = std::move(device);
	impl.programReflection = std::move(program_reflection);
	impl.pipelineLayout    = PipelineLayout::create(impl.device, impl.programReflection);
	impl.descriptorPool    = descriptor_pool;
	impl.rootNode          = refl_impl.rootNode;

	if (!impl.descriptorPool)
		impl.descriptorPool = DescriptorPool::create(impl.device);
	
	auto vk_device = get_vk_device(impl.device);
	auto vk_pool   = get_vk_descriptor_pool(impl.descriptorPool);

	impl.setStates.reserve(impl.rootNode->setCount);
	impl.descriptorSets.reserve(impl.rootNode->setCount);

	for (uint32_t set_idx = 0; set_idx < impl.rootNode->setCount; ++set_idx) {
		auto&    new_set_state      = impl.setStates.emplace_back();
		auto&    new_desc_set       = impl.descriptorSets.emplace_back();
		uint32_t object_offset      = 0;
		uint32_t block_offset       = 0;
		uint32_t image_offset       = 0;
		uint32_t buffer_offset      = 0;
		uint32_t buffer_view_offset = 0;
		uint32_t as_offset          = 0;
		uint32_t elem_count         = 0;

		for (const auto* binding : impl.rootNode->enumerateDescriptorSet(set_idx)) {
			ShaderParameterSetState::BindingRange binding_range;

			elem_count = get_element_count(binding);

			switch (binding->descriptorType) {
			case DescriptorType::Sampler: {
				binding_range.samplerRange = { object_offset, object_offset + elem_count };
				binding_range.infoRange    = { image_offset, image_offset + elem_count };
				object_offset += elem_count;
				image_offset  += elem_count;
			} break;
			case DescriptorType::CombinedTextureSampler: {
				object_offset += elem_count;
				binding_range.objectRange  = { object_offset - elem_count, object_offset };
				binding_range.samplerRange = { object_offset, object_offset + elem_count };
				binding_range.infoRange    = { image_offset, image_offset + elem_count };
				object_offset += elem_count;
				image_offset  += elem_count;
			} break;
			case DescriptorType::SampledTexture:
			case DescriptorType::StorageTexture:
			case DescriptorType::InputAttachment: {
				binding_range.objectRange = { object_offset, object_offset + elem_count };
				binding_range.infoRange   = { image_offset, image_offset + elem_count };
				object_offset += elem_count;
				image_offset  += elem_count;
			} break;
			case DescriptorType::UniformTexelBuffer:
			case DescriptorType::StorageTexelBuffer: {
				binding_range.objectRange = { object_offset, object_offset + elem_count };
				binding_range.infoRange   = { buffer_view_offset, buffer_view_offset + elem_count };
				object_offset      += elem_count;
				buffer_view_offset += elem_count;
			} break;
			case DescriptorType::UniformBuffer:
			case DescriptorType::StorageBuffer:
			case DescriptorType::UniformBufferDynamic:
			case DescriptorType::StorageBufferDynamic: {
				if (binding->type == ReflectionNodeType::DescriptorBlock) {
					binding_range.blockRange = { block_offset, block_offset + elem_count };
					block_offset            += elem_count;
				}

				binding_range.objectRange = { object_offset, object_offset + elem_count };
				binding_range.infoRange   = { buffer_offset, buffer_offset + elem_count };
				object_offset += elem_count;
				buffer_offset += elem_count;
			} break;
			case DescriptorType::AccelerationStructure:
			case DescriptorType::AccelerationStructureNV: {
				binding_range.objectRange = { object_offset, object_offset + elem_count };
				binding_range.infoRange   = { as_offset, as_offset + elem_count };
				object_offset += elem_count;
				as_offset     += elem_count;
			} break;
			}

			new_set_state.bindingRanges.insert(std::make_pair(binding->binding, binding_range));
			new_set_state.objects.resize(object_offset);
			new_set_state.blockStorages.resize(block_offset);
			new_set_state.imageInfos.resize(image_offset);
			new_set_state.bufferInfos.resize(buffer_offset);
			new_set_state.bufferViewInfos.resize(buffer_view_offset);
			new_set_state.accelStructInfos.resize(as_offset);
		}

		auto  set_layout     = impl.pipelineLayout->getDescriptorSetLayout(set_idx);
		auto& layout_binding = set_layout->getBinding(set_idx);

		new_desc_set.allocateDescriptorSet(impl.descriptorPool, set_layout, elem_count);
		new_desc_set.stateIdRange = { 1, 2 };
	}

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
	return { &impl, impl.rootNode, nullptr, 0 };
}

void ShaderParameter::reset()
{
}

///////////////////////////////////////////////////////////////////////////////

void ShaderParameterDescriptorSet::allocateDescriptorSet(
	ref<DescriptorPool>      pool,
	ref<DescriptorSetLayout> layout,
	uint32_t                 elem_count
) {
	auto vk_device = get_vk_device(pool->getDevice());
	auto vk_pool   = get_vk_descriptor_pool(pool);
	auto flags     = layout->getBindings().back().flags;

	vk::DescriptorSetVariableDescriptorCountAllocateInfo var_count_info{};
	vk::DescriptorSetAllocateInfo                        alloc_info{};
	alloc_info.descriptorPool     = vk_pool;
	alloc_info.pSetLayouts        = &get_vk_descriptor_set_layout(layout);
	alloc_info.descriptorSetCount = 1;

	if (flags.has(DescriptorSetLayoutBindingFlagBits::VariableDescriptorCount)) {
		var_count_info.descriptorSetCount = 1;
		var_count_info.pDescriptorCounts  = &elem_count;
		alloc_info.pNext                  = &var_count_info;
	}

	auto result = vk_device.allocateDescriptorSets(&alloc_info, &descriptorSet);

	if (result != vk::Result::eSuccess)
		throw Exception("Failed to allocate descriptor sets for ShaderParameter");
}

///////////////////////////////////////////////////////////////////////////////

void vr::ShaderParameterSetState::writeSampler(obj<Sampler> sampler, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_range = bindingRanges.at(binding);

	VERA_ASSERT(array_idx < binding_range.infoRange.size());
	VERA_ASSERT(array_idx < binding_range.samplerRange.size());

	uint32_t info_idx    = binding_range.infoRange.first() + array_idx;
	uint32_t sampler_idx = binding_range.samplerRange.first() + array_idx;

	imageInfos[info_idx].sampler = get_vk_sampler(sampler);
	objects[sampler_idx]         = std::move(sampler);
}

void vr::ShaderParameterSetState::writeTextureView(obj<TextureView> texture_view, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_range = bindingRanges.at(binding);

	VERA_ASSERT(array_idx < binding_range.infoRange.size());
	VERA_ASSERT(array_idx < binding_range.objectRange.size());

	uint32_t info_idx   = binding_range.infoRange.first() + array_idx;
	uint32_t object_idx = binding_range.objectRange.first() + array_idx;

	imageInfos[info_idx].imageView = get_vk_image_view(texture_view);
	objects[object_idx]            = std::move(texture_view);
}

void vr::ShaderParameterSetState::writeCombinedTextureSampler(obj<Sampler> sampler, obj<TextureView> texture_view, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_range = bindingRanges.at(binding);

	VERA_ASSERT(array_idx < binding_range.infoRange.size());
	VERA_ASSERT(array_idx < binding_range.objectRange.size());

	uint32_t info_idx    = binding_range.infoRange.first() + array_idx;
	uint32_t sampler_idx = binding_range.samplerRange.first() + array_idx;
	uint32_t object_idx  = binding_range.objectRange.first() + array_idx;

	imageInfos[info_idx].sampler   = get_vk_sampler(sampler);
	imageInfos[info_idx].imageView = get_vk_image_view(texture_view);
	objects[sampler_idx]           = std::move(sampler);
	objects[object_idx]            = std::move(texture_view);
}

void vr::ShaderParameterSetState::writeBufferView(obj<BufferView> buffer_view, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_range = bindingRanges.at(binding);

	VERA_ASSERT(array_idx < binding_range.infoRange.size());
	VERA_ASSERT(array_idx < binding_range.objectRange.size());

	uint32_t info_idx   = binding_range.infoRange.first() + array_idx;
	uint32_t object_idx = binding_range.objectRange.first() + array_idx;

	bufferViewInfos[info_idx] = get_vk_buffer_view(buffer_view);
	objects[object_idx]       = std::move(buffer_view);
}

void vr::ShaderParameterSetState::writeBuffer(obj<Buffer> buffer, size_t offset, size_t range, uint32_t binding, uint32_t array_idx)
{
	const auto& binding_range = bindingRanges.at(binding);

	VERA_ASSERT(array_idx < binding_range.infoRange.size());
	VERA_ASSERT(array_idx < binding_range.objectRange.size());

	uint32_t info_idx   = binding_range.infoRange.first() + array_idx;
	uint32_t object_idx = binding_range.objectRange.first() + array_idx;

	bufferInfos[info_idx].buffer = get_vk_buffer(buffer);
	bufferInfos[info_idx].offset = offset;
	bufferInfos[info_idx].range  = range;
	objects[object_idx]          = std::move(buffer);
}

///////////////////////////////////////////////////////////////////////////////

void ShaderParameterImpl::bind(CommandBufferImpl& cmd_impl)
{

}

void ShaderParameterImpl::prepareDescriptorWrite(uint32_t set, uint32_t binding, uint32_t array_idx)
{

}

void ShaderParameterImpl::submitFrame(CommandBufferImpl& cmd_impl)
{

}

bool ShaderParameterImpl::checkStateLocked(uint64_t state_id) const
{
	return false;
}

VERA_NAMESPACE_END
