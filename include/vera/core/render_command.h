#pragma once

#include "pipeline.h"
#include "buffer.h"
#include "texture.h"
#include "device_memory.h"
#include "../graphics/color.h"
#include "../util/rect.h"
#include <vector>

VERA_NAMESPACE_BEGIN

enum class ResolveMode VERA_ENUM
{
	None,
	SampleZero,
	Average,
	Min,
	Max
};

enum class LoadOp VERA_ENUM
{
	Load,
	Clear,
	DontCare
};

enum class StoreOp VERA_ENUM
{
	Store,
	DontCare
};

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
	ref<Texture> texture        = {};
	ref<Texture> resolveTexture = {};
	ResolveMode  resolveMode    = {};
	LoadOp       loadOp         = LoadOp::Load;
	StoreOp      storeOp        = StoreOp::Store;
	ClearType    clearValue     = {};
};

typedef AttachmentInfo<Color> ColorAtttachmentInfo;
typedef AttachmentInfo<float> DepthAtttachmentInfo;
typedef AttachmentInfo<uint32_t> StencilAtttachmentInfo;

struct RenderingInfo
{
	irect2d                               renderArea        = {};
	uint32_t                              layerCount        = 1;
	std::vector<ColorAtttachmentInfo>     colorAttachments;
	std::optional<DepthAtttachmentInfo>   depthAttachment;
	std::optional<StencilAtttachmentInfo> stencilAttachment;
};

class RenderCommand : protected CoreObject // TODO: consider rename to command buffer
{
	VERA_CORE_OBJECT_INIT(RenderCommand)
public:
	static obj<RenderCommand> create(obj<Device> device);
	~RenderCommand();

	obj<Device> getDevice();

	void begin();

	void setViewport(const Viewport& viewport);
	void setScissor(const Scissor& scissor);
	void setVertexBuffer(ref<Buffer> buffer);
	void setIndexBuffer(ref<Buffer> buffer);
	void setPipeline(ref<Pipeline> pipeline);

	void transitionImageLayout(
		ref<Texture>       texture,
		PipelineStageFlags src_stage_mask,
		PipelineStageFlags dst_stage_mask,
		AccessFlags        src_access_mask,
		AccessFlags        dst_access_mask,
		ImageLayout        old_layout,
		ImageLayout        new_layout);

	void copyBufferToTexture(
		ref<Texture> dst,
		ref<Buffer>  src,
		size_t       buffer_offset,
		uint32_t     buffer_row_length,
		uint32_t     buffer_image_height,
		uint3        image_offset,
		extent3d     image_extent);

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

	void endRendering();

	void end();

	void reset();
};

VERA_NAMESPACE_END