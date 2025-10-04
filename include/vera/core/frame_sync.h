#pragma once

#include "semaphore.h"

VERA_NAMESPACE_BEGIN

class FrameSync
{
	friend class RenderContext;
	friend struct RenderFrame;

	FrameSync(const RenderFrame& ctx_impl, uint64_t frame_id);

public:
	FrameSync() = default;

	ref<Semaphore> getRenderCompleteSemaphore() const;

	void waitForRenderComplete() const;
	bool isRenderComplete() const;

	bool empty() const;

private:
	const RenderFrame* m_render_frame;
	uint64_t           m_frame_id;
};

VERA_NAMESPACE_END
