#include "../../include/vera/graphics/graphics_state.h"
#include "../impl/command_buffer_impl.h"

VERA_NAMESPACE_BEGIN

template <class ClearType>
bool operator==(const AttachmentInfo<ClearType>& lhs, const AttachmentInfo<ClearType>& rhs)
{
	return !memcmp(&lhs, &rhs, sizeof(lhs));
}

bool operator==(const RenderingInfo& lhs, const RenderingInfo& rhs)
{
	if (&lhs == &rhs) return true;

	if (lhs.renderArea != rhs.renderArea ||
		lhs.layerCount != rhs.layerCount ||
		lhs.colorAttachments.size() != rhs.colorAttachments.size()) return false;
	
	auto* colorl_ptr = lhs.colorAttachments.data();
	auto* colorr_ptr = rhs.colorAttachments.data();
	auto  color_size = sizeof(AttachmentInfo<Color>) * lhs.colorAttachments.size();
	if (memcmp(colorl_ptr, colorr_ptr, color_size)) return false;

	auto& depthl = lhs.depthAttachment;
	auto& depthr = rhs.depthAttachment;
	if (depthl.has_value() != depthr.has_value()) return false;
	if (depthl.has_value() && depthl.value() == depthr.value()) return false;

	auto& stencill = lhs.stencilAttachment;
	auto& stencilr = rhs.stencilAttachment;
	if (stencill.has_value() != stencilr.has_value()) return false;
	if (stencill.has_value() && stencill.value() == stencilr.value()) return false;

	return true;
}

GraphicsState::GraphicsState()
{
}

GraphicsState::~GraphicsState()
{
}

void GraphicsState::setViewport(const Viewport& viewport)
{
	if (m_viewports.empty())
		m_viewports.push_back(viewport);
	else
		m_viewports.back() = viewport;
}

const Viewport& GraphicsState::getViewport() const
{
	return m_viewports.back();
}

void GraphicsState::pushViewport(const Viewport& viewport)
{
	m_viewports.push_back(viewport);
}

void GraphicsState::popViewport()
{
	m_viewports.pop_back();
}

void GraphicsState::setScissor(const Scissor& scissor)
{
	if (m_scissors.empty())
		m_scissors.push_back(scissor);
	else
		m_scissors.back() = scissor;
}

const Scissor& GraphicsState::getScissor() const
{
	return m_scissors.back();
}

void GraphicsState::pushScissor(const Scissor& scissor)
{
	m_scissors.push_back(scissor);
}

void GraphicsState::popScissor()
{
	m_scissors.pop_back();
}

void GraphicsState::setVertexBuffer(ref<Buffer> buffer)
{
	if (!buffer->getUsageFlags().has(BufferUsageFlagBits::VertexBuffer))
		throw Exception("buffer is not for vertex");

	if (m_vertex_buffers.empty())
		m_vertex_buffers.push_back(buffer);
	else
		m_vertex_buffers.back() = buffer;
}

ref<Buffer> GraphicsState::getVertexBuffer() const
{
	return m_vertex_buffers.back();
}

void GraphicsState::pushVertexBuffer(ref<Buffer> buffer)
{
	if (!buffer->getUsageFlags().has(BufferUsageFlagBits::VertexBuffer))
		throw Exception("buffer is not for vertex");

	m_vertex_buffers.push_back(buffer);
}

void GraphicsState::popVertexBuffer()
{
	m_vertex_buffers.pop_back();
}

void GraphicsState::setIndexBuffer(ref<Buffer> buffer)
{
	if (!buffer->getUsageFlags().has(BufferUsageFlagBits::IndexBuffer))
		throw Exception("buffer is not for index");

	if (m_index_buffers.empty())
		m_index_buffers.push_back(buffer);
	else
		m_index_buffers.back() = buffer;
}

ref<Buffer> GraphicsState::getIndexBuffer() const
{
	return m_index_buffers.back();
}

void GraphicsState::pushIndexBuffer(ref<Buffer> buffer)
{
	if (!buffer->getUsageFlags().has(BufferUsageFlagBits::IndexBuffer))
		throw Exception("buffer is not for index");

	m_index_buffers.push_back(buffer);
}

void GraphicsState::popIndexBuffer()
{
	m_index_buffers.pop_back();
}

void GraphicsState::setRenderingInfo(const RenderingInfo& info)
{
	if (m_renderingInfos.empty())
		m_renderingInfos.push_back(info);
	else
		m_renderingInfos.back() = info;
}

const RenderingInfo& GraphicsState::getRenderingInfo() const
{
	return m_renderingInfos.back();
}

void GraphicsState::pushRenderingInfo(const RenderingInfo& info)
{
	m_renderingInfos.push_back(info);
}

void GraphicsState::popRenderingInfo()
{
	m_renderingInfos.pop_back();
}

void GraphicsState::setPipeline(ref<Pipeline> pipeline)
{
	if (m_pipelines.empty())
		m_pipelines.push_back(pipeline);
	else
		m_pipelines.back() = std::move(pipeline);
}

ref<Pipeline> GraphicsState::getPipeline() const
{
	return m_pipelines.back();
}

void GraphicsState::pushPipeline(ref<Pipeline> pipeline)
{
	m_pipelines.push_back(std::move(pipeline));
}

void GraphicsState::popPipelineInfo()
{
	m_pipelines.pop_back();
}

void GraphicsState::bindCommandBuffer(ref<CommandBuffer> cmd) const
{
	auto& cmd_impl = CoreObject::getImpl(cmd);

	if (!m_viewports.empty())
		cmd->setViewport(m_viewports.back());

	if (!m_scissors.empty())
		cmd->setScissor(m_scissors.back());

	if (!m_vertex_buffers.empty() && m_vertex_buffers.back() != cmd_impl.currentVertexBuffer)
		cmd->setVertexBuffer(m_vertex_buffers.back());

	if (!m_index_buffers.empty() && m_index_buffers.back() != cmd_impl.currentIndexBuffer)
		cmd->setIndexBuffer(m_index_buffers.back());

	if (!m_pipelines.empty() && m_pipelines.back() != cmd_impl.currentPipeline)
		cmd->setPipeline(m_pipelines.back());

	if (!m_renderingInfos.empty()) {
		if (cmd_impl.currentRenderingInfo.colorAttachments.empty()) {
			cmd->beginRendering(m_renderingInfos.back());
		} else if (!(m_renderingInfos.back() == cmd_impl.currentRenderingInfo)) {
			cmd->endRendering();
			cmd->beginRendering(m_renderingInfos.back());
		}
	}
}

void GraphicsState::clear()
{
	m_viewports.clear();
	m_scissors.clear();
	m_renderingInfos.clear();
	m_pipelines.clear();
}

VERA_NAMESPACE_END