#include "../../include/vera/core/render_context.h"
#include "../impl/device_impl.h"
#include "../impl/command_buffer_impl.h"
#include "../impl/framebuffer_impl.h"
#include "../impl/render_context_impl.h"
#include "../impl/texture_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/framebuffer.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/semaphore.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/shader_parameter.h"
#include "../../include/vera/graphics/graphics_state.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

static void append_render_frame(RenderContextImpl& impl, uint32_t at, uint64_t id)
{
	auto* render_frame = *impl.renderFrames.emplace(impl.renderFrames.cbegin() + at, new RenderContextFrame);

	render_frame->commandBuffer     = CommandBuffer::create(impl.device);
	render_frame->framebuffers      = {};
	render_frame->commandBufferSync = render_frame->commandBuffer->getSync();
	render_frame->frameID           = id;

	render_frame->commandBuffer->begin();
}

static void reset_render_frame(RenderContextImpl& impl, RenderContextFrame& render_frame, uint64_t id)
{
	render_frame.commandBuffer->reset();
	render_frame.framebuffers.clear();
	render_frame.commandBufferSync = render_frame.commandBuffer->getSync();
	render_frame.frameID           = id;

	render_frame.commandBuffer->begin();
}

static void render_context_next_frame(RenderContextImpl& impl)
{
	auto  next_idx   = (impl.frameIndex + 1) % static_cast<int32_t>(impl.renderFrames.size());
	auto& next_frame = *impl.renderFrames[next_idx];

	impl.currentFrameID++;

	if (next_frame.commandBufferSync.isComplete()) {
		reset_render_frame(impl, next_frame, impl.currentFrameID);
		impl.frameIndex = next_idx;
	} else if (impl.dynamicFrameCount) {
		append_render_frame(impl, impl.frameIndex + 1, impl.currentFrameID);
		impl.frameIndex = impl.frameIndex + 1;
	} else {
		next_frame.commandBufferSync.waitForComplete();

		reset_render_frame(impl, next_frame, impl.currentFrameID);
		impl.frameIndex = next_idx;
	}
}

static RenderContextFrame& get_current_frame(RenderContextImpl& impl)
{
	return *impl.renderFrames[impl.frameIndex];
}

obj<RenderContext> RenderContext::create(obj<Device> device, const RenderContextCreateInfo& info)
{
	auto  obj  = createNewCoreObject<RenderContext>();
	auto& impl = getImpl(obj);

	impl.device            = std::move(device);
	impl.frameIndex        = 0;
	impl.currentFrameID    = 0;
	impl.dynamicFrameCount = info.dynamicFrameCount;

	for (uint32_t i = 0; i < info.frameCount; ++i)
		append_render_frame(impl, i, i);

	return obj;
}

RenderContext::~RenderContext()
{
	auto& impl = getImpl(this);

	for (auto* frame_ptr : impl.renderFrames)
		delete frame_ptr;

	destroyObjectImpl(this);
}

obj<Device> RenderContext::getDevice()
{
	return getImpl(this).device;
}

obj<CommandBuffer> RenderContext::getRenderCommand()
{
	auto& impl = getImpl(this);

	return impl.renderFrames[impl.frameIndex]->commandBuffer;
}

uint32_t RenderContext::getCurrentFrameIndex() const
{
	return getImpl(this).frameIndex;
}

VERA_NODISCARD uint32_t RenderContext::getFrameCount() const
{
	return static_cast<uint32_t>(getImpl(this).renderFrames.size());
}

VERA_NODISCARD const RenderFrame& RenderContext::getCurrentFrame() const
{
	auto& impl = getImpl(this);

	return *impl.renderFrames[impl.frameIndex];
}

CommandBufferSync RenderContext::getCommandBufferSync() const
{
	auto& impl = getImpl(this);

	return impl.renderFrames[impl.frameIndex]->commandBufferSync;
}

void RenderContext::transitionImageLayout(ref<Texture> texture, TextureLayout old_layout, TextureLayout new_layout)
{
	auto& impl     = getImpl(this);
	auto& cmd      = get_current_frame(impl).commandBuffer;
	auto& cmd_impl = getImpl(cmd);

	if (!cmd_impl.currentRenderingInfo.colorAttachments.empty())
		cmd->endRendering();

	if (old_layout == TextureLayout::Undefined && new_layout == TextureLayout::AttachmentOptimal) {
		cmd->transitionImageLayout(
			texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::AccessFlagBits{},
			vr::AccessFlagBits::ColorAttachmentWrite,
			vr::TextureLayout::Undefined,
			vr::TextureLayout::ColorAttachmentOptimal);
	} else if (old_layout == TextureLayout::ColorAttachmentOptimal && new_layout == TextureLayout::PresentSrc) {
		cmd->transitionImageLayout(
			texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::BottomOfPipe,
			vr::AccessFlagBits::ColorAttachmentWrite | vr::AccessFlagBits::ColorAttachmentWrite,
			vr::AccessFlagBits{},
			vr::TextureLayout::ColorAttachmentOptimal,
			vr::TextureLayout::PresentSrc);
	} else {
		throw Exception("unsupported image layout transition");
	}
}

