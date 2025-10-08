#include "../../include/vera/core/framebuffer.h"
#include "../impl/framebuffer_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/semaphore.h"
#include "../../include/vera/graphics/format_traits.h"

VERA_NAMESPACE_BEGIN

obj<FrameBuffer> FrameBuffer::create(obj<Device> device, const FrameBufferCreateInfo& info)
{
	auto  obj            = createNewCoreObject<FrameBuffer>();
	auto& impl           = getImpl(obj);
	auto  depth_format   = static_cast<Format>(info.depthFormat);
	auto  stencil_format = static_cast<Format>(info.stencilFormat);

	// TODO: implement later

	TextureCreateInfo color_info = {

	};

	impl.colorAttachment = Texture::create(device, color_info);
	
	bool has_depth_ds     = format_has_stencil(depth_format);
	bool has_stencil_ds   = format_has_depth(stencil_format);
	bool is_depth_stencil = has_depth_ds || has_stencil_ds;

	VERA_ASSERT_MSG(!(has_depth_ds && has_stencil_ds) || depth_format == stencil_format,
		"if depth format and stencil format both are depth-stencil format, both format must be identical");

	if (is_depth_stencil) {
		impl.depthAttachment = Texture::createDepth(device, info.width, info.height, info.depthFormat);
	} else {
		if (info.depthFormat != DepthFormat::Unknown)
			impl.depthAttachment = Texture::createDepth(device, info.width, info.height, info.depthFormat);

		if (info.stencilFormat != StencilFormat::Unknown)
			impl.stencilAttachment = Texture::createStencil(device, info.width, info.height, info.stencilFormat);
	}

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
	auto& impl      = getImpl(this);
	auto  has_depth = format_has_depth(static_cast<Format>(impl.stencilFormat));

	return has_depth ? impl.depthAttachment : impl.stencilAttachment;
}

void FrameBuffer::waitForComplete() const
{
	auto& sync = getImpl(this).commandBufferSync;

	if (!sync.empty())
		sync.waitForComplete();
}

bool FrameBuffer::isComplete() const
{
	auto& sync = getImpl(this).commandBufferSync;
	
	return !sync.empty() || sync.isComplete();
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
