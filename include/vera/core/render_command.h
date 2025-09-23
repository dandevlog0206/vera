#pragma once

#include "pipeline.h"
#include "texture.h"
#include "device_memory.h"
#include "color.h"
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

class RenderCommand : protected CoreObject
{
	VERA_CORE_OBJECT(RenderCommand)
public:
	static ref<RenderCommand> create(ref<Device> device);
	~RenderCommand();

	void begin();

	void setViewport(const Viewport& viewport);
	void setScissor(const Scissor& scissor);
	void setPipeline(ref<Pipeline> pipeline);

	void transitionImageLayout(
		ref<Texture>       texture,
		PipelineStageFlags src_stage_mask,
		PipelineStageFlags dst_stage_mask,
		AccessFlags        src_access_mask,
		AccessFlags        dst_access_mask,
		ImageLayout        old_layout,
		ImageLayout        new_layout);

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