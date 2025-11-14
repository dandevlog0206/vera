#pragma once

#include "device.h"
#include <span>

VERA_NAMESPACE_BEGIN

class Semaphore : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Semaphore)
public:
	static bool waitAll(std::span<obj<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);
	static bool waitAll(std::span<obj<Semaphore>> semaphores, uint64_t value, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<obj<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<obj<Semaphore>> semaphores, uint64_t value, uint64_t timeout = UINT64_MAX);

	static obj<Semaphore> create(obj<Device> device);
	static obj<Semaphore> createTimeline(obj<Device> device, uint64_t initial_value = 0);
	~Semaphore();

	obj<Device> getDevice();

	bool wait(uint64_t timeout = UINT64_MAX);

	// for timeline semaphore
	bool wait(uint64_t value, uint64_t timeout = UINT64_MAX);
	void signal(uint64_t value);
	uint64_t value();
};

VERA_NAMESPACE_END