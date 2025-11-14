#include "../../include/vera/graphics/graphics_state.h"

#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/pipeline.h"
#include "../../include/vera/core/buffer.h"

#define EMPTY_VIEWPORT Viewport{ .posX = INFINITY, .posY = INFINITY }
#define EMPTY_SCISSOR  Scissor{ .minX = UINT32_MAX, .minY = UINT32_MAX }

VERA_NAMESPACE_BEGIN

GraphicsState::GraphicsState() VERA_NOEXCEPT :
	m_viewport(EMPTY_VIEWPORT),
	m_scissor(EMPTY_SCISSOR),
	m_vertex_buffer(),
	m_index_buffer(),
	m_rendering_info(),
	m_pipeline() {}

GraphicsState::~GraphicsState()
{
	// nothing to do
}

void GraphicsState::setViewport(const Viewport& viewport) VERA_NOEXCEPT
{
	m_viewport = viewport;
}

std::optional<Viewport> GraphicsState::getViewport() const VERA_NOEXCEPT
{
	return m_viewport;
}

void GraphicsState::setScissor(const Scissor& scissor) VERA_NOEXCEPT
{
	m_scissor = scissor;
}

std::optional<Scissor> GraphicsState::getScissor() const VERA_NOEXCEPT
{
	return m_scissor;
}

void GraphicsState::setVertexBuffer(obj<Buffer> buffer) VERA_NOEXCEPT
{
	if (!buffer->getUsageFlags().has(BufferUsageFlagBits::VertexBuffer))
		throw Exception("buffer is not for vertex");

	m_vertex_buffer = std::move(buffer);
}

obj<Buffer> GraphicsState::getVertexBuffer() const VERA_NOEXCEPT
{
	return m_vertex_buffer;
}

void GraphicsState::setIndexBuffer(obj<Buffer> buffer) VERA_NOEXCEPT
{
	if (!buffer->getUsageFlags().has(BufferUsageFlagBits::IndexBuffer))
		throw Exception("buffer is not for index");

	m_index_buffer = std::move(buffer);
}

obj<Buffer> GraphicsState::getIndexBuffer() const VERA_NOEXCEPT
{
	return m_index_buffer;
}

void GraphicsState::setRenderingInfo(const RenderingInfo& info) VERA_NOEXCEPT
{
	m_rendering_info = info;
}

const RenderingInfo& GraphicsState::getRenderingInfo() const VERA_NOEXCEPT
{
	return m_rendering_info;
}

void GraphicsState::setPipeline(obj<Pipeline> pipeline) VERA_NOEXCEPT
{
	m_pipeline = std::move(pipeline);
}

obj<Pipeline> GraphicsState::getPipeline() const VERA_NOEXCEPT
{
	return m_pipeline;
}

void GraphicsState::clear()
{
	m_viewport       = EMPTY_VIEWPORT;
	m_scissor        = EMPTY_SCISSOR;
	m_vertex_buffer  = {};
	m_index_buffer   = {};
	m_rendering_info = {};
	m_pipeline       = {};
}

VERA_NAMESPACE_END