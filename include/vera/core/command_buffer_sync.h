#pragma once

#include "intrusive_ptr.h"

VERA_NAMESPACE_BEGIN

class Semaphore;
struct CommandBufferImpl;

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

	CommandBufferSync(const CommandBufferImpl* impl, uint64_t id);
public:
	CommandBufferSync() = default;
	CommandBufferSync(const CommandBufferSync&) = default;

	VERA_NODISCARD CommandBufferState getState() const VERA_NOEXCEPT;

	VERA_NODISCARD const_ref<Semaphore> getCompleteSemaphore() const VERA_NOEXCEPT;

	VERA_NODISCARD void waitForComplete() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isComplete() const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	const CommandBufferImpl* m_impl;
	uint64_t                 m_submit_id;
};

VERA_NAMESPACE_END