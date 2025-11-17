#pragma once

#include "object_impl.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/command_buffer_sync.h"

VERA_NAMESPACE_BEGIN

enum class SubmitQueueType VERA_ENUM
{
	Transfer,
	Compute,
	Graphics
};

class CommandBufferImpl
{
public:
	using DescriptorSetState = std::vector<const_ref<DescriptorSet>>;
	using ObjectBindingArray = std::vector<obj<CoreObject>>;
	using ShaderParameterSet = std::vector<obj<ShaderParameter>>;

	obj<Device>         device                = {};
	obj<Semaphore>      semaphore             = {};
	obj<Fence>          fence                 = {};

	vk::CommandPool     vkCommandPool         = {};
	vk::CommandBuffer   vkCommandBuffer       = {};

	uint64_t            submitID              = {};
	SubmitQueueType     submitQueueType       = {};
	CommandBufferState  state                 = {};
	ObjectBindingArray  boundObjects          = {};
	ShaderParameterSet  boundShaderParameters = {};
	Viewport            currentViewport       = {};
	Scissor             currentScissor        = {};
	const_ref<Buffer>   currentVertexBuffer   = {};
	const_ref<Buffer>   currentIndexBuffer    = {};
	RenderingInfo       currentRenderingInfo  = {};
	DescriptorSetState  currentDescriptorSets = {};
	const_ref<Pipeline> currentPipeline       = {};
};

VERA_NAMESPACE_END