#pragma once

#include "device.h"
#include "command_buffer_sync.h"
#include "../graphics/color.h"
#include "../util/array_view.h"
#include "../util/rect.h"
#include <optional>
#include <vector>

VERA_NAMESPACE_BEGIN

class Pipeline;
class PipelineLayout;
class ShaderParameter;
class DescriptorSet;
class Buffer;
class Texture;
class GraphicsState;
class ShaderParameter;

struct Viewport
{
	float posX     = 0.f;
	float posY     = 0.f;
	float width    = 1.0f;
	float height   = 1.0f;
	float minDepth = 0.f;
	float maxDepth = 1.f;
};

struct Scissor
{
	uint32_t minX = 0;
	uint32_t minY = 0;
	uint32_t maxX = 0;
	uint32_t maxY = 0;
};

template <class ClearType>
struct AttachmentInfo
{
	ref<Texture>        texture        = {};
	ref<Texture>        resolveTexture = {};
	ResolveModeFlagBits resolveMode    = {};
	LoadOp              loadOp         = LoadOp::Load;
	StoreOp             storeOp        = StoreOp::Store;
	ClearType           clearValue     = {};
};

typedef AttachmentInfo<Color> ColorAttachmentInfo;
typedef AttachmentInfo<float> DepthAttachmentInfo;
typedef AttachmentInfo<uint32_t> StencilAttachmentInfo;

struct RenderingInfo
{
	irect2d                              renderArea        = {};
	uint32_t                             layerCount        = 1;
	std::vector<ColorAttachmentInfo>     colorAttachments;
	std::optional<DepthAttachmentInfo>   depthAttachment;
	std::optional<StencilAttachmentInfo> stencilAttachment;
};

class CommandBuffer : protected CoreObject // TODO: consider rename to command buffer
{
	VERA_CORE_OBJECT_INIT(CommandBuffer)
public:
	static obj<CommandBuffer> create(obj<Device> device);
	~CommandBuffer();

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;

	VERA_NODISCARD CommandBufferSync getSync() const VERA_NOEXCEPT;

	void reset();

	void begin();

	void copyBufferToTexture(
		ref<Texture> dst,
		ref<Buffer>  src,
		size_t       buffer_offset,
		uint32_t     buffer_row_length,
		uint32_t     buffer_image_height,
		uint3        image_offset,
		extent3d     image_extent);

	void transitionImageLayout(
		ref<Texture>       texture,
		PipelineStageFlags src_stage_mask,
		PipelineStageFlags dst_stage_mask,
		AccessFlags        src_access_mask,
		AccessFlags        dst_access_mask,
		TextureLayout      old_layout,
		TextureLayout      new_layout);

	void setViewport(const Viewport& viewport);
	void setScissor(const Scissor& scissor);
	void bindVertexBuffer(obj<Buffer> buffer, size_t offset = 0);
	void bindIndexBuffer(obj<Buffer> buffer, size_t offset = 0);
	void bindPipeline(obj<Pipeline> pipeline);
	
	void pushConstant(
		const_ref<PipelineLayout> pipeline_layout,
		ShaderStageFlags          stage_flags,
		uint32_t                  offset,
		const void*               data,
		uint32_t                  size);

	void bindDescriptorSet(
		const_ref<PipelineLayout> pipeline_layout,
		uint32_t                  set,
		ref<DescriptorSet>        desc_set);

	void bindDescriptorSet(
		const_ref<PipelineLayout> pipeline_layout,
		uint32_t                  set,
		ref<DescriptorSet>        desc_set,
		array_view<uint32_t>      dynamic_offsets);
	
	void bindGraphicsState(const GraphicsState& state);

	void bindShaderParameter(obj<ShaderParameter> params);

	void beginRendering(const RenderingInfo& info);

	void draw(
		uint32_t vtx_count,
		uint32_t instance_count,
		uint32_t vtx_offset,
		uint32_t instance_offset);

	void drawIndexed(
		uint32_t idx_count,
		uint32_t instance_count,
		uint32_t idx_offset,
		uint32_t vtx_offset,
		uint32_t instance_offset);

	void drawMeshTask(
		uint32_t group_count_x,
		uint32_t group_count_y,
		uint32_t group_count_z);

	void endRendering();

	void end();

	CommandBufferSync submit();
};

VERA_NAMESPACE_END