#pragma once

#include "fence.h"
#include "semaphore.h"
#include "command_sync.h"
#include "../util/small_vector.h"
#include <mutex>

VERA_NAMESPACE_BEGIN

class CommandStream
{
	friend class CommandBuffer;
public:
	CommandStream();
	~CommandStream() VERA_NOEXCEPT;

	void addWaitSemaphore(obj<Semaphore> semaphore, PipelineStageFlags wait_stage_mask = {}) VERA_NOEXCEPT;
	void addSignalSemaphore(obj<Semaphore> semaphore) VERA_NOEXCEPT;

	VERA_NODISCARD uint64_t getId() const VERA_NOEXCEPT;

	void wait() const VERA_NOEXCEPT;

private:
	struct WaitInfo
	{
		obj<Semaphore>     semaphore;
		PipelineStageFlags waitStageMask;
	};

	small_vector<WaitInfo>       m_wait;
	small_vector<obj<Semaphore>> m_signal;
	CommandSync                  m_sync;
	mutable std::mutex           m_mutex;
	uint64_t                     m_id;
};

VERA_NAMESPACE_END
