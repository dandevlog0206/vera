#pragma once

#include "core_object.h"
#include <span>

VERA_NAMESPACE_BEGIN

class Device;

class Semaphore : protected CoreObject
{
	VERA_CORE_OBJECT(Semaphore)
public:
	static bool waitAll(std::span<ref<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<ref<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);

	static ref<Semaphore> create(ref<Device> device);

	void signal(uint64_t value);
	uint64_t value();
	bool wait(uint64_t timeout = UINT64_MAX);
};

VERA_NAMESPACE_END