#pragma once

#include "../core/pipeline.h"
#include "../core/render_context.h"
#include "../core/graphics_state.h"
#include "../shader/shader_parameter.h"

VERA_NAMESPACE_BEGIN

struct GraphicsPassCreateInfo
{
	obj<Shader> vertexShader;
	obj<Shader> fragmentShader;

	bool        useVertexBuffer = false;
	uint32_t    vertexCount;
};

class GraphicsPass
{
public:
	struct Vertex
	{
		vr::float2 pos;
		vr::float4 color;
		vr::float2 uv;

		VERA_VERTEX_DESCRIPTOR_BEGIN(Vertex)
			VERA_VERTEX_ATTRIBUTE(0, pos),
			VERA_VERTEX_ATTRIBUTE(1, color),
			VERA_VERTEX_ATTRIBUTE(1, uv),
		VERA_VERTEX_DESCRIPTOR_END
	};

	GraphicsPass(obj<Device> device, const GraphicsPassCreateInfo& info);
	virtual ~GraphicsPass();

	obj<Device> getDevice();
	obj<Pipeline> getPipeline();
	obj<Buffer> getVertexBuffer();

	ShaderParameter& getShaderParameter();

	virtual void execute(obj<RenderContext> cmd, ref<Texture> texture);

private:
	obj<Device>     m_device;
	obj<Pipeline>   m_pipeline;
	obj<Buffer>     m_vertex_buffer;
	ShaderParameter m_parameter;
	GraphicsState   m_states;

	uint32_t        m_vertex_count;
};

VERA_NAMESPACE_END