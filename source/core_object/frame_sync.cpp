#include "../../include/vera/core/frame_sync.h"
#include "../impl/render_context_impl.h"

#include "../../include/vera/core/fence.h"

VERA_NAMESPACE_BEGIN

FrameSync::FrameSync(const RenderFrame& ctx_impl, uint64_t frame_id) :
	m_render_frame(&ctx_impl), m_frame_id(frame_id) {}

ref<Semaphore> FrameSync::getRenderCompleteSemaphore() const
{
	VERA_ASSERT(m_render_frame);

	if (m_render_frame->frameID == m_frame_id)
		return m_render_frame->renderCompleteSemaphore;

	return {};
}

void FrameSync::waitForRenderComplete() const
{
	VERA_ASSERT(m_render_frame);

	if (m_render_frame->frameID == m_frame_id)
		m_render_frame->fence->wait();
}

bool FrameSync::isRenderComplete() const
{
	return m_render_frame->frameID == m_frame_id ? m_render_frame->fence->signaled() : true;
}

bool FrameSync::empty() const
{
	return !m_render_frame;
}

VERA_NAMESPACE_END