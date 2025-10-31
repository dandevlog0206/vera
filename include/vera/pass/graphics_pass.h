#pragma once

#include "../core/framebuffer.h"
#include "../core/pipeline.h"
#include "../core/render_context.h"
#include "../core/texture.h"
#include "../graphics/graphics_state.h"
#include "../graphics/vertex_input.h"
#include "../graphics/shader_parameter.h"

VERA_NAMESPACE_BEGIN

struct GraphicsPassCreateInfo
{
	obj<Shader>     vertexShader;
	obj<Shader>     geometryShader;
	obj<Shader>     fragmentShader;

	VertexInputInfo   vertexInput;
	uint32_t          vertexCount;
	IndexType         indexType         = IndexType::Unknown;
	uint32_t          indexCount;
	PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
	DepthFormat       depthFormat       = DepthFormat::Unknown;
};

class GraphicsPass
{
public:
	GraphicsPass(obj<Device> device, const GraphicsPassCreateInfo& info);
	virtual ~GraphicsPass();

	obj<Device> getDevice();
	obj<Pipeline> getPipeline();
	obj<Buffer> getVertexBuffer();
	obj<Buffer> getIndexBuffer();

	ShaderParameter& getShaderParameter();

	virtual void execute(obj<RenderContext> cmd, ref<FrameBuffer> framebuffer);

private:
	obj<Device>     m_device;
	obj<Pipeline>   m_pipeline;
	obj<Buffer>     m_vertex_buffer;
	obj<Buffer>     m_index_buffer;
	obj<Texture>    m_depth;
	ShaderParameter m_parameter;
	GraphicsState   m_states;

	DepthFormat     m_depth_format;
	uint32_t        m_vertex_count;
	uint32_t        m_index_count;
};

VERA_NAMESPACE_END