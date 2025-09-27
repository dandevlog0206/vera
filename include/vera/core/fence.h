#pragma once

#include "core_object.h"
#include <span>

VERA_NAMESPACE_BEGIN

class Device;

class Fence : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Fence)
public:
	static bool waitAll(std::span<obj<Fence>> fences, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<obj<Fence>> fences, uint64_t timeout = UINT64_MAX);

	static obj<Fence> create(obj<Device> device, bool signaled = false);
	~Fence();

	obj<Device> getDevice();

	bool signaled() const;
	void reset();
	bool wait(uint64_t timeout = UINT64_MAX) const;
};

VERA_NAMESPACE_END