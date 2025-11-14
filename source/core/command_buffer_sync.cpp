#include "../../include/vera/core/command_buffer_sync.h"
#include "../impl/command_buffer_impl.h"

#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/semaphore.h"

VERA_NAMESPACE_BEGIN

CommandBufferSync::CommandBufferSync(const CommandBufferImpl* impl, uint64_t id) VERA_NOEXCEPT :
	m_impl(impl),
	m_submit_id(id) {}

CommandBufferSync::CommandBufferSync() VERA_NOEXCEPT :
	m_impl(nullptr),
	m_submit_id(0) {}

obj<Semaphore> CommandBufferSync::getCompleteSemaphore() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");

	if (m_impl->submitID == m_submit_id)
		return m_impl->semaphore;

	// next recording already begun
	return nullptr;
}

obj<Fence> CommandBufferSync::getCompleteFence() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");

	return m_impl->fence;
}

CommandBufferState CommandBufferSync::getState() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");

	if (m_submit_id < m_impl->submitID || m_impl->fence->signaled())
		return CommandBufferState::Completed;

	return m_impl->state;
}

uint64_t CommandBufferSync::getSumbitID() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");
	return m_submit_id;
}

void CommandBufferSync::waitForComplete() const VERA_NOEXCEPT
{
	if (!m_impl || m_submit_id < m_impl->submitID || m_impl->fence->signaled())
		return;

	m_impl->fence->wait();
}

bool CommandBufferSync::isComplete() const VERA_NOEXCEPT
{
	if (!m_impl) return false;
	return m_submit_id < m_impl->submitID || m_impl->fence->signaled();
}

bool CommandBufferSync::empty() const VERA_NOEXCEPT
{
	return !m_impl;
}

VERA_NAMESPACE_END