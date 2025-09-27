#include "../../include/vera/core/render_context.h"
#include "../impl/device_impl.h"
#include "../impl/render_command_impl.h"
#include "../impl/render_context_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/graphics_state.h"
#include "../../include/vera/core/semaphore.h"
#include "../../include/vera/shader/shader_parameter.h"

VERA_NAMESPACE_BEGIN

static void append_render_frame(RenderContextImpl& impl)
{
	auto& render_frame = impl.renderFrames.emplace_back();

	render_frame.renderCommand           = RenderCommand::create(impl.device);
	render_frame.fence                   = Fence::create(impl.device);
	render_frame.renderCompleteSemaphore = Semaphore::create(impl.device);
	render_frame.imageWaitSemaphore      = {};
	render_frame.swapchainImage          = {};
	render_frame.isBegin                 = true;
	render_frame.isSubmitted             = false;

	render_frame.renderCommand->begin();
}

static void reset_render_frame(RenderFrame& render_frame)
{
	render_frame.renderCommand->reset();
	render_frame.fence->reset();
	render_frame.imageWaitSemaphore = {};
	render_frame.isBegin            = true;
	render_frame.isSubmitted        = false;

	render_frame.renderCommand->begin();
}

static RenderFrame& get_render_frame(RenderContextImpl& impl)
{
	return impl.renderFrames[impl.frameIndex];
}

obj<RenderContext> RenderContext::create(obj<Device> device)
{
	auto  obj  = createNewObject<RenderContext>();
	auto& impl = getImpl(obj);

	impl.device          = std::move(device);
	impl.frameIndex      = 0;
	impl.lastFrameIndex  = -1;
	impl.totalFrameCount = 0;

	append_render_frame(impl);

	return obj;
}

RenderContext::~RenderContext()
{
	auto& impl = getImpl(this);

	destroyObjectImpl(this);
}

obj<Device> RenderContext::getDevice()
{
	return getImpl(this).device;
}

obj<RenderCommand> RenderContext::getRenderCommand()
{
	auto& impl = getImpl(this);

	return impl.renderFrames[impl.frameIndex].renderCommand;
}

void RenderContext::draw(const GraphicsState& states, uint32_t vtx_count, uint32_t vtx_off)
{
	auto& impl = getImpl(this);
	auto& cmd  = get_render_frame(impl).renderCommand;

	states.bindRenderCommand(cmd);

	cmd->draw(vtx_count, 1, vtx_off, 0);
}

void RenderContext::draw(const GraphicsState& states, const ShaderParameter& params, uint32_t vtx_count, uint32_t vtx_off)
{
	auto& impl = getImpl(this);
	auto& cmd  = get_render_frame(impl).renderCommand;

	// TODO: verify image layout transition
	for (auto& color : states.getRenderingInfo().colorAttachments) {
		// Identify swapchain image and save for future use
		if (auto& texture_impl = getImpl(color.texture); texture_impl.isSwapchainImage)
			impl.renderFrames[impl.frameIndex].swapchainImage = color.texture;

		cmd->transitionImageLayout(
			color.texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::AccessFlagBits{},
			vr::AccessFlagBits::ColorAttachmentWrite,
			vr::ImageLayout::Undefined,
			vr::ImageLayout::ColorAttachmentOptimal);
	}

	states.bindRenderCommand(cmd);
	
	if (!params.empty())
		params.bindRenderCommand(states.getPipeline()->getPipelineLayout(), cmd);

	cmd->draw(vtx_count, 1, vtx_off, 0);
}

void RenderContext::submit()
{
	static const vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	
	auto& impl         = getImpl(this);
	auto& device_impl  = getImpl(impl.device);
	auto& render_frame = impl.renderFrames[impl.frameIndex];
	auto& command_impl = getImpl(render_frame.renderCommand);
	auto  next_index   = (impl.frameIndex + 1) % static_cast<int32_t>(impl.renderFrames.size());
	auto& next_frame   = impl.renderFrames[next_index];

	if (!command_impl.currentRenderingInfo.colorAttachments.empty())
		render_frame.renderCommand->endRendering();

	if (render_frame.swapchainImage) {
		render_frame.renderCommand->transitionImageLayout(
			render_frame.swapchainImage,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::BottomOfPipe,
			vr::AccessFlagBits::ColorAttachmentWrite | vr::AccessFlagBits::ColorAttachmentWrite,
			vr::AccessFlagBits{},
			vr::ImageLayout::ColorAttachmentOptimal,
			vr::ImageLayout::PresentSrc);
	}

	render_frame.renderCommand->end();
	render_frame.isSubmitted = true;

	vk::SubmitInfo submit_info;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &get_vk_command_buffer(render_frame.renderCommand);
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &get_vk_semaphore(render_frame.renderCompleteSemaphore);

	if (render_frame.imageWaitSemaphore) {
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores    = &get_vk_semaphore(render_frame.imageWaitSemaphore);
		submit_info.pWaitDstStageMask  = &wait_stage;
	}

	device_impl.graphicsQueue.submit(submit_info, get_vk_fence(render_frame.fence));

	if (next_frame.fence->signaled()) {
		reset_render_frame(next_frame);
		impl.lastFrameIndex = std::exchange(impl.frameIndex, next_index);
	} else {
		append_render_frame(impl);
		impl.lastFrameIndex = std::exchange(impl.frameIndex, static_cast<int32_t>(impl.renderFrames.size() - 1));
	}

	impl.totalFrameCount++;
}

VERA_NAMESPACE_END