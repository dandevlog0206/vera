#pragma once

#include "core_object.h"

VERA_NAMESPACE_BEGIN

class Device;
class RenderCommand;
class GraphicsState;
class ShaderParameter;

class RenderContext : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(RenderContext)
public:
	static obj<RenderContext> create(obj<Device> device);
	~RenderContext();

	obj<Device> getDevice();
	obj<RenderCommand> getRenderCommand();

	void draw(
		const GraphicsState& states,
		uint32_t             vtx_count,
		uint32_t             vtx_off);

	void draw(
		const GraphicsState&   states,
		const ShaderParameter& params,
		uint32_t               vtx_count,
		uint32_t               vtx_off);

	// void draw(ref<GraphicsState> states, ref<Geometry>);

	void submit();
};

VERA_NAMESPACE_END