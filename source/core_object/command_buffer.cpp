#include "../../include/vera/core/command_buffer.h"
#include "../impl/device_impl.h"
#include "../impl/descriptor_set_layout_impl.h"
#include "../impl/descriptor_set_impl.h"
#include "../impl/buffer_impl.h"
#include "../impl/command_buffer_impl.h"
#include "../impl/pipeline_impl.h"
#include "../impl/pipeline_layout_impl.h"
#include "../impl/texture_impl.h"
#include "../impl/device_memory_impl.h"
#include "../impl/shader_parameter_impl.h"

#include "../../include/vera/core/context.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/semaphore.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/core/shader_parameter.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/graphics/graphics_state.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

template <class ClearType>
bool operator==(const AttachmentInfo<ClearType>& lhs, const AttachmentInfo<ClearType>& rhs)
{
	return !memcmp(&lhs, &rhs, sizeof(lhs));
}

static bool operator==(const RenderingInfo& lhs, const RenderingInfo& rhs)
{
	if (&lhs == &rhs) return true;

	if (lhs.renderArea != rhs.renderArea ||
		lhs.layerCount != rhs.layerCount ||
		lhs.colorAttachments.size() != rhs.colorAttachments.size()) return false;

	auto* colorl_ptr = lhs.colorAttachments.data();
	auto* colorr_ptr = rhs.colorAttachments.data();
	auto  color_size = sizeof(AttachmentInfo<Color>) * lhs.colorAttachments.size();
	if (memcmp(colorl_ptr, colorr_ptr, color_size)) return false;

	auto& depthl = lhs.depthAttachment;
	auto& depthr = rhs.depthAttachment;
	if (depthl.has_value() != depthr.has_value()) return false;
	if (depthl.has_value() && depthl.value() == depthr.value()) return false;

	auto& stencill = lhs.stencilAttachment;
	auto& stencilr = rhs.stencilAttachment;
	if (stencill.has_value() != stencilr.has_value()) return false;
	if (stencill.has_value() && stencill.value() == stencilr.value()) return false;

	return true;
}

static bool check_command_buffer_in_use(const CommandBufferImpl& impl)
{
	const auto& tracker = *impl.tracker;
	return tracker.state == CommandBufferState::Pending && !tracker.fence->signaled();
}

static vk::ImageView get_vk_image_view(ref<Texture> texture)
{
	return get_vk_image_view(texture->getTextureView());
}

const vk::CommandBuffer& get_vk_command_buffer(cref<CommandBuffer> cmd_buffer) VERA_NOEXCEPT
{
	return CoreObject::getImpl(cmd_buffer).vkCommandBuffer;
}

vk::CommandBuffer& get_vk_command_buffer(ref<CommandBuffer> cmd_buffer) VERA_NOEXCEPT
{
	return CoreObject::getImpl(cmd_buffer).vkCommandBuffer;
}

obj<CommandBuffer> CommandBuffer::create(obj<Device> device)
{
	auto  obj       = createNewCoreObject<CommandBuffer>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	vk::CommandPoolCreateInfo pool_info;
	pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	vk::CommandBufferAllocateInfo alloc_info;
	alloc_info.commandPool        = vk_device.createCommandPool(pool_info);
	alloc_info.level              = vk::CommandBufferLevel::ePrimary;
	alloc_info.commandBufferCount = 1;

	impl.device                = device;
	impl.vkCommandPool         = alloc_info.commandPool;
	impl.vkCommandBuffer       = vk_device.allocateCommandBuffers(alloc_info).front();
	impl.submitQueueType       = SubmitQueueType::Transfer;
	impl.tracker               = std::make_shared<CommandBufferTracker>();
	impl.currentViewport       = {};
	impl.currentScissor        = {};
	impl.currentVertexBuffer   = {};
	impl.currentIndexBuffer    = {};
	impl.currentRenderingInfo  = {};
	impl.currentDescriptorSets = {};
	impl.currentPipeline       = {};

	impl.tracker->semaphore = Semaphore::create(impl.device);
	impl.tracker->fence     = Fence::create(impl.device);
	impl.tracker->state     = CommandBufferState::Initial;
	impl.tracker->submitID  = 0;

	return obj;
}

CommandBuffer::~CommandBuffer() VERA_NOEXCEPT
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	// TODO: use CommandSync later
	vk_device.waitIdle();

	impl.tracker->state = CommandBufferState::Invalid;

	vk_device.freeCommandBuffers(impl.vkCommandPool, impl.vkCommandBuffer);
	vk_device.destroy(impl.vkCommandPool);

	destroyObjectImpl(this);
}

obj<Device> CommandBuffer::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

CommandSync CommandBuffer::getSync() const VERA_NOEXCEPT
{
	auto& impl = getImpl(this);
	return CommandSync(impl.tracker, impl.tracker->submitID);
}

void CommandBuffer::reset()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	if (check_command_buffer_in_use(impl))
		throw Exception("cannot reset a submitted command buffer that is not completed");

	impl.tracker->fence->reset();
	impl.tracker->state     = CommandBufferState::Initial;
	impl.tracker->submitID += 1;

	impl.submitQueueType       = SubmitQueueType::Transfer;
	impl.currentViewport       = {};
	impl.currentScissor        = {};
	impl.currentVertexBuffer   = {};
	impl.currentIndexBuffer    = {};
	impl.currentRenderingInfo  = {};
	impl.currentDescriptorSets = {};
	impl.currentPipeline       = {};

	vk_device.resetCommandPool(impl.vkCommandPool);
}

void CommandBuffer::begin()
{
	auto& impl = getImpl(this);

	impl.tracker->state = CommandBufferState::Recording;

	vk::CommandBufferBeginInfo begin_info;
	begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	impl.vkCommandBuffer.begin(begin_info);
}

void CommandBuffer::copyBufferToTexture(
	ref<Texture> dst,
	ref<Buffer>  src,
	size_t       buffer_offset,
	uint32_t     buffer_row_length,
	uint32_t     buffer_image_height,
	uint3        image_offset,
	extent3d     image_extent
) {
	auto& impl         = getImpl(this);
	auto& texture_impl = getImpl(dst);
	auto& buffer_impl  = getImpl(src);

	vk::BufferImageCopy copy_info;
	copy_info.bufferOffset                    = buffer_offset;
	copy_info.bufferRowLength                 = buffer_row_length;
	copy_info.bufferImageHeight               = buffer_image_height;
	copy_info.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
	copy_info.imageSubresource.mipLevel       = 0;
	copy_info.imageSubresource.baseArrayLayer = 0;
	copy_info.imageSubresource.layerCount     = 1;
	copy_info.imageOffset.x                   = image_offset.x;
	copy_info.imageOffset.y                   = image_offset.y;
	copy_info.imageOffset.z                   = image_offset.z;
	copy_info.imageExtent.width               = image_extent.width;
	copy_info.imageExtent.height              = image_extent.height;
	copy_info.imageExtent.depth               = image_extent.depth;

	impl.vkCommandBuffer.copyBufferToImage(
		buffer_impl.vkBuffer,
		texture_impl.vkImage,
		vk::ImageLayout::eTransferDstOptimal,
		copy_info);
}

void CommandBuffer::transitionImageLayout(
	ref<Texture>       texture,
	PipelineStageFlags src_stage_mask,
	PipelineStageFlags dst_stage_mask,
	AccessFlags        src_access_mask,
	AccessFlags        dst_access_mask,
	TextureLayout      old_layout,
	TextureLayout      new_layout)
{
	auto& impl         = getImpl(this);
	auto& texture_impl = getImpl(texture);

	vk::ImageMemoryBarrier barrier;
	barrier.srcAccessMask                   = to_vk_access_flags(src_access_mask);
	barrier.dstAccessMask                   = to_vk_access_flags(dst_access_mask);
	barrier.oldLayout                       = to_vk_image_layout(old_layout);
	barrier.newLayout                       = to_vk_image_layout(new_layout);
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = get_vk_image(texture);
	barrier.subresourceRange.aspectMask     = to_vk_image_aspect_flags(texture_impl.textureAspect);
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;

	impl.vkCommandBuffer.pipelineBarrier(
		to_vk_pipeline_stage_flags(src_stage_mask),
		to_vk_pipeline_stage_flags(dst_stage_mask),
		vk::DependencyFlagBits{},
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier);
}

void CommandBuffer::setViewport(const Viewport& viewport)
{
	auto& impl = getImpl(this);

	vk::Viewport vk_viewport;
	vk_viewport.x        = viewport.posX;
	vk_viewport.y        = viewport.posY;
	vk_viewport.width    = viewport.width;
	vk_viewport.height   = viewport.height;
	vk_viewport.minDepth = viewport.minDepth;
	vk_viewport.maxDepth = viewport.maxDepth;

	impl.vkCommandBuffer.setViewport(0, vk_viewport);
	impl.currentViewport = viewport;
}

void CommandBuffer::setScissor(const Scissor& scissor)
{
	auto& impl = getImpl(this);

	vk::Rect2D vk_rect;
	vk_rect.offset.x      = scissor.minX;
	vk_rect.offset.y      = scissor.minY;
	vk_rect.extent.width  = scissor.maxX;
	vk_rect.extent.height = scissor.maxY;

	impl.vkCommandBuffer.setScissor(0, vk_rect);
	impl.currentScissor = scissor;
}

void CommandBuffer::bindVertexBuffer(cref<Buffer> buffer, size_t offset)
{
	auto& impl        = getImpl(this);
	auto& buffer_impl = getImpl(buffer);
	auto  offsets     = vk::DeviceSize{ offset };

	if (!buffer_impl.usage.has(BufferUsageFlagBits::VertexBuffer))
		throw Exception("buffer is not for vertex");

	impl.vkCommandBuffer.bindVertexBuffers(0, 1, &buffer_impl.vkBuffer, &offsets);
	impl.currentVertexBuffer = buffer;
}

void CommandBuffer::bindIndexBuffer(cref<Buffer> buffer, size_t offset)
{
	auto& impl        = getImpl(this);
	auto& buffer_impl = getImpl(buffer);

	if (!buffer_impl.usage.has(BufferUsageFlagBits::IndexBuffer))
		throw Exception("buffer is not for index");

	impl.vkCommandBuffer.bindIndexBuffer(buffer_impl.vkBuffer, offset, to_vk_index_type(buffer_impl.indexType));
	impl.currentIndexBuffer = buffer;
}

void CommandBuffer::bindPipeline(cref<Pipeline> pipeline)
{
	auto& impl          = getImpl(this);
	auto& pipeline_impl = getImpl(pipeline);

	impl.vkCommandBuffer.bindPipeline(
		to_vk_pipeline_bind_point(pipeline_impl.pipelineBindPoint),
		pipeline_impl.vkPipeline);
	impl.currentPipeline = pipeline;
}

void CommandBuffer::pushConstant(
	cref<PipelineLayout> pipeline_layout,
	ShaderStageFlags          stage_flags,
	uint32_t                  offset,
	const void*               data,
	uint32_t                  size)
{
	auto& impl = getImpl(this);
	impl.vkCommandBuffer.pushConstants(
		get_vk_pipeline_layout(pipeline_layout),
		to_vk_shader_stage_flags(stage_flags),
		offset,
		size,
		data);
}

void CommandBuffer::bindDescriptorSet(
	cref<PipelineLayout> pipeline_layout,
	uint32_t             set,
	cref<DescriptorSet>  desc_set
) {
	auto& impl = getImpl(this);

	impl.vkCommandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		get_vk_pipeline_layout(pipeline_layout),
		set,
		1,
		&get_vk_descriptor_set(desc_set),
		0,
		nullptr);
}

void CommandBuffer::bindDescriptorSet(
	cref<PipelineLayout> pipeline_layout,
	uint32_t             set,
	cref<DescriptorSet>  desc_set,
	array_view<uint32_t> dynamic_offsets
) {
	VERA_ASSERT_MSG(pipeline_layout, "pipeline layout is null");
	
	auto& impl = getImpl(this);

	impl.vkCommandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		get_vk_pipeline_layout(pipeline_layout),
		set,
		1,
		&get_vk_descriptor_set(desc_set),
		static_cast<uint32_t>(dynamic_offsets.size()),
		dynamic_offsets.data());
}

void CommandBuffer::bindGraphicsState(const GraphicsState& state)
{
	auto& impl   = getImpl(this);
	auto  vk_cmd = impl.vkCommandBuffer;

	if (state.m_viewport.posX != INFINITY)
		setViewport(state.m_viewport);

	if (state.m_scissor.minX != UINT32_MAX)
		setScissor(state.m_scissor);

	if (state.m_vertex_buffer)
		bindVertexBuffer(state.m_vertex_buffer);

	if (state.m_index_buffer)
		bindIndexBuffer(state.m_index_buffer);

	if (state.m_pipeline)
		bindPipeline(state.m_pipeline);

	if (!state.m_rendering_info.colorAttachments.empty()) {
		if (impl.currentRenderingInfo.colorAttachments.empty()) {
			beginRendering(state.m_rendering_info);
		} else if (!(state.m_rendering_info == impl.currentRenderingInfo)) {
			endRendering();
			beginRendering(state.m_rendering_info);
		}
	}
}

void CommandBuffer::bindShaderParameter(ref<ShaderParameter> params)
{
	VERA_ASSERT_MSG(params, "shader parameter is null");
	getImpl(params).bind(this);
}

void CommandBuffer::beginRendering(const RenderingInfo& info)
{
	auto& impl = getImpl(this);

	impl.submitQueueType = SubmitQueueType::Graphics;

	static_vector<vk::RenderingAttachmentInfo, 16> color_attachments;
	for (const auto& color_info : info.colorAttachments) {
		vk::ClearColorValue clear_value;
		clear_value.float32[0] = static_cast<float>(color_info.clearValue.r) / 255.f;
		clear_value.float32[1] = static_cast<float>(color_info.clearValue.g) / 255.f;
		clear_value.float32[2] = static_cast<float>(color_info.clearValue.b) / 255.f;
		clear_value.float32[3] = static_cast<float>(color_info.clearValue.a) / 255.f;

		auto& color_attachment = color_attachments.emplace_back();
		color_attachment.imageView   = get_vk_image_view(color_info.texture);
		color_attachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		color_attachment.resolveMode = to_vk_resolve_mode_flag_bits(color_info.resolveMode);
		color_attachment.loadOp      = to_vk_attachment_load_op(color_info.loadOp);
		color_attachment.storeOp     = to_vk_attachment_store_op(color_info.storeOp);
		color_attachment.clearValue  = clear_value;

		if (color_info.resolveTexture) {
			color_attachment.resolveImageView   = get_vk_image_view(color_info.resolveTexture);
			color_attachment.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		}
	}

	vk::RenderingAttachmentInfo depth_attachment;
	if (info.depthAttachment) {
		auto& depth_info = *info.depthAttachment;

		depth_attachment.imageView   = get_vk_image_view(depth_info.texture);
		depth_attachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment.resolveMode = to_vk_resolve_mode_flag_bits(depth_info.resolveMode);
		depth_attachment.loadOp      = to_vk_attachment_load_op(depth_info.loadOp);
		depth_attachment.storeOp     = to_vk_attachment_store_op(depth_info.storeOp);
		depth_attachment.clearValue  = vk::ClearDepthStencilValue(depth_info.clearValue, 0);

		if (depth_info.resolveTexture) {
			depth_attachment.resolveImageView   = get_vk_image_view(depth_info.resolveTexture);
			depth_attachment.resolveImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		}
	}

	vk::RenderingAttachmentInfo stencil_attachment;
	if (info.stencilAttachment) {
		auto& stencil_info = *info.stencilAttachment;

		stencil_attachment.imageView   = get_vk_image_view(stencil_info.texture);
		stencil_attachment.imageLayout = vk::ImageLayout::eStencilAttachmentOptimal;
		stencil_attachment.resolveMode = to_vk_resolve_mode_flag_bits(stencil_info.resolveMode);
		stencil_attachment.loadOp      = to_vk_attachment_load_op(stencil_info.loadOp);
		stencil_attachment.storeOp     = to_vk_attachment_store_op(stencil_info.storeOp);
		stencil_attachment.clearValue  = vk::ClearDepthStencilValue(0.f, stencil_info.clearValue);

		if (stencil_info.resolveTexture) {
			stencil_attachment.resolveImageView   = get_vk_image_view(stencil_info.resolveTexture);
			stencil_attachment.resolveImageLayout = vk::ImageLayout::eStencilAttachmentOptimal;
		}
	}

	vk::RenderingInfo render_info;
	render_info.renderArea           = to_vk_rect2d(info.renderArea);
	render_info.layerCount           = info.layerCount;
	render_info.viewMask             = {};
	render_info.colorAttachmentCount = static_cast<uint32_t>(color_attachments.size());
	render_info.pColorAttachments    = color_attachments.data();
	render_info.pDepthAttachment     = info.depthAttachment ? &depth_attachment : nullptr;
	render_info.pStencilAttachment   = info.stencilAttachment ? &stencil_attachment : nullptr;

	impl.vkCommandBuffer.beginRendering(render_info);
	impl.currentRenderingInfo = info;
}

void CommandBuffer::draw(
	uint32_t vtx_count,
	uint32_t instance_count,
	uint32_t vtx_offset,
	uint32_t instance_offset
) {
	auto& impl = getImpl(this);
	impl.vkCommandBuffer.draw(vtx_count, instance_count, vtx_offset, instance_offset);
}

void CommandBuffer::drawIndexed(
	uint32_t idx_count,
	uint32_t instance_count,
	uint32_t idx_offset,
	uint32_t vtx_offset,
	uint32_t instance_offset
) {
	auto& impl = getImpl(this);
	impl.vkCommandBuffer.drawIndexed(idx_count, instance_count, idx_offset, vtx_offset, instance_offset);
}

void CommandBuffer::drawMeshTask(
	uint32_t group_count_x,
	uint32_t group_count_y,
	uint32_t group_count_z
) {
	auto& impl = getImpl(this);
	impl.vkCommandBuffer.drawMeshTasksEXT(group_count_x, group_count_y, group_count_z);
}

void CommandBuffer::endRendering()
{
	auto& impl = getImpl(this);

	impl.vkCommandBuffer.endRendering();

	impl.currentPipeline      = {};
	impl.currentRenderingInfo = {};
}

void CommandBuffer::end()
{
	auto& impl = getImpl(this);

	impl.tracker->state = CommandBufferState::Executable;

	impl.vkCommandBuffer.end();
}

CommandSync CommandBuffer::submit(const SubmitInfo& info)
{
	auto& impl = getImpl(this);

	vk::SubmitInfo                          submit_info;
	small_vector<vk::Semaphore, 8>          wait_semaphores;
	small_vector<vk::PipelineStageFlags, 8> wait_stages;
	small_vector<vk::Semaphore, 8>          signal_semaphores;

	if (info.waitInfos.empty() && info.signalInfos.empty()) {
		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &impl.vkCommandBuffer;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores    = &get_vk_semaphore(impl.tracker->semaphore);
	} else {
		wait_semaphores.reserve(info.waitInfos.size());
		wait_stages.reserve(info.waitInfos.size());
		signal_semaphores.reserve(info.signalInfos.size() + 1);

		for (auto& wait_info : info.waitInfos) {
			wait_semaphores.push_back(get_vk_semaphore(wait_info.semaphore));
			wait_stages.push_back(to_vk_pipeline_stage_flags(wait_info.stageMask));
		}
		for (auto& signal_info : info.signalInfos)
			signal_semaphores.push_back(get_vk_semaphore(signal_info.semaphore));

		signal_semaphores.push_back(get_vk_semaphore(impl.tracker->semaphore));

		submit_info.waitSemaphoreCount   = static_cast<uint32_t>(wait_semaphores.size());
		submit_info.pWaitSemaphores      = wait_semaphores.data();
		submit_info.pWaitDstStageMask    = wait_stages.data();
		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &impl.vkCommandBuffer;
		submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
		submit_info.pSignalSemaphores    = signal_semaphores.data();
	}

	impl.submitToDedicatedQueue(submit_info);

	return CommandSync();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void CommandBufferImpl::submitToDedicatedQueue(const vk::SubmitInfo& submit_info)
{
	auto& device_impl = CoreObject::getImpl(device);

	tracker->state = CommandBufferState::Pending;

	switch (submitQueueType) {
	case SubmitQueueType::Transfer:
		device_impl.vkTransferQueue.submit(submit_info, get_vk_fence(tracker->fence));
		break;
	case SubmitQueueType::Compute:
		device_impl.vkComputeQueue.submit(submit_info, get_vk_fence(tracker->fence));
		break;
	case SubmitQueueType::Graphics:
		device_impl.vkGraphicsQueue.submit(submit_info, get_vk_fence(tracker->fence));
		break;
	}
}

VERA_NAMESPACE_END