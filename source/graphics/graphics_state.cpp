#include "../../include/vera/graphics/graphics_state.h"
#include "../impl/command_buffer_impl.h"

VERA_NAMESPACE_BEGIN

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

void GraphicsState::clear()
{
	m_viewports.clear();
	m_scissors.clear();
	m_renderingInfos.clear();
	m_pipelines.clear();
}

VERA_NAMESPACE_END