#pragma once

#include "core_object.h"
#include "../graphics/format.h"

VERA_NAMESPACE_BEGIN

class Device;
class Texture;

struct FrameBufferCreateInfo
{
	uint32_t      width;
	uint32_t      height;

	Format        format        = Format::Unknown;
	DepthFormat   depthFormat   = DepthFormat::Unknown;
	StencilFormat stencilFormat = StencilFormat::Unknown;
};

class FrameBuffer : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(FrameBuffer)
public:
	static obj<FrameBuffer> create(obj<Device> device, const FrameBufferCreateInfo& info);
	~FrameBuffer();

	obj<Device> getDevice();
	ref<Texture> getTexture();
	ref<Texture> getDepthTexture();
	ref<Texture> getStencilTexture();

	void waitForComplete() const;
	bool isComplete() const;

	uint32_t width() const;
	uint32_t height() const;
	float aspect() const;

	Format getFormat() const;
	DepthFormat getDepthFormat() const;
	StencilFormat getStencilFormat() const;
};

VERA_NAMESPACE_END