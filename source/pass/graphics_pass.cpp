#include "../../include/vera/pass/graphics_pass.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/shader_reflection.h"

#include <random>

VERA_NAMESPACE_BEGIN

GraphicsPass::GraphicsPass(obj<Device> device, const GraphicsPassCreateInfo& info) :
	m_device(device),
	m_parameter(ShaderReflection::create({ info.vertexShader, info.fragmentShader })),
	m_depth_format(info.depthFormat),
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
		.colorBlendInfo                = ColorBlendInfo{}
	};

	if (!info.vertexInput.vertexInputDescriptor.empty()) {
		uint32_t vertex_size = info.vertexInput.vertexInputDescriptor.vertexSize();

		pipeline_info.vertexInputInfo = info.vertexInput;

		m_vertex_buffer = Buffer::createVertex(device, info.vertexCount * vertex_size);
		m_states.setVertexBuffer(m_vertex_buffer);
	}

	if (info.depthFormat != DepthFormat::Unknown) {
		pipeline_info.depthStencilInfo = DepthStencilInfo{
			.depthFormat      = info.depthFormat,
			.depthWriteEnable = true,
			.depthCompareOp   = CompareOp::Less
		};
	}

	m_pipeline = Pipeline::create(device, pipeline_info);
	m_states.setPipeline(m_pipeline);
}

GraphicsPass::~GraphicsPass()
{

}

obj<Device> GraphicsPass::getDevice()
{
	return m_device;
}

obj<Pipeline> GraphicsPass::getPipeline()
{
	return m_pipeline;
}

obj<Buffer> GraphicsPass::getVertexBuffer()
{
	return m_vertex_buffer;
}

ShaderParameter& GraphicsPass::getShaderParameter()
{
	return m_parameter;
}

void GraphicsPass::execute(obj<RenderContext> ctx, ref<FrameBuffer> framebuffer)
{
	uint32_t width  = framebuffer->width();
	uint32_t height = framebuffer->height();

	if (m_depth_format != DepthFormat::Unknown && !(m_depth && m_depth->width() == width && m_depth->height() == height)) {
		auto cmd = ctx->getRenderCommand();
		
		m_depth = Texture::createDepth(m_device, width, height, m_depth_format);
	
		cmd->transitionImageLayout(
			m_depth,
			vr::PipelineStageFlagBits::TopOfPipe,
			vr::PipelineStageFlagBits::EarlyFragmentTests,
			vr::AccessFlagBits::None,
			vr::AccessFlagBits::DepthStencilAttachmentWrite | vr::AccessFlagBits::DepthStencilAttachmentWrite,
			vr::ImageLayout::Undefined,
			vr::ImageLayout::DepthAttachmentOptimal);
	}

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
			.texture    = framebuffer->getTexture(),
			.loadOp     = LoadOp::Clear,
			.storeOp    = StoreOp::Store,
			.clearValue = Colors::Black
		});

	if (m_depth) {
		rendering_info.depthAttachment =
			DepthAtttachmentInfo{
				.texture    = m_depth,
				.loadOp     = LoadOp::Clear,
				.storeOp    = StoreOp::DontCare,
				.clearValue = 1.f
			};
	}

	m_states.setRenderingInfo(rendering_info);

	ctx->draw(m_states, m_parameter, m_vertex_count, 0);
}

VERA_NAMESPACE_END