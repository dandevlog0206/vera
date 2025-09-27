#pragma once

#include "core_object.h"
#include <span>

VERA_NAMESPACE_BEGIN

class Device;

class Semaphore : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Semaphore)
public:
	static bool waitAll(std::span<obj<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<obj<Semaphore>> semaphores, uint64_t timeout = UINT64_MAX);

	static obj<Semaphore> create(obj<Device> device);
	~Semaphore();

	obj<Device> getDevice();

	void signal(uint64_t value);
	uint64_t value();
	bool wait(uint64_t timeout = UINT64_MAX);
};

VERA_NAMESPACE_END