void RenderContext::draw(const GraphicsState& states, uint32_t vtx_count, uint32_t vtx_off)
{
	auto& impl     = getImpl(this);
	auto& cmd      = get_current_frame(impl).commandBuffer;


	cmd->bindGraphicsState(states);

	cmd->draw(vtx_count, 1, vtx_off, 0);
}

void RenderContext::draw(
	const GraphicsState& states,
	obj<ShaderParameter> param,
	uint32_t             vtx_count,
	uint32_t             vtx_off
) {
	auto& impl = getImpl(this);
	auto& cmd  = get_current_frame(impl).commandBuffer;

	// TODO: verify image layout transition
	for (auto& color : states.getRenderingInfo().colorAttachments) {
		auto& texture_impl = getImpl(color.texture);

		// Identify swapchain image and save for future use
		if (texture_impl.textureUsage.has(TextureUsageFlagBits::FrameBuffer)) {
			auto& render_frame = *impl.renderFrames[impl.frameIndex];

			// TODO: optimize
			if (std::none_of(VERA_SPAN(render_frame.framebuffers),
				[=](const auto& elem) {
					return elem == texture_impl.frameBuffer;
				})) {
				auto& framebuffer_impl = getImpl(texture_impl.frameBuffer);
				
				render_frame.framebuffers.push_back(texture_impl.frameBuffer);
				framebuffer_impl.commandBufferSync = render_frame.commandBufferSync;
			}
		}

		cmd->transitionImageLayout(
			color.texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::AccessFlagBits{},
			vr::AccessFlagBits::ColorAttachmentWrite,
			vr::TextureLayout::Undefined,
			vr::TextureLayout::ColorAttachmentOptimal);
	}

	cmd->bindGraphicsState(states);
	// cmd->bindShaderParameter(param);

	cmd->draw(vtx_count, 1, vtx_off, 0);
}

void RenderContext::drawIndexed(
	const GraphicsState& states,
	obj<ShaderParameter> param,
	uint32_t             idx_count,
	uint32_t             idx_off,
	uint32_t             vtx_off
) {
	auto& impl = getImpl(this);
	auto& cmd  = get_current_frame(impl).commandBuffer;

	// TODO: verify image layout transition
	for (auto& color : states.getRenderingInfo().colorAttachments) {
		auto& texture_impl = getImpl(color.texture);

		// Identify swapchain image and save for future use
		if (texture_impl.textureUsage.has(TextureUsageFlagBits::FrameBuffer)) {
			auto& render_frame = *impl.renderFrames[impl.frameIndex];

			// TODO: optimize
			if (std::none_of(VERA_SPAN(render_frame.framebuffers),
				[=](const auto& elem) {
					return elem == texture_impl.frameBuffer;
				})) {
				auto& framebuffer_impl = getImpl(texture_impl.frameBuffer);
				
				render_frame.framebuffers.push_back(texture_impl.frameBuffer);
				framebuffer_impl.commandBufferSync = render_frame.commandBufferSync;
			}
		}

		cmd->transitionImageLayout(
			color.texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::AccessFlagBits{},
			vr::AccessFlagBits::ColorAttachmentWrite,
			vr::TextureLayout::Undefined,
			vr::TextureLayout::ColorAttachmentOptimal);
	}

	cmd->bindGraphicsState(states);
	cmd->bindShaderParameter(param);

	cmd->drawIndexed(idx_count, 1, idx_off, vtx_off, 0);
}

void RenderContext::submit()
{
	auto& impl         = getImpl(this);
	auto& device_impl  = getImpl(impl.device);
	auto& render_frame = *impl.renderFrames[impl.frameIndex];
	auto& cmd_impl     = getImpl(render_frame.commandBuffer);

	for (auto& framebuffer : render_frame.framebuffers)
		transitionImageLayout(
			framebuffer->getTexture(),
			TextureLayout::ColorAttachmentOptimal,
			TextureLayout::PresentSrc);

	render_frame.commandBuffer->end();

	cmd_impl.state = CommandBufferState::Submitted;

	vk::TimelineSemaphoreSubmitInfo timeline_info;
	timeline_info.signalSemaphoreValueCount = 1;
	timeline_info.pSignalSemaphoreValues    = &cmd_impl.submitID;

	vk::SubmitInfo submit_info;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &cmd_impl.commandBuffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &get_vk_semaphore(cmd_impl.semaphore);
	submit_info.pNext                = &timeline_info;

	static_vector<vk::Semaphore, 32>          semaphores;
	static_vector<vk::PipelineStageFlags, 32> stage_masks;

	if (!render_frame.framebuffers.empty()) {
		for (auto& framebuffer : render_frame.framebuffers) {
			auto& framebuffer_impl = getImpl(framebuffer);

			if (!framebuffer_impl.waitSemaphore) continue;

			semaphores.push_back(get_vk_semaphore(framebuffer_impl.waitSemaphore));
			stage_masks.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		}

		submit_info.waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
		submit_info.pWaitSemaphores    = semaphores.data();
		submit_info.pWaitDstStageMask  = stage_masks.data();
	}

	device_impl.graphicsQueue.submit(submit_info, get_vk_fence(cmd_impl.fence));

	render_context_next_frame(impl);
}

VERA_NAMESPACE_END