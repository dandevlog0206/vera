#pragma once

#include "device.h"
#include <span>

VERA_NAMESPACE_BEGIN

class Semaphore : public CoreObject
{
	VERA_CORE_OBJECT_INIT(Semaphore)
public:
	static bool waitAll(std::span<obj<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<obj<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);

	static obj<Semaphore> create(obj<Device> device);
	~Semaphore() VERA_NOEXCEPT override;

	obj<Device> getDevice();

	bool wait(uint64_t timeout = UINT64_MAX) const;
};

class TimelineSemaphore : public CoreObject
{
	VERA_CORE_OBJECT_INIT(TimelineSemaphore)
public:
	static bool waitAll(std::span<obj<Semaphore>> semaphores, uint64_t value, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<obj<Semaphore>> semaphores, uint64_t value, uint64_t timeout = UINT64_MAX);

	static obj<TimelineSemaphore> create(obj<Device> device, uint64_t initial_value = 0);
	~TimelineSemaphore() VERA_NOEXCEPT override;

	obj<Device> getDevice();

	bool wait(uint64_t value, uint64_t timeout = UINT64_MAX) const;
	void signal(uint64_t value);
	VERA_NODISCARD uint64_t value() const;
};

VERA_NAMESPACE_END