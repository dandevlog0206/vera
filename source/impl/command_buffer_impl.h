#pragma once

#include "object_impl.h"

#include "../../include/vera/core/command_buffer_sync.h"
#include "../../include/vera/graphics/graphics_state.h"

VERA_NAMESPACE_BEGIN

enum class SubmitQueueType VERA_ENUM
{
	Transfer,
	Compute,
	Graphics
};

struct ResourceSetBindingState
{

};

struct CommandBufferImpl
{
	using DescriptorSetState = std::vector<const_ref<DescriptorSet>>;
	using ShaderParameters   = std::vector<ShaderParameter*>;

	obj<Device>        device;
	obj<Semaphore>     semaphore;
	obj<Fence>         fence;

	vk::CommandPool    commandPool;
	vk::CommandBuffer  commandBuffer;

	uint64_t           submitID;
	SubmitQueueType    submitQueueType;
	CommandBufferState state;
	Viewport           currentViewport;
	Scissor            currentScissor;
	ref<Buffer>        currentVertexBuffer;
	ref<Buffer>        currentIndexBuffer;
	RenderingInfo      currentRenderingInfo;
	DescriptorSetState currentDescriptorSets;
	ref<Pipeline>      currentPipeline;
};

static vk::ResolveModeFlagBits to_vk_resolve_mode(ResolveMode mode)
{
	switch (mode) {
	case ResolveMode::None:       return vk::ResolveModeFlagBits::eNone;
	case ResolveMode::SampleZero: return vk::ResolveModeFlagBits::eSampleZero;
	case ResolveMode::Average:    return vk::ResolveModeFlagBits::eAverage;
	case ResolveMode::Min:        return vk::ResolveModeFlagBits::eMin;
	case ResolveMode::Max:        return vk::ResolveModeFlagBits::eMax;
	}

	VERA_ASSERT_MSG(false, "invalid resolve mode");
	return {};
}

static vk::AttachmentLoadOp to_vk_attachment_load_op(LoadOp op)
{
	switch (op) {
	case LoadOp::Load:     return vk::AttachmentLoadOp::eLoad;
	case LoadOp::DontCare: return vk::AttachmentLoadOp::eDontCare;
	case LoadOp::Clear:    return vk::AttachmentLoadOp::eClear;
	}

	VERA_ASSERT_MSG(false, "invalid load op");
	return {};
}

static vk::AttachmentStoreOp to_vk_attachment_store_op(StoreOp op)
{
	switch (op) {
	case StoreOp::Store:    return vk::AttachmentStoreOp::eStore;
	case StoreOp::DontCare: return vk::AttachmentStoreOp::eDontCare;
	}

	VERA_ASSERT_MSG(false, "invalid store op");
	return {};
}

VERA_NAMESPACE_END