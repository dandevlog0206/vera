#pragma once

#include "object_impl.h"

#include "../../include/vera/core/device_memory.h"

VERA_NAMESPACE_BEGIN

enum class MemoryResourceType VERA_ENUM
{
	Buffer,
	Texture
};

struct MemoryResourceBind
{
	MemoryResourceType resourceType;
	size_t             size;
	size_t             offset;
	void*              resourcePtr;
};

struct DeviceMemoryImpl
{
	obj<Device>                     device;

	vk::DeviceMemory                memory;

	MemoryPropertyFlags             propertyFlags;
	std::vector<MemoryResourceBind> resourceBind;
	size_t                          allocated;
	uint32_t                        typeIndex;
	void*                           mapPtr;
};

static vk::AccessFlags to_vk_access_flags(AccessFlags flags)
{
	vk::AccessFlags result;

	if (flags.has(AccessFlagBits::IndirectCommandRead))
		result |= vk::AccessFlagBits::eIndirectCommandRead;
	if (flags.has(AccessFlagBits::IndexRead))
		result |= vk::AccessFlagBits::eIndexRead;
	if (flags.has(AccessFlagBits::VertexAttributeRead))
		result |= vk::AccessFlagBits::eVertexAttributeRead;
	if (flags.has(AccessFlagBits::UniformRead))
		result |= vk::AccessFlagBits::eUniformRead;
	if (flags.has(AccessFlagBits::InputAttachmentRead))
		result |= vk::AccessFlagBits::eInputAttachmentRead;
	if (flags.has(AccessFlagBits::ShaderRead))
		result |= vk::AccessFlagBits::eShaderRead;
	if (flags.has(AccessFlagBits::ShaderWrite))
		result |= vk::AccessFlagBits::eShaderWrite;
	if (flags.has(AccessFlagBits::ColorAttachmentRead))
		result |= vk::AccessFlagBits::eColorAttachmentRead;
	if (flags.has(AccessFlagBits::ColorAttachmentWrite))
		result |= vk::AccessFlagBits::eColorAttachmentWrite;
	if (flags.has(AccessFlagBits::DepthStencilAttachmentRead))
		result |= vk::AccessFlagBits::eDepthStencilAttachmentRead;
	if (flags.has(AccessFlagBits::DepthStencilAttachmentWrite))
		result |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	if (flags.has(AccessFlagBits::TransferRead))
		result |= vk::AccessFlagBits::eTransferRead;
	if (flags.has(AccessFlagBits::TransferWrite))
		result |= vk::AccessFlagBits::eTransferWrite;
	if (flags.has(AccessFlagBits::HostRead))
		result |= vk::AccessFlagBits::eHostRead;
	if (flags.has(AccessFlagBits::HostWrite))
		result |= vk::AccessFlagBits::eHostWrite;
	if (flags.has(AccessFlagBits::MemoryRead))
		result |= vk::AccessFlagBits::eMemoryRead;
	if (flags.has(AccessFlagBits::MemoryWrite))
		result |= vk::AccessFlagBits::eMemoryWrite;
	if (flags.has(AccessFlagBits::None))
		result |= vk::AccessFlagBits::eNone;

	return result;
}

VERA_NAMESPACE_END