#pragma once

#include "../core/pipeline.h"
#include "../core/render_context.h"
#include "../core/graphics_state.h"
#include "../shader/shader_parameter.h"

VERA_NAMESPACE_BEGIN

struct GraphicsPassCreateInfo
{
	ref<Shader> vertexShader;
	ref<Shader> fragmentShader;
};

class GraphicsPass
{
public:
	GraphicsPass(ref<Device> device, const GraphicsPassCreateInfo& info);
	virtual ~GraphicsPass();

	ref<Pipeline> getPipeline();
	ShaderParameter& getShaderParameter();

	virtual void execute(ref<RenderContext> cmd, ref<Texture> texture);

private:
	ref<Device>     m_device;
	ref<Pipeline>   m_pipeline;
	ShaderParameter m_parameter;
	GraphicsState   m_states;
};

VERA_NAMESPACE_END