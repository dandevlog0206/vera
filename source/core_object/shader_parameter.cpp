#include "../../include/vera/core/shader_parameter.h"
#include "../impl/shader_parameter_impl.h"
#include "../impl/pipeline_layout_impl.h"

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

obj<ShaderParameter> ShaderParameter::create(
	obj<Device>            device,
	obj<ProgramReflection> program_reflection,
	obj<DescriptorPool>    descriptor_pool)
{
	auto  obj  = createNewCoreObject<ShaderParameter>();
	auto& impl = getImpl(obj);

	impl.device         = std::move(device);
	impl.pipelineLayout = PipelineLayout::create(impl.device, program_reflection);
	impl.descriptorPool = descriptor_pool;

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
