#include "../../include/vera/core/shader_parameter.h"
#include "../impl/shader_parameter_impl.h"
#include "../impl/pipeline_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/core/descriptor_pool.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

//static uint32_t get_element_count(const ReflectionDescriptorNode* node)
//{
//	if (node->getType() == ReflectionType::DescriptorArray) {
//		uint32_t elem_count = node->getElementCount();
//		return elem_count == UINT32_MAX ? 4096 : elem_count;
//	}
//
//	return 1;
//}

obj<ShaderParameter> ShaderParameter::create(obj<PipelineLayout> pipeline_layout, obj<DescriptorPool> descriptor_pool)
{
	return nullptr;

	//auto  obj         = createNewCoreObject<ShaderParameter>();
	//auto& impl        = getImpl(obj);
	//auto& layout_impl = getImpl(pipeline_layout);
	//auto* root_node   = get_reflection_root_node(layout_impl.shaderReflection);

	//impl.device          = layout_impl.device;
	//impl.pipelineLayout  = std::move(pipeline_layout);
	//impl.descriptorPool  = std::move(descriptor_pool);
	//impl.currentStateID  = 1;
	//impl.completeStateID = 0;

	//if (root_node->getSetCount() == 0) return obj;

	//if (!impl.descriptorPool)
	//	impl.descriptorPool = DescriptorPool::create(impl.device);

	//uint32_t set_count          = root_node->getSetCount();
	//uint32_t image_info_offset  = 0;
	//uint32_t buffer_info_offset = 0;
	//uint32_t buffer_view_offset = 0;

	//impl.setStates.reserve(static_cast<size_t>(set_count));

	//for (uint32_t set_id = 0; set_id < set_count; ++set_id) {
	//	auto& set_state = impl.setStates.emplace_back();

	//	for (const auto* binding : root_node->enumerateDescriptorSet(set_id)) {
	//		auto& binding_state = set_state.bindingStates[binding->binding];

	//		switch (binding->descriptorType) {
	//		case DescriptorType::Sampler:
	//		case DescriptorType::CombinedTextureSampler:
	//		case DescriptorType::SampledTexture:
	//		case DescriptorType::InputAttachment: {
	//			uint32_t range_first = image_info_offset;
	//			image_info_offset += get_element_count(binding);
	//			binding_state.imageInfoRange = basic_range(range_first, image_info_offset);
	//		} break;
	//		case DescriptorType::UniformTexelBuffer:
	//		case DescriptorType::StorageTexelBuffer: {
	//			uint32_t range_first = buffer_view_offset;
	//			buffer_view_offset += get_element_count(binding);
	//			binding_state.bufferViewRange = basic_range(range_first, buffer_view_offset);
	//		} break;
	//		case DescriptorType::UniformBuffer:
	//		case DescriptorType::UniformBufferDynamic:
	//		case DescriptorType::StorageBuffer:
	//		case DescriptorType::StorageBufferDynamic: {
	//			uint32_t range_first = buffer_info_offset;
	//			buffer_info_offset += get_element_count(binding);
	//			binding_state.bufferInfoRange = basic_range(range_first, buffer_info_offset);
	//		} break;
	//		}
	//	}

	//	const auto& last_node = root_node->enumerateDescriptorSet(set_id).back();

	//	auto& set_frame = set_state.descriptorSetFrames.emplace_back();
	//	set_frame.descriptorSet = impl.descriptorPool->allocate(
	//		impl.pipelineLayout->getDescriptorSetLayout(0),
	//		get_element_count(last_node)
	//	);
	//	set_frame.stateIDRange = basic_range<uint64_t>(1, 2);
	//}

	//impl.imageInfos.resize(image_info_offset);
	//impl.bufferInfos.resize(buffer_info_offset);
	//impl.bufferViews.resize(buffer_view_offset);

	//return obj;
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
	return {};
}

void ShaderParameter::reset()
{
}

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
