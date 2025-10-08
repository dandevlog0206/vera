#include "../../include/vera/core/command_buffer_sync.h"
#include "../impl/command_buffer_impl.h"

#include "../../include/vera/core/fence.h"
#include "../../include/vera/core/semaphore.h"

VERA_NAMESPACE_BEGIN

CommandBufferSync::CommandBufferSync(const CommandBufferImpl* impl, uint64_t id) :
	m_impl(impl),
	m_submit_id(id) {}

CommandBufferState CommandBufferSync::getState() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");

	if (m_submit_id < m_impl->submitID || m_impl->fence->signaled())
		return CommandBufferState::Completed;

	return m_impl->state;
}

ref<Semaphore> CommandBufferSync::getCompleteSemaphore()
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");

	if (m_submit_id < m_impl->submitID || m_impl->fence->signaled())
		throw Exception("command buffer already completed");

	return m_impl->semaphore;
}

void CommandBufferSync::waitForComplete() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");

	if (m_submit_id < m_impl->submitID || m_impl->fence->signaled())
		return;

	m_impl->fence->wait();
}

bool CommandBufferSync::isComplete() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl, "empty command buffer sync");

	return m_submit_id < m_impl->submitID || m_impl->fence->signaled();
}

bool CommandBufferSync::empty() const VERA_NOEXCEPT
{
	return !m_impl;
}

VERA_NAMESPACE_END