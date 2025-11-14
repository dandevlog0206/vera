#pragma once

#include "semaphore.h"
#include "fence.h"

VERA_NAMESPACE_BEGIN

class CommandBufferImpl;

enum class CommandBufferState VERA_ENUM
{
	Unknown,     // command buffer is in an unknown state or destroyed
	Initialized, // command buffer created or reset and ready to begin recording
	Recording,   // command buffer is in recording state
	Executable,  // command buffer recording is ended and ready to be submitted
	Submitted,   // command buffer has been submitted to the queue
	Completed    // command buffer has completed execution
};

class CommandBufferSync
{
	friend class CommandBuffer;

	CommandBufferSync(const CommandBufferImpl* impl, uint64_t id) VERA_NOEXCEPT;
public:
	CommandBufferSync() VERA_NOEXCEPT;
	CommandBufferSync(const CommandBufferSync&) = default;

	VERA_NODISCARD obj<Semaphore> getCompleteSemaphore() const VERA_NOEXCEPT;
	VERA_NODISCARD obj<Fence> getCompleteFence() const VERA_NOEXCEPT;

	VERA_NODISCARD CommandBufferState getState() const VERA_NOEXCEPT;
	VERA_NODISCARD uint64_t getSumbitID() const VERA_NOEXCEPT;

	VERA_NODISCARD void waitForComplete() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isComplete() const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	const CommandBufferImpl* m_impl;
	uint64_t                 m_submit_id;
};

VERA_NAMESPACE_END