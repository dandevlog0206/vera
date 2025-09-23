#pragma once

#include "object_impl.h"

#include "../../include/vera/core/buffer.h"

VERA_NAMESPACE_BEGIN

class DeviceMemory;

struct BufferImpl
{
	ref<Device>       device;
	ref<DeviceMemory> memory;

	vk::Buffer        buffer;

	BufferUsageFlags  usage;
};

struct BufferViewImpl
{
	ref<Device>    device;
	ref<Buffer>    buffer;

	vk::BufferView bufferView;
};

static vk::BufferUsageFlags to_vk_buffer_usage_flags(BufferUsageFlags flags)
{
	vk::BufferUsageFlags result;

	if (flags.has(BufferUsageFlagBits::TransferSrc))
		result |= vk::BufferUsageFlagBits::eTransferSrc;
	if (flags.has(BufferUsageFlagBits::TransferDst))
		result |= vk::BufferUsageFlagBits::eTransferDst;
	if (flags.has(BufferUsageFlagBits::UniformTexelBuffer))
		result |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
	if (flags.has(BufferUsageFlagBits::StorageTexelBuffer))
		result |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
	if (flags.has(BufferUsageFlagBits::UniformBuffer))
		result |= vk::BufferUsageFlagBits::eUniformBuffer;
	if (flags.has(BufferUsageFlagBits::StorageBuffer))
		result |= vk::BufferUsageFlagBits::eStorageBuffer;
	if (flags.has(BufferUsageFlagBits::IndexBuffer))
		result |= vk::BufferUsageFlagBits::eIndexBuffer;
	if (flags.has(BufferUsageFlagBits::VertexBuffer))
		result |= vk::BufferUsageFlagBits::eVertexBuffer;
	if (flags.has(BufferUsageFlagBits::IndirectBuffer))
		result |= vk::BufferUsageFlagBits::eIndirectBuffer;

	return result;
}

VERA_NAMESPACE_END