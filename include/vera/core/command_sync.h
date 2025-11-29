#pragma once

#include "semaphore.h"
#include "fence.h"
#include <memory>

VERA_NAMESPACE_BEGIN

class CommandBuffer;
struct CommandBufferTracker;

enum class CommandBufferState VERA_ENUM
{
	// The command buffer is in an uninitialized or invalid state.
	// This is the default state before allocation or after being destroyed.
	Invalid,

	// The command buffer has been allocated and is ready to be recorded.
	// A command buffer enters this state after being reset.
	Initial,

	// The command buffer is actively being recorded, between vkBeginCommandBuffer
	// and vkEndCommandBuffer.
	Recording,

	// The command buffer has finished recording (vkEndCommandBuffer has been called)
	// and is ready to be submitted to a queue.
	Executable,

	// The command buffer has been submitted to a queue via vkQueueSubmit and is
	// either pending execution or currently being executed by the GPU.
	Pending,

	// The command buffer has completed execution on the GPU. This state is
	// typically determined by waiting on a fence.
	Complete
};

class CommandSync
{
	friend class CommandBuffer;

	CommandSync(
		std::shared_ptr<CommandBufferTracker> tracker,
		uint64_t                              id) VERA_NOEXCEPT;
public:
	CommandSync() VERA_NOEXCEPT;
	CommandSync(const CommandSync&) = default;

	VERA_NODISCARD obj<Semaphore> getCompleteSemaphore() const VERA_NOEXCEPT;
	VERA_NODISCARD obj<Fence> getCompleteFence() const VERA_NOEXCEPT;

	VERA_NODISCARD CommandBufferState getState() const VERA_NOEXCEPT;
	VERA_NODISCARD uint64_t getSubmitID() const VERA_NOEXCEPT;

	// Waits until the command buffer has completed execution.
	VERA_NODISCARD void wait() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isComplete() const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	std::shared_ptr<CommandBufferTracker> m_tracker;
	uint64_t                              m_submit_id;
};

VERA_NAMESPACE_END