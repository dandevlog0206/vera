#pragma once

#include "core_object.h"
#include "render_frame.h"

VERA_NAMESPACE_BEGIN

enum class TextureLayout VERA_ENUM;

class Device;
class CommandBuffer;
class GraphicsState;
class ShaderParameter;
class Texture;

struct RenderContextCreateInfo
{
	uint32_t frameCount        = 3;
	bool     dynamicFrameCount = true;
};

class RenderContext : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(RenderContext)
public:
	static obj<RenderContext> create(obj<Device> device, const RenderContextCreateInfo& info);
	~RenderContext();

	obj<Device> getDevice();
	obj<CommandBuffer> getRenderCommand();

	VERA_NODISCARD uint32_t getCurrentFrameIndex() const;
	VERA_NODISCARD uint32_t getFrameCount() const;
	VERA_NODISCARD const RenderFrame& getCurrentFrame() const;

	VERA_NODISCARD CommandBufferSync getCommandBufferSync() const;

	void transitionImageLayout(
		ref<Texture>   texture,
		TextureLayout  old_layout,
		TextureLayout  new_layout);

	void draw(
		const GraphicsState& states,
		uint32_t             vtx_count,
		uint32_t             vtx_off);

	void draw(
		const GraphicsState&   states,
		const ShaderParameter& params,
		uint32_t               vtx_count,
		uint32_t               vtx_off);

	void drawIndexed(
		const GraphicsState&   states,
		const ShaderParameter& params,
		uint32_t               idx_count,
		uint32_t               idx_off,
		uint32_t               vtx_off);

	void submit();
};

VERA_NAMESPACE_END