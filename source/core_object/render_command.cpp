#include "../../include/vera/core/render_command.h"
#include "../impl/buffer_impl.h"
#include "../impl/render_command_impl.h"
#include "../impl/pipeline_impl.h"
#include "../impl/texture_impl.h"
#include "../impl/device_memory_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

static vk::ImageView get_vk_image_view(ref<Texture> texture)
{
	// TODO: remove
	return get_vk_image_view(CoreObject::getImpl(texture).textureView);
}

static void clear_rendering_info(RenderingInfo& info)
{
	info.renderArea = {};
	info.layerCount = 1;
	info.colorAttachments.clear();
	info.depthAttachment.reset();
	info.stencilAttachment.reset();
}

vk::CommandBuffer& get_vk_command_buffer(ref<RenderCommand> render_command)
{
	return CoreObject::getImpl(render_command).commandBuffer;
}

obj<RenderCommand> RenderCommand::create(obj<Device> device)
{
	auto  obj       = createNewObject<RenderCommand>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	vk::CommandPoolCreateInfo pool_info;
	pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	vk::CommandBufferAllocateInfo alloc_info;
	alloc_info.commandPool        = vk_device.createCommandPool(pool_info);
	alloc_info.level              = vk::CommandBufferLevel::ePrimary;
	alloc_info.commandBufferCount = 1;

	impl.device        = std::move(device);
	impl.commandPool   = alloc_info.commandPool;
	impl.commandBuffer = vk_device.allocateCommandBuffers(alloc_info).front();

	return obj;
}

RenderCommand::~RenderCommand()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.freeCommandBuffers(impl.commandPool, impl.commandBuffer);
	vk_device.destroy(impl.commandPool);

	destroyObjectImpl(this);
}

obj<Device> RenderCommand::getDevice()
{
	return getImpl(this).device;
}

void RenderCommand::begin()
{
	auto& impl = getImpl(this);

	vk::CommandBufferBeginInfo begin_info;
	begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	impl.commandBuffer.begin(begin_info);
}

void RenderCommand::setViewport(const Viewport& viewport)
{
	auto& impl = getImpl(this);

	vk::Viewport vk_viewport;
	vk_viewport.x        = viewport.posX;
	vk_viewport.y        = viewport.posY;
	vk_viewport.width    = viewport.width;
	vk_viewport.height   = viewport.height;
	vk_viewport.minDepth = viewport.minDepth;
	vk_viewport.maxDepth = viewport.maxDepth;

	impl.commandBuffer.setViewport(0, vk_viewport);
	impl.currentViewport = viewport;
}

void RenderCommand::setScissor(const Scissor& scissor)
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

void RenderCommand::setVertexBuffer(ref<Buffer> buffer)
{
	auto& impl        = getImpl(this);
	auto& buffer_impl = getImpl(buffer);
	auto  offset      = vk::DeviceSize{0};

	if (!buffer_impl.usage.has(BufferUsageFlagBits::VertexBuffer))
		throw Exception("buffer is not for vertex");

	impl.commandBuffer.bindVertexBuffers(0, 1, &buffer_impl.buffer, &offset);
	impl.currentVertexBuffer = buffer;
}

void RenderCommand::setIndexBuffer(ref<Buffer> buffer)
{
	auto& impl        = getImpl(this);
	auto& buffer_impl = getImpl(buffer);

	if (!buffer_impl.usage.has(BufferUsageFlagBits::IndexBuffer))
		throw Exception("buffer is not for index");

	// consider unbinding index buffer
	impl.commandBuffer.bindIndexBuffer(buffer_impl.buffer, 0, to_vk_index_type(buffer_impl.indexType));
	impl.currentIndexBuffer = buffer;
}

void RenderCommand::setPipeline(ref<Pipeline> pipeline)
{
	auto& impl          = getImpl(this);
	auto& pipeline_impl = getImpl(pipeline);

	impl.commandBuffer.bindPipeline(
		pipeline_impl.pipelineBindPoint,
		pipeline_impl.pipeline);
	impl.currentPipeline = pipeline;
}

void RenderCommand::transitionImageLayout(
	ref<Texture> texture,
	PipelineStageFlags src_stage_mask,
	PipelineStageFlags dst_stage_mask,
	AccessFlags src_access_mask,
	AccessFlags dst_access_mask,
	ImageLayout old_layout,
	ImageLayout new_layout)
{
	auto& impl = getImpl(this);

	vk::ImageMemoryBarrier barrier;
	barrier.srcAccessMask                   = to_vk_access_flags(src_access_mask);
	barrier.dstAccessMask                   = to_vk_access_flags(dst_access_mask);
	barrier.oldLayout                       = to_vk_image_layout(old_layout);
	barrier.newLayout                       = to_vk_image_layout(new_layout);
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = get_vk_image(texture);
	barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
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

void RenderCommand::copyBufferToTexture(
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

void RenderCommand::beginRendering(const RenderingInfo& info)
{
	auto& impl = getImpl(this);

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
		color_attachment.resolveMode = to_vk_resolve_mode(color_info.resolveMode);
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
		depth_attachment.resolveMode = to_vk_resolve_mode(depth_info.resolveMode);
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
		stencil_attachment.resolveMode = to_vk_resolve_mode(stencil_info.resolveMode);
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

void RenderCommand::draw(uint32_t vtx_count, uint32_t instance_count, uint32_t vtx_offset, uint32_t instance_offset)
{
	auto& impl = getImpl(this);

	impl.commandBuffer.draw(vtx_count, instance_count, vtx_offset, instance_offset);
}

void RenderCommand::drawIndexed(uint32_t idx_count, uint32_t instance_count, uint32_t idx_offset, uint32_t vtx_offset, uint32_t instance_offset)
{
	auto& impl = getImpl(this);

	impl.commandBuffer.drawIndexed(idx_count, instance_count, idx_offset, vtx_offset, instance_offset);
}

void RenderCommand::endRendering()
{
	auto& impl = getImpl(this);

	impl.commandBuffer.endRendering();

	impl.currentPipeline = {};
	clear_rendering_info(impl.currentRenderingInfo);
}

void RenderCommand::end()
{
	auto& impl = getImpl(this);

	impl.commandBuffer.end();
}

void RenderCommand::reset()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	
	impl.currentViewport     = {};
	impl.currentScissor      = {};
	impl.currentVertexBuffer = {};
	impl.currentIndexBuffer  = {};
	clear_rendering_info(impl.currentRenderingInfo);
	impl.currentPipeline     = {};

	vk_device.resetCommandPool(impl.commandPool);
}

VERA_NAMESPACE_END