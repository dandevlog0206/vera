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

static vk::ImageView get_vk_image_view(ref<Texture> texture)
{
	return get_vk_image_view(texture->getTextureView());
}

const vk::CommandBuffer& get_vk_command_buffer(const_ref<CommandBuffer> cmd_buffer) VERA_NOEXCEPT
{
	return CoreObject::getImpl(cmd_buffer).commandBuffer;
}

vk::CommandBuffer& get_vk_command_buffer(ref<CommandBuffer> cmd_buffer) VERA_NOEXCEPT
{
	return CoreObject::getImpl(cmd_buffer).commandBuffer;
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
	impl.semaphore             = Semaphore::create(impl.device);
	impl.fence                 = Fence::create(impl.device);
	impl.commandPool           = alloc_info.commandPool;
	impl.commandBuffer         = vk_device.allocateCommandBuffers(alloc_info).front();
	impl.submitID              = 0;
	impl.submitQueueType       = SubmitQueueType::Transfer;
	impl.state                 = CommandBufferState::Initialized;
	impl.currentViewport       = {};
	impl.currentScissor        = {};
	impl.currentVertexBuffer   = {};
	impl.currentIndexBuffer    = {};
	impl.currentRenderingInfo  = {};
	impl.currentDescriptorSets = {};
	impl.currentPipeline       = {};

	return obj;
}

CommandBuffer::~CommandBuffer()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	// TODO: use CommandBufferSync lator
	vk_device.waitIdle();
	impl.boundObjects.clear();
	impl.boundShaderParameters.clear();

	vk_device.freeCommandBuffers(impl.commandPool, impl.commandBuffer);
	vk_device.destroy(impl.commandPool);

	destroyObjectImpl(this);
}

obj<Device> CommandBuffer::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

CommandBufferSync CommandBuffer::getSync() const VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	return CommandBufferSync(&impl, impl.submitID);
}

void CommandBuffer::reset()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	if (impl.state == CommandBufferState::Submitted && !impl.fence->signaled())
		throw Exception("cannot reset a submitted command buffer that is not completed");

	impl.fence->reset();

	impl.boundObjects.clear();
	impl.boundShaderParameters.clear();

	impl.submitID               += 1;
	impl.submitQueueType         = SubmitQueueType::Transfer;
	impl.state                   = CommandBufferState::Initialized;
	impl.currentViewport         = {};
	impl.currentScissor          = {};
	impl.currentVertexBuffer     = {};
	impl.currentIndexBuffer      = {};
	impl.currentRenderingInfo    = {};
	impl.currentDescriptorSets   = {};
	impl.currentPipeline         = {};

	vk_device.resetCommandPool(impl.commandPool);
}

void CommandBuffer::begin()
{
	auto& impl = getImpl(this);

	impl.state = CommandBufferState::Recording;

	vk::CommandBufferBeginInfo begin_info;
	begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	impl.commandBuffer.begin(begin_info);
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

	impl.commandBuffer.copyBufferToImage(
		buffer_impl.buffer,
		texture_impl.image,
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

	impl.commandBuffer.pipelineBarrier(
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
	vk_viewport.y        = viewport.height + viewport.posY;
	vk_viewport.width    = viewport.width;
	vk_viewport.height   = -viewport.height;
	vk_viewport.minDepth = viewport.minDepth;
	vk_viewport.maxDepth = viewport.maxDepth;

	impl.commandBuffer.setViewport(0, vk_viewport);
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

	impl.commandBuffer.setScissor(0, vk_rect);
	impl.currentScissor = scissor;
}

void CommandBuffer::bindVertexBuffer(obj<Buffer> buffer, size_t offset)
{
	auto& impl        = getImpl(this);
	auto& buffer_impl = getImpl(buffer);
	auto  offsets     = vk::DeviceSize{ offset };

	if (!buffer_impl.usage.has(BufferUsageFlagBits::VertexBuffer))
		throw Exception("buffer is not for vertex");

	impl.commandBuffer.bindVertexBuffers(0, 1, &buffer_impl.buffer, &offsets);
	impl.currentVertexBuffer = buffer;
	impl.boundObjects.push_back(obj_cast<CoreObject>(std::move(buffer)));
}

void CommandBuffer::bindIndexBuffer(obj<Buffer> buffer, size_t offset)
{
	auto& impl        = getImpl(this);
	auto& buffer_impl = getImpl(buffer);

	if (!buffer_impl.usage.has(BufferUsageFlagBits::IndexBuffer))
		throw Exception("buffer is not for index");

	impl.commandBuffer.bindIndexBuffer(buffer_impl.buffer, offset, to_vk_index_type(buffer_impl.indexType));
	impl.currentIndexBuffer = buffer;
	impl.boundObjects.push_back(obj_cast<CoreObject>(std::move(buffer)));
}

void CommandBuffer::bindPipeline(obj<Pipeline> pipeline)
{
	auto& impl          = getImpl(this);
	auto& pipeline_impl = getImpl(pipeline);

	impl.commandBuffer.bindPipeline(
		to_vk_pipeline_bind_point(pipeline_impl.pipelineBindPoint),
		pipeline_impl.pipeline);
	impl.currentPipeline = pipeline;
	impl.boundObjects.push_back(obj_cast<CoreObject>(std::move(pipeline)));
}

void CommandBuffer::pushConstant(
	const_ref<PipelineLayout> pipeline_layout,
	ShaderStageFlags          stage_flags,
	uint32_t                  offset,
	const void*               data,
	uint32_t                  size)
{
	auto& impl = getImpl(this);
	impl.commandBuffer.pushConstants(
		get_vk_pipeline_layout(pipeline_layout),
		to_vk_shader_stage_flags(stage_flags),
		offset,
		size,
		data);
}

void CommandBuffer::bindDescriptorSet(
	const_ref<PipelineLayout> pipeline_layout,
	uint32_t                  set,
	ref<DescriptorSet>        desc_set
) {
	auto& impl = getImpl(this);

	impl.commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		get_vk_pipeline_layout(pipeline_layout),
		set,
		1,
		&get_vk_descriptor_set(desc_set),
		0,
		nullptr);
}

void CommandBuffer::bindDescriptorSet(
	const_ref<PipelineLayout> pipeline_layout,
	uint32_t                  set,
	ref<DescriptorSet>        desc_set,
	array_view<uint32_t>      dynamic_offsets
) {
	VERA_ASSERT_MSG(pipeline_layout, "pipeline layout is null");
	
	auto& impl = getImpl(this);

	impl.commandBuffer.bindDescriptorSets(
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
	auto  vk_cmd = impl.commandBuffer;

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

void CommandBuffer::bindShaderParameter(obj<ShaderParameter> params)
{
	VERA_ASSERT_MSG(params, "shader storage is null");

	auto& impl         = getImpl(this);
	auto& storage_impl = getImpl(params);

	for (const auto& bound_storage : impl.boundShaderParameters) {
		if (bound_storage == params) {
			storage_impl.bind(impl);
			return;
		}
	}

	impl.boundShaderParameters.push_back(params);
	storage_impl.bind(impl);
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

	impl.commandBuffer.beginRendering(render_info);
	impl.currentRenderingInfo = info;
}

void CommandBuffer::draw(
	uint32_t vtx_count,
	uint32_t instance_count,
	uint32_t vtx_offset,
	uint32_t instance_offset
) {
	auto& impl = getImpl(this);
	impl.commandBuffer.draw(vtx_count, instance_count, vtx_offset, instance_offset);
}

void CommandBuffer::drawIndexed(
	uint32_t idx_count,
	uint32_t instance_count,
	uint32_t idx_offset,
	uint32_t vtx_offset,
	uint32_t instance_offset
) {
	auto& impl = getImpl(this);
	impl.commandBuffer.drawIndexed(idx_count, instance_count, idx_offset, vtx_offset, instance_offset);
}

void CommandBuffer::drawMeshTask(
	uint32_t group_count_x,
	uint32_t group_count_y,
	uint32_t group_count_z
) {
	auto& impl = getImpl(this);
	impl.commandBuffer.drawMeshTasksEXT(group_count_x, group_count_y, group_count_z);
}

void CommandBuffer::endRendering()
{
	auto& impl = getImpl(this);

	impl.commandBuffer.endRendering();

	impl.currentPipeline      = {};
	impl.currentRenderingInfo = {};
}

void CommandBuffer::end()
{
	auto& impl = getImpl(this);

	impl.state = CommandBufferState::Executable;

	impl.commandBuffer.end();
}

CommandBufferSync CommandBuffer::submit()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	impl.state = CommandBufferState::Submitted;

	vk::SubmitInfo submit_info;
	submit_info.commandBufferCount   = 1;
	submit_info.pCommandBuffers      = &impl.commandBuffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores    = &get_vk_semaphore(impl.semaphore);

	for (auto& obj : impl.boundShaderParameters) {
		auto& param_impl = getImpl(obj);
		param_impl.submitFrame(impl);
	}

	switch (impl.submitQueueType) {
	case SubmitQueueType::Transfer:
		device_impl.transferQueue.submit(submit_info, get_vk_fence(impl.fence));
		break;
	case SubmitQueueType::Compute:
		device_impl.computeQueue.submit(submit_info, get_vk_fence(impl.fence));
		break;
	case SubmitQueueType::Graphics:
		device_impl.graphicsQueue.submit(submit_info, get_vk_fence(impl.fence));
		break;
	}

	return CommandBufferSync(&impl, impl.submitID);
}

VERA_NAMESPACE_END