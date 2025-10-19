#include "../../include/vera/pass/graphics_pass.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/texture_view.h"

#include <random>

VERA_NAMESPACE_BEGIN

GraphicsPass::GraphicsPass(obj<Device> device, const GraphicsPassCreateInfo& info) :
	m_device(device),
	m_depth_format(info.depthFormat),
	m_vertex_count(info.vertexCount),
	m_index_count(info.indexCount)
{
	GraphicsPipelineCreateInfo pipeline_info = {
		.vertexShader                   = info.vertexShader,
		.fragmentShader                 = info.fragmentShader,
		.primitiveInfo                  = PrimitiveInfo{
			.enableRestart = false,
			.topology      = PrimitiveTopology::TriangleList
		},
		.rasterizationInfo              = RasterizationInfo{},
		.tessellationPatchControlPoints = 0,
		.depthStencilInfo               = DepthStencilInfo{},
		.colorBlendInfo                 = ColorBlendInfo{
			.attachments = { 
				ColorBlendAttachmentState{} // default attachment state
			}
		}
	};

	if (!info.vertexInput.vertexInputDescriptor.empty()) {
		uint32_t vertex_size = info.vertexInput.vertexInputDescriptor.vertexSize();

		pipeline_info.vertexInputInfo = info.vertexInput;

		m_vertex_buffer = Buffer::createVertex(device, info.vertexCount * vertex_size);
		m_states.setVertexBuffer(m_vertex_buffer);
	}

	if (info.indexType != IndexType::Unknown) {
		m_index_buffer = Buffer::createIndex(device, info.indexType, info.indexCount);
		m_states.setIndexBuffer(m_index_buffer);
	}

	if (info.depthFormat != DepthFormat::Unknown) {
		pipeline_info.depthStencilInfo = DepthStencilInfo{
			.depthFormat      = info.depthFormat,
			.depthWriteEnable = true,
			.depthCompareOp   = CompareOp::Less
		};
	}

	m_pipeline = Pipeline::create(device, pipeline_info);
	m_parameter.init(m_pipeline->getPipelineLayout());
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

obj<Buffer> GraphicsPass::getIndexBuffer()
{
	return m_index_buffer;
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
			vr::TextureLayout::Undefined,
			vr::TextureLayout::DepthAttachmentOptimal);
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

	if (m_index_buffer)
		ctx->drawIndexed(m_states, m_parameter, m_index_count, 0, 0);
	else
		ctx->draw(m_states, m_parameter, m_vertex_count, 0);
}

VERA_NAMESPACE_END