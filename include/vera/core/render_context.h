#pragma once

#include "texture.h"
#include "frame_sync.h"

VERA_NAMESPACE_BEGIN

class Device;
class CommandBuffer;
class GraphicsState;
class ShaderParameter;

class RenderContext : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(RenderContext)
public:
	static obj<RenderContext> create(obj<Device> device);
	~RenderContext();

	obj<Device> getDevice();
	obj<CommandBuffer> getRenderCommand();

	FrameSync getFrameSync() const;

	void transitionImageLayout(
		ref<Texture> texture,
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