#include "../../include/vera/core/command_sync.h"
#include "../impl/command_buffer_impl.h"

#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/semaphore.h"

VERA_NAMESPACE_BEGIN

CommandSync::CommandSync(
	std::shared_ptr<CommandBufferTracker> tracker,
	uint64_t                              id) VERA_NOEXCEPT :
	m_tracker(tracker),
	m_submit_id(id) {}

CommandSync::CommandSync() VERA_NOEXCEPT :
	m_tracker(nullptr),
	m_submit_id(0) {}

obj<Semaphore> CommandSync::getCompleteSemaphore() const VERA_NOEXCEPT
{
	return m_tracker && m_tracker->submitID == m_submit_id ? m_tracker->semaphore : nullptr;
}

obj<Fence> CommandSync::getCompleteFence() const VERA_NOEXCEPT
{
	return m_tracker ? m_tracker->fence : nullptr;
}

CommandBufferState CommandSync::getState() const VERA_NOEXCEPT
{
	if (!m_tracker)
		return CommandBufferState::Invalid;
	if (m_submit_id < m_tracker->submitID || m_tracker->fence->signaled())
		return CommandBufferState::Complete;
	return m_tracker->state;
}

uint64_t CommandSync::getSubmitID() const VERA_NOEXCEPT
{
	return m_tracker ? m_submit_id : 0;
}

void CommandSync::wait() const VERA_NOEXCEPT
{
	if (!m_tracker || m_submit_id < m_tracker->submitID || m_tracker->fence->signaled())
		return;

	m_tracker->fence->wait();
}

bool CommandSync::isComplete() const VERA_NOEXCEPT
{
	if (!m_tracker) return false;
	return m_submit_id < m_tracker->submitID || m_tracker->fence->signaled();
}

bool CommandSync::empty() const VERA_NOEXCEPT
{
	return !m_tracker;
}

VERA_NAMESPACE_END