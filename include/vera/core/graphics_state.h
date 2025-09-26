#pragma once

#include "render_command.h"

VERA_NAMESPACE_BEGIN

class GraphicsState
{
	friend class RenderContext;
	friend class RenderCommand;
public:
	GraphicsState();
	~GraphicsState();

	void setViewport(const Viewport& viewport);
	const Viewport& getViewport() const;
	void pushViewport(const Viewport& viewport);
	void popViewport();

	void setScissor(const Scissor& viewport);
	const Scissor& getScissor() const;
	void pushScissor(const Scissor& viewport);
	void popScissor();

	void setVertexBuffer(ref<Buffer> buffer);
	ref<Buffer> getVertexBuffer() const;
	void pushVertexBuffer(ref<Buffer> buffer);
	void popVertexBuffer();

	void setIndexBuffer(ref<Buffer> buffer);
	ref<Buffer> getIndexBuffer() const;
	void pushIndexBuffer(ref<Buffer> buffer);
	void popIndexBuffer();

	void setRenderingInfo(const RenderingInfo& info);
	const RenderingInfo& getRenderingInfo() const;
	void pushRenderingInfo(const RenderingInfo& info);
	void popRenderingInfo();

	void setPipeline(ref<Pipeline> pipeline);
	ref<Pipeline> getPipeline() const;
	void pushPipeline(ref<Pipeline> pipeline);
	void popPipelineInfo();

	void bindRenderCommand(ref<RenderCommand> cmd) const;

	void clear();

private:
	std::vector<Viewport>      m_viewports;
	std::vector<Scissor>       m_scissors;
	std::vector<ref<Buffer>>   m_vertex_buffers;
	std::vector<ref<Buffer>>   m_index_buffers;
	std::vector<RenderingInfo> m_renderingInfos;
	std::vector<ref<Pipeline>> m_pipelines;
};

VERA_NAMESPACE_END