#include "../../include/vera/core/buffer_view.h"
#include "../impl/buffer_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/buffer.h"

VERA_NAMESPACE_BEGIN

const vk::BufferView& get_vk_buffer_view(const_ref<BufferView> buffer_view)
{
	return CoreObject::getImpl(buffer_view).bufferView;
}

vk::BufferView& get_vk_buffer_view(ref<BufferView> buffer_view)
{
	return CoreObject::getImpl(buffer_view).bufferView;
}

obj<BufferView> BufferView::create(obj<Buffer> buffer, const BufferViewCreateInfo& info)
{
	auto obj          = createNewCoreObject<BufferView>();
	auto& impl        = getImpl(obj);
	auto& buffer_impl = getImpl(buffer);
	auto  vk_device   = get_vk_device(buffer_impl.device);

	vk::BufferViewCreateInfo view_info;
	view_info.buffer = buffer_impl.buffer;
	view_info.format = to_vk_format(info.format);
	view_info.offset = info.offset;
	view_info.range  = info.size;

	impl.device     = buffer_impl.device;
	impl.buffer     = std::move(buffer);
	impl.bufferView = vk_device.createBufferView(view_info);
	impl.format     = info.format;
	impl.offset     = info.offset;
	impl.size       = info.size;

	return obj;
}

BufferView::~BufferView()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.destroy(impl.bufferView);

	destroyObjectImpl(this);
}

obj<Device> BufferView::getDevice()
{
	return getImpl(this).device;
}

obj<Buffer> BufferView::getBuffer()
{
	return getImpl(this).buffer;
}

Format BufferView::getFormat() const
{
	return getImpl(this).format;
}

size_t BufferView::size()
{
	return getImpl(this).size;
}

size_t BufferView::offset()
{
	return getImpl(this).offset;
}

VERA_NAMESPACE_END