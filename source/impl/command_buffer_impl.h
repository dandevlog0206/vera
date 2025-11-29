#pragma once

#include "object_impl.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/command_sync.h"

VERA_NAMESPACE_BEGIN

enum class SubmitQueueType VERA_ENUM
{
	Transfer,
	Compute,
	Graphics
};

struct CommandBufferTracker
{
	obj<Semaphore>     semaphore = {};
	obj<Fence>         fence     = {};
	CommandBufferState state     = CommandBufferState::Invalid;
	uint64_t           submitID  = 0;
};

class CommandBufferImpl
{
public:
	using DescriptorSetState = std::vector<cref<DescriptorSet>>;
	using Tracker            = std::shared_ptr<CommandBufferTracker>;

	obj<Device>        device                = {};

	vk::CommandPool    vkCommandPool         = {};
	vk::CommandBuffer  vkCommandBuffer       = {};

	SubmitQueueType    submitQueueType       = {};
	Tracker            tracker               = {};
	Viewport           currentViewport       = {};
	Scissor            currentScissor        = {};
	cref<Buffer>       currentVertexBuffer   = {};
	cref<Buffer>       currentIndexBuffer    = {};
	RenderingInfo      currentRenderingInfo  = {};
	DescriptorSetState currentDescriptorSets = {};
	cref<Pipeline>     currentPipeline       = {};

	void submitToDedicatedQueue(const vk::SubmitInfo& submit_info);
};

VERA_NAMESPACE_END