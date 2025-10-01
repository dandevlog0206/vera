#include "../../include/vera/core/framebuffer.h"
#include "../impl/framebuffer_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/semaphore.h"

VERA_NAMESPACE_BEGIN

obj<FrameBuffer> FrameBuffer::create(obj<Device> device, const FrameBufferCreateInfo& info)
{
	auto  obj  = createNewObject<FrameBuffer>();
	auto& impl = getImpl(obj);

	// TODO: implement later

	TextureCreateInfo color_info = {

	};

	impl.colorAttachment = Texture::create(device, color_info);

	return obj;
}

FrameBuffer::~FrameBuffer()
{
	auto& impl = getImpl(this);

	// nothing to destroy

	destroyObjectImpl(this);
}

obj<Device> FrameBuffer::getDevice()
{
	return getImpl(this).device;
}

ref<Texture> FrameBuffer::getTexture()
{
	return getImpl(this).colorAttachment;
}

ref<Texture> FrameBuffer::getDepthTexture()
{
	return getImpl(this).depthAttachment;
}

ref<Texture> FrameBuffer::getStencilTexture()
{
	return getImpl(this).stencilAttachment;
}

void FrameBuffer::waitForComplete() const
{
	auto& sync = getImpl(this).frameSync;

	if (!sync.empty())
		sync.waitForRenderComplete();
}

bool FrameBuffer::isComplete() const
{
	auto& sync = getImpl(this).frameSync;
	
	return !sync.empty() || sync.isRenderComplete();
}

uint32_t FrameBuffer::width() const
{
	return getImpl(this).width;
}

uint32_t FrameBuffer::height() const
{
	return getImpl(this).height;
}

float FrameBuffer::aspect() const
{
	auto& impl = getImpl(this);
	return impl.width / impl.height;
}

Format FrameBuffer::getFormat() const
{
	return getImpl(this).format;
}

DepthFormat FrameBuffer::getDepthFormat() const
{
	return getImpl(this).depthFormat;
}

StencilFormat FrameBuffer::getStencilFormat() const
{
	return getImpl(this).stencilFormat;
}

VERA_NAMESPACE_END
