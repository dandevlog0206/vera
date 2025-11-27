#pragma once

#include "core_object.h"

VERA_NAMESPACE_BEGIN

class Device;

class Stream : public CoreObject
{
	VERA_CORE_OBJECT_INIT(Stream)
public:
	static obj<Stream> create(obj<Device> device);
	~Stream() VERA_NOEXCEPT override;
	
	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;
};

VERA_NAMESPACE_END
