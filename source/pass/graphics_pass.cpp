#include "../../include/vera/pass/graphics_pass.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/shader_reflection.h"

#include <random>

VERA_NAMESPACE_BEGIN

GraphicsPass::GraphicsPass(ref<Device> device, const GraphicsPassCreateInfo& info) :
	m_device(device),
	m_parameter(ShaderReflection::create({ info.vertexShader, info.fragmentShader })),
	m_vertex_count(info.vertexCount)
{
	GraphicsPipelineCreateInfo pipeline_info = {
		.vertexShader                  = info.vertexShader,
		.fragmentShader                = info.fragmentShader,
		.primitiveInfo                 = PrimitiveInfo{
			.enableRestart = false,
			.topology      = PrimitiveTopology::TriangleList
		},
		.rasterizationInfo             = RasterizationInfo{},
		.tesselationPatchControlPoints = 0,
		.depthStencilInfo              = DepthStencilInfo{},
		.colorBlendInfo                = ColorBlendInfo{}
	};

	if (info.useVertexBuffer) {
		m_vertex_buffer = Buffer::createVertex(device, info.vertexCount * sizeof(Vertex));
		m_states.setVertexBuffer(m_vertex_buffer);

		pipeline_info.vertexInputInfo = VertexInputInfo{
			.vertexInputDescriptor = GraphicsPass::Vertex{}
		};
	}

	m_pipeline = Pipeline::create(device, pipeline_info);
	m_states.setPipeline(m_pipeline);
}

GraphicsPass::~GraphicsPass()
{

}

ref<Device> GraphicsPass::getDevice()
{
	return m_device;
}

ref<Pipeline> GraphicsPass::getPipeline()
{
	return m_pipeline;
}

ref<Buffer> GraphicsPass::getVertexBuffer()
{
	return m_vertex_buffer;
}

ShaderParameter& GraphicsPass::getShaderParameter()
{
	return m_parameter;
}

void GraphicsPass::execute(ref<RenderContext> ctx, ref<Texture> texture)
{
	uint32_t width  = texture->width();
	uint32_t height = texture->height();

	m_states.setViewport(Viewport{
		.width  = static_cast<float>(width),
		.height = static_cast<float>(height)
	});

	m_states.setScissor(Scissor{
		.minX = 0,
		.minY = 0,
		.maxX = static_cast<uint32_t>(width),
		.maxY = static_cast<uint32_t>(height)
	});

	RenderingInfo rendering_info = {
	.renderArea = irect2d{
		static_cast<int32_t>(0),
		static_cast<int32_t>(0),
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)}
	};

	rendering_info.colorAttachments.push_back(
		ColorAtttachmentInfo{
			.texture    = texture,
			.loadOp     = LoadOp::Clear,
			.storeOp    = StoreOp::Store,
			.clearValue = Colors::Black
		});

	m_states.setRenderingInfo(rendering_info);

	ctx->draw(m_states, m_parameter, m_vertex_count, 0);
}

VERA_NAMESPACE_END