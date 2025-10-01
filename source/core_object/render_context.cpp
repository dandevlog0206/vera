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
#include "../../include/vera/graphics/graphics_state.h"
#include "../../include/vera/shader/shader_parameter.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

static void append_render_frame(RenderContextImpl& impl, uint32_t at, uint64_t id)
{
	auto* render_frame = *impl.renderFrames.emplace(impl.renderFrames.cbegin() + at, new RenderFrame);

	render_frame->renderCommand           = CommandBuffer::create(impl.device);
	render_frame->framebuffers            = {};
	render_frame->renderCompleteSemaphore = Semaphore::create(impl.device);
	render_frame->fence                   = Fence::create(impl.device);
	render_frame->frameID                 = id;

	render_frame->renderCommand->begin();
}

static void reset_render_frame(RenderContextImpl& impl, RenderFrame& render_frame, uint64_t id)
{
	render_frame.renderCommand->reset();
	render_frame.framebuffers.clear();
	render_frame.fence->reset();
	render_frame.frameID = id;

	render_frame.renderCommand->begin();
}

static void render_context_next_frame(RenderContextImpl& impl)
{
	auto  next_idx   = (impl.frameIndex + 1) % static_cast<int32_t>(impl.renderFrames.size());
	auto& next_frame = *impl.renderFrames[next_idx];

	impl.currentFrameID++;

	if (next_frame.fence->signaled()) {
		reset_render_frame(impl, next_frame, impl.currentFrameID);
		impl.frameIndex = next_idx;
	} else {
		append_render_frame(impl, impl.frameIndex + 1, impl.currentFrameID);
		impl.frameIndex = impl.frameIndex + 1;
	}
}

static RenderFrame& get_render_frame(RenderContextImpl& impl)
{
	return *impl.renderFrames[impl.frameIndex];
}

FrameSync::FrameSync(const RenderFrame& ctx_impl, uint64_t frame_id) :
	m_render_frame(&ctx_impl), m_frame_id(frame_id) {}

void FrameSync::waitForRenderComplete() const
{
	VERA_ASSERT(m_render_frame);

	if (m_render_frame->frameID == m_frame_id)
		m_render_frame->fence->wait();
}

bool FrameSync::isRenderComplete() const
{
	return m_render_frame->frameID == m_frame_id ? m_render_frame->fence->signaled() : true;
}

ref<Semaphore> FrameSync::getRenderCompleteSemaphore() const
{
	VERA_ASSERT(m_render_frame);

	if (m_render_frame->frameID == m_frame_id)
		return m_render_frame->renderCompleteSemaphore;
	
	return {};
}

bool FrameSync::empty() const
{
	return !m_render_frame;
}

obj<RenderContext> RenderContext::create(obj<Device> device)
{
	auto  obj  = createNewObject<RenderContext>();
	auto& impl = getImpl(obj);

	impl.device         = std::move(device);
	impl.frameIndex     = 0;
	impl.currentFrameID = 0;

	append_render_frame(impl, 0, 0);

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

	return impl.renderFrames[impl.frameIndex]->renderCommand;
}

void RenderContext::transitionImageLayout(ref<Texture> texture, ImageLayout old_layout, ImageLayout new_layout)
{
	auto& impl     = getImpl(this);
	auto& cmd      = get_render_frame(impl).renderCommand;
	auto& cmd_impl = getImpl(cmd);

	if (!cmd_impl.currentRenderingInfo.colorAttachments.empty())
		cmd->endRendering();

	if (old_layout == ImageLayout::Undefined && new_layout == ImageLayout::AttachmentOptimal) {
		cmd->transitionImageLayout(
			texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::AccessFlagBits{},
			vr::AccessFlagBits::ColorAttachmentWrite,
			vr::ImageLayout::Undefined,
			vr::ImageLayout::ColorAttachmentOptimal);
	} else if (old_layout == ImageLayout::ColorAttachmentOptimal && new_layout == ImageLayout::PresentSrc) {
		cmd->transitionImageLayout(
			texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::BottomOfPipe,
			vr::AccessFlagBits::ColorAttachmentWrite | vr::AccessFlagBits::ColorAttachmentWrite,
			vr::AccessFlagBits{},
			vr::ImageLayout::ColorAttachmentOptimal,
			vr::ImageLayout::PresentSrc);
	} else {
		throw Exception("unsupported image layout transition");
	}
}

void RenderContext::draw(const GraphicsState& states, uint32_t vtx_count, uint32_t vtx_off)
{
	auto& impl     = getImpl(this);
	auto& cmd      = get_render_frame(impl).renderCommand;


	states.bindCommandBuffer(cmd);

	cmd->draw(vtx_count, 1, vtx_off, 0);
}

void RenderContext::draw(const GraphicsState& states, const ShaderParameter& params, uint32_t vtx_count, uint32_t vtx_off)
{
	auto& impl = getImpl(this);
	auto& cmd  = get_render_frame(impl).renderCommand;

	// TODO: verify image layout transition
	for (auto& color : states.getRenderingInfo().colorAttachments) {
		auto& texture_impl = getImpl(color.texture);

		// Identify swapchain image and save for future use
		if (texture_impl.imageUsage.has(ImageUsageFlagBits::FrameBuffer)) {
			auto& render_frame = *impl.renderFrames[impl.frameIndex];

			// TODO: optimize
			if (std::none_of(VERA_SPAN(render_frame.framebuffers),
				[=](const auto& elem) {
					return elem == texture_impl.frameBuffer;
				})) {
				auto& framebuffer_impl = getImpl(texture_impl.frameBuffer);
				
				render_frame.framebuffers.push_back(texture_impl.frameBuffer);
				framebuffer_impl.frameSync = FrameSync(render_frame, impl.currentFrameID);
			}
		}

		cmd->transitionImageLayout(
			color.texture,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::PipelineStageFlagBits::ColorAttachmentOutput,
			vr::AccessFlagBits{},
			vr::AccessFlagBits::ColorAttachmentWrite,
			vr::ImageLayout::Undefined,
			vr::ImageLayout::ColorAttachmentOptimal);
	}

	states.bindCommandBuffer(cmd);
	params.bindCommandBuffer(states.getPipeline()->getPipelineLayout(), cmd);

	cmd->draw(vtx_count, 1, vtx_off, 0);
}

void RenderContext::submit()
{
	static_vector<vk::Semaphore, 32>          semaphores;
	static_vector<vk::PipelineStageFlags, 32> stage_masks;

	auto& impl         = getImpl(this);
	auto& device_impl  = getImpl(impl.device);
	auto& render_frame = *impl.renderFrames[impl.frameIndex];
	auto& command_impl = getImpl(render_frame.renderCommand);

	for (auto& framebuffer : render_frame.framebuffers)
		transitionImageLayout(
			framebuffer->getTexture(),
			ImageLayout::ColorAttachmentOptimal,
			ImageLayout::PresentSrc);

	render_frame.renderCommand->end();

	vk::SubmitInfo submit_info;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &get_vk_command_buffer(render_frame.renderCommand);
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &get_vk_semaphore(render_frame.renderCompleteSemaphore);

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

	device_impl.graphicsQueue.submit(submit_info, get_vk_fence(render_frame.fence));

	render_context_next_frame(impl);
}

VERA_NAMESPACE_END