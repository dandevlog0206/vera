#pragma once

#include "core_object.h"
#include <span>

VERA_NAMESPACE_BEGIN

class Device;

class Fence : protected CoreObject
{
	VERA_CORE_OBJECT(Fence)
public:
	static bool waitAll(std::span<ref<Fence>> fences, uint64_t timeout = UINT64_MAX);
	static bool waitAny(std::span<ref<Fence>> fences, uint64_t timeout = UINT64_MAX);

	static ref<Fence> create(ref<Device> device, bool signaled = false);
	~Fence();

	bool signaled() const;
	void reset();
	bool wait(uint64_t timeout = UINT64_MAX) const;
};

VERA_NAMESPACE_END