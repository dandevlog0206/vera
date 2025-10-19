#pragma once

#include "../core/framebuffer.h"
#include "../core/pipeline.h"
#include "../core/render_context.h"
#include "../core/texture.h"
#include "../graphics/graphics_state.h"
#include "../graphics/vertex_input.h"
#include "../graphics/shader_parameter.h"

VERA_NAMESPACE_BEGIN

struct ForwardPassCreateInfo
{

};

class ForwardPass
{
public:
	ForwardPass(obj<Device> device);
	virtual ~ForwardPass();
	
	obj<Device> getDevice();
	obj<Pipeline> getPipeline();
	obj<Buffer> getVertexBuffer();
	obj<Buffer> getIndexBuffer();
	ShaderParameter& getShaderParameter();

	virtual void execute(obj<RenderContext> cmd, ref<FrameBuffer> framebuffer);
};

VERA_NAMESPACE_END