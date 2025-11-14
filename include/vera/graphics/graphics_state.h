#pragma once

#include "../core/command_buffer.h"
#include <map>

VERA_NAMESPACE_BEGIN

class GraphicsState
{
	friend class RenderContext;
	friend class CommandBuffer;
public:
	GraphicsState() VERA_NOEXCEPT;
	~GraphicsState();

	void setViewport(const Viewport& viewport) VERA_NOEXCEPT;
	VERA_NODISCARD std::optional<Viewport> getViewport() const VERA_NOEXCEPT;

	void setScissor(const Scissor& viewport) VERA_NOEXCEPT;
	VERA_NODISCARD std::optional<Scissor> getScissor() const VERA_NOEXCEPT;

	void setVertexBuffer(obj<Buffer> buffer) VERA_NOEXCEPT;
	VERA_NODISCARD obj<Buffer> getVertexBuffer() const VERA_NOEXCEPT;

	void setIndexBuffer(obj<Buffer> buffer) VERA_NOEXCEPT;
	VERA_NODISCARD obj<Buffer> getIndexBuffer() const VERA_NOEXCEPT;

	void setRenderingInfo(const RenderingInfo& info) VERA_NOEXCEPT;
	VERA_NODISCARD const RenderingInfo& getRenderingInfo() const VERA_NOEXCEPT;

	void setPipeline(obj<Pipeline> pipeline) VERA_NOEXCEPT;
	VERA_NODISCARD obj<Pipeline> getPipeline() const VERA_NOEXCEPT;

	void clear();

private:
	Viewport      m_viewport;
	Scissor       m_scissor;
	obj<Buffer>   m_vertex_buffer;
	obj<Buffer>   m_index_buffer;
	RenderingInfo m_rendering_info;
	obj<Pipeline> m_pipeline;
};

VERA_NAMESPACE_END