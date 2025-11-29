#include "../../include/vera/core/command_stream.h"

VERA_NAMESPACE_BEGIN

CommandStream::CommandStream()
{
}

CommandStream::~CommandStream() VERA_NOEXCEPT
{
	// nothing to do
}

void CommandStream::addWaitSemaphore(obj<Semaphore> semaphore, PipelineStageFlags wait_stage_mask) VERA_NOEXCEPT
{
	m_wait.emplace_back(std::move(semaphore), wait_stage_mask );
}

void CommandStream::addSignalSemaphore(obj<Semaphore> semaphore) VERA_NOEXCEPT
{
	m_signal.emplace_back(std::move(semaphore));
}

uint64_t CommandStream::getId() const VERA_NOEXCEPT
{
	return m_id;
}

void CommandStream::wait() const VERA_NOEXCEPT
{
	m_sync.wait();
}

VERA_NAMESPACE_END
