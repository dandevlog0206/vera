#pragma once

#include "include_vulkan.h"
#include "../../include/vera/core/core_object.h"
#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/enum_types.h"
#include "../../include/vera/util/rect.h"
#include "../../include/vera/util/hash.h"

VERA_NAMESPACE_BEGIN

// list of core object ordered by dependency and importance
class Context;
	class Device;
		// Pipeline
		class Pipeline;
		class PipelineLayout;
		
		// Shader
		class Shader;
		class ShaderReflection;
		class ProgramReflection;
		class ShaderParameter;
		
		// Resource Management
		class DescriptorSetLayout;
		class DescriptorPool;
		class DescriptorSet;

		// Rendering
		class RenderContext;
		class CommandBuffer;
		
		// Syncronization
		class Fence;
		class Semaphore;
		class TimelineSemaphore;

		// Resources
		class Swapchain;
		class FrameBuffer;
		class Sampler;
		class DeviceMemory;
		class Buffer;
		class BufferView;
		class Texture;
		class TextureView;

const vk::Instance& get_vk_instance(const_ref<Context> context) VERA_NOEXCEPT;
vk::Instance& get_vk_instance(ref<Context> context) VERA_NOEXCEPT;
const vk::Device& get_vk_device(const_ref<Device> device) VERA_NOEXCEPT;
vk::Device& get_vk_device(ref<Device> device) VERA_NOEXCEPT;
const vk::Pipeline& get_vk_pipeline(const_ref<Pipeline> pipeline) VERA_NOEXCEPT;
vk::Pipeline& get_vk_pipeline(ref<Pipeline> pipeline) VERA_NOEXCEPT;
const vk::PipelineLayout& get_vk_pipeline_layout(const_ref<PipelineLayout> pipeline_layout) VERA_NOEXCEPT;
vk::PipelineLayout& get_vk_pipeline_layout(ref<PipelineLayout> pipeline_layout) VERA_NOEXCEPT;
const vk::ShaderModule& get_vk_shader_module(const_ref<Shader> shader) VERA_NOEXCEPT;
vk::ShaderModule& get_vk_shader_module(ref<Shader> shader) VERA_NOEXCEPT;
const vk::CommandBuffer& get_vk_command_buffer(const_ref<CommandBuffer> cmd_buffer) VERA_NOEXCEPT;
vk::CommandBuffer& get_vk_command_buffer(ref<CommandBuffer> cmd_buffer) VERA_NOEXCEPT;
const vk::DescriptorSetLayout& get_vk_descriptor_set_layout(const_ref<DescriptorSetLayout> set_layout) VERA_NOEXCEPT;
vk::DescriptorSetLayout& get_vk_descriptor_set_layout(ref<DescriptorSetLayout> set_layout) VERA_NOEXCEPT;
const vk::DescriptorPool& get_vk_descriptor_pool(const_ref<DescriptorPool> descriptor_pool) VERA_NOEXCEPT;
vk::DescriptorPool& get_vk_descriptor_pool(ref<DescriptorPool> descriptor_pool) VERA_NOEXCEPT;
const vk::DescriptorSet& get_vk_descriptor_set(const_ref<DescriptorSet> descriptor_set) VERA_NOEXCEPT;
vk::DescriptorSet& get_vk_descriptor_set(ref<DescriptorSet> descriptor_set) VERA_NOEXCEPT;
const vk::Fence& get_vk_fence(const_ref<Fence> fence) VERA_NOEXCEPT;
vk::Fence& get_vk_fence(ref<Fence> fence) VERA_NOEXCEPT;
const vk::Semaphore& get_vk_semaphore(const_ref<Semaphore> semaphore) VERA_NOEXCEPT;
vk::Semaphore& get_vk_semaphore(ref<Semaphore> semaphore) VERA_NOEXCEPT;
const vk::Semaphore& get_vk_semaphore(const_ref<TimelineSemaphore> timeline_semaphore) VERA_NOEXCEPT;
vk::Semaphore& get_vk_semaphore(ref<TimelineSemaphore> timeline_semaphore) VERA_NOEXCEPT;
const vk::Sampler& get_vk_sampler(const_ref<Sampler> sampler) VERA_NOEXCEPT;
vk::Sampler& get_vk_sampler(ref<Sampler> sampler) VERA_NOEXCEPT;
const vk::DeviceMemory& get_vk_device_memory(const_ref<DeviceMemory> device_memory) VERA_NOEXCEPT;
vk::DeviceMemory& get_vk_device_memory(ref<DeviceMemory> device_memory) VERA_NOEXCEPT;
const vk::Buffer& get_vk_buffer(const_ref<Buffer> buffer) VERA_NOEXCEPT;
vk::Buffer& get_vk_buffer(ref<Buffer> buffer) VERA_NOEXCEPT;
const vk::BufferView& get_vk_buffer_view(const_ref<BufferView> buffer_view) VERA_NOEXCEPT;
vk::BufferView& get_vk_buffer_view(ref<BufferView> buffer_view) VERA_NOEXCEPT;
const vk::Image& get_vk_image(const_ref<Texture> texture) VERA_NOEXCEPT;
vk::Image& get_vk_image(ref<Texture> texture) VERA_NOEXCEPT;
const vk::ImageView& get_vk_image_view(const_ref<TextureView> texture_view) VERA_NOEXCEPT;
vk::ImageView& get_vk_image_view(ref<TextureView> texture_view) VERA_NOEXCEPT;

static vk::DeviceFaultAddressTypeEXT to_vk_device_fault_address_type(DeviceFaultAddressType type) VERA_NOEXCEPT
{
	// vr::DeviceFaultAddressType is VERA_VK_ABI_COMPATIBLE with vk::DeviceFaultAddressTypeEXT
	return std::bit_cast<vk::DeviceFaultAddressTypeEXT>(type);
}

static vk::MemoryHeapFlags to_vk_memory_heap_flags(MemoryHeapFlags flags) VERA_NOEXCEPT
{
	// vr::MemoryHeapFlags is VERA_VK_ABI_COMPATIBLE with vk::MemoryHeapFlags
	return std::bit_cast<vk::MemoryHeapFlags>(flags);
}

static vk::MemoryPropertyFlags to_vk_memory_property_flags(MemoryPropertyFlags flags) VERA_NOEXCEPT
{
	// vr::MemoryPropertyFlags is VERA_VK_ABI_COMPATIBLE with vk::MemoryPropertyFlags
	return std::bit_cast<vk::MemoryPropertyFlags>(flags);
}

static vk::PipelineBindPoint to_vk_pipeline_bind_point(PipelineBindPoint bind_point) VERA_NOEXCEPT
{
	switch (bind_point) {
	case PipelineBindPoint::Graphics: return vk::PipelineBindPoint::eGraphics;
	case PipelineBindPoint::Compute:  return vk::PipelineBindPoint::eCompute;
	}

	VERA_ASSERT_MSG(false, "invalid pipeline bind point");
	return {};
}

static vk::PrimitiveTopology to_vk_primitive_topology(PrimitiveTopology topology) VERA_NOEXCEPT
{
	// vr::PrimitiveTopology is VERA_VK_ABI_COMPATIBLE with vk::PrimitiveTopology
	return std::bit_cast<vk::PrimitiveTopology>(topology);
}

static vk::PolygonMode to_vk_polygon_mode(PolygonMode mode) VERA_NOEXCEPT
{
	// vr::PolygonMode is VERA_VK_ABI_COMPATIBLE with vk::PolygonMode
	return std::bit_cast<vk::PolygonMode>(mode);
}

static vk::CullModeFlags to_vk_cull_mode_flags(CullModeFlags flags) VERA_NOEXCEPT
{
	// vr::CullModeFlags is VERA_VK_ABI_COMPATIBLE with vk::CullModeFlags
	return std::bit_cast<vk::CullModeFlags>(flags);
}

static vk::FrontFace to_vk_front_face(FrontFace mode) VERA_NOEXCEPT
{
	// vr::FrontFace is VERA_VK_ABI_COMPATIBLE with vk::FrontFace
	return std::bit_cast<vk::FrontFace>(mode);
}

static vk::CompareOp to_vk_compare_op(CompareOp op) VERA_NOEXCEPT
{
	// vr::CompareOp is VERA_VK_ABI_COMPATIBLE with vk::CompareOp
	return std::bit_cast<vk::CompareOp>(op);
}

static vk::StencilOp to_vk_stencil_op(StencilOp op) VERA_NOEXCEPT
{
	// vr::StencilOp is VERA_VK_ABI_COMPATIBLE with vk::StencilOp
	return std::bit_cast<vk::StencilOp>(op);
}

static vk::LogicOp to_vk_logic_op(LogicOp op) VERA_NOEXCEPT
{
	// vr::LogicOp is VERA_VK_ABI_COMPATIBLE with vk::LogicOp
	return std::bit_cast<vk::LogicOp>(op);
}

static vk::BlendFactor to_vk_blend_factor(BlendFactor factor) VERA_NOEXCEPT
{
	// vr::BlendFactor is VERA_VK_ABI_COMPATIBLE with vk::BlendFactor
	return std::bit_cast<vk::BlendFactor>(factor);
}

static vk::BlendOp to_vk_blend_op(BlendOp op) VERA_NOEXCEPT
{
	// vr::BlendOp is VERA_VK_ABI_COMPATIBLE with vk::BlendOp
	return std::bit_cast<vk::BlendOp>(op);
}

static vk::ColorComponentFlags to_vk_color_component_flags(ColorComponentFlags flags) VERA_NOEXCEPT
{
	// vr::ColorComponentFlags is VERA_VK_ABI_COMPATIBLE with vk::ColorComponentFlags
	return std::bit_cast<vk::ColorComponentFlags>(flags);
}

static vk::PipelineStageFlags to_vk_pipeline_stage_flags(PipelineStageFlags flags) VERA_NOEXCEPT
{
	// vr::PipelineStageFlags is VERA_VK_ABI_COMPATIBLE with vk::PipelineStageFlags
	return std::bit_cast<vk::PipelineStageFlags>(flags);
}

static vk::DynamicState to_vk_dynamic_state(DynamicState state) VERA_NOEXCEPT
{
	// vr::DynamicState is VERA_VK_ABI_COMPATIBLE with vk::DynamicState
	return std::bit_cast<vk::DynamicState>(state);
}

static vk::ShaderStageFlagBits to_vk_shader_stage_flag_bits(ShaderStageFlagBits stage) VERA_NOEXCEPT
{
	// vr::ShaderStageFlagBits is VERA_VK_ABI_COMPATIBLE with vk::ShaderStageFlagBits
	return std::bit_cast<vk::ShaderStageFlagBits>(stage);
}

static vk::ShaderStageFlags to_vk_shader_stage_flags(ShaderStageFlags flags) VERA_NOEXCEPT
{
	// vr::ShaderStageFlags is VERA_VK_ABI_COMPATIBLE with vk::ShaderStageFlags
	return std::bit_cast<vk::ShaderStageFlags>(flags);
}

static vk::DescriptorSetLayoutCreateFlags to_vk_descriptor_set_layout_create_flags(DescriptorSetLayoutCreateFlags flags)
{
	// vr::DescriptorSetLayoutCreateFlags is VERA_VK_ABI_COMPATIBLE with vk::DescriptorSetLayoutCreateFlags
	return std::bit_cast<vk::DescriptorSetLayoutCreateFlags>(flags);
}

static vk::DescriptorBindingFlags to_vk_descriptor_binding_flags(DescriptorSetLayoutBindingFlags flags)
{
	// vr::DescriptorSetLayoutBindingFlags is VERA_VK_ABI_COMPATIBLE with vk::DescriptorBindingFlags
	return std::bit_cast<vk::DescriptorBindingFlags>(flags);
}

static vk::DescriptorType to_vk_descriptor_type(DescriptorType type)
{
	VERA_ASSERT(type != DescriptorType::Unknown);
	return static_cast<vk::DescriptorType>(static_cast<uint32_t>(type) - 1);
}

static vk::DescriptorPoolCreateFlags to_vk_descriptor_pool_create_flags(DescriptorPoolCreateFlags flags)
{
	// vr::DescriptorPoolCreateFlags is VERA_VK_ABI_COMPATIBLE with vk::DescriptorPoolCreateFlags
	return std::bit_cast<vk::DescriptorPoolCreateFlags>(flags);
}

static vk::PresentModeKHR to_vk_present_mode(PresentMode mode)
{
	// vr::PresentMode is VERA_VK_ABI_COMPATIBLE with vk::PresentModeKHR
	return std::bit_cast<vk::PresentModeKHR>(mode);
}

static vk::Filter to_vk_filter(SamplerFilter filter)
{
	// vr::SamplerFilter is VERA_VK_ABI_COMPATIBLE with vk::Filter
	return std::bit_cast<vk::Filter>(filter);
}

static vk::SamplerMipmapMode to_vk_sampler_mipmap_mode(SamplerMipmapMode filter)
{
	// vr::SamplerMipmapMode is VERA_VK_ABI_COMPATIBLE with vk::SamplerMipmapMode
	return std::bit_cast<vk::SamplerMipmapMode>(filter);
}

static vk::SamplerAddressMode to_vk_sampler_address_mode(SamplerAddressMode mode)
{
	// vr::SamplerAddressMode is VERA_VK_ABI_COMPATIBLE with vk::SamplerAddressMode
	return std::bit_cast<vk::SamplerAddressMode>(mode);
}

static vk::CompareOp to_vk_compare_op(SamplerCompareOp op)
{
	// vr::SamplerCompareOp is VERA_VK_ABI_COMPATIBLE with vk::CompareOp
	return std::bit_cast<vk::CompareOp>(op);
}

static vk::ResolveModeFlagBits to_vk_resolve_mode_flag_bits(ResolveModeFlagBits flag) VERA_NOEXCEPT
{
	// vr::ResolveModeFlagBits is VERA_VK_ABI_COMPATIBLE with vk::ResolveModeFlagBits
	return std::bit_cast<vk::ResolveModeFlagBits>(flag);
}

static vk::ResolveModeFlags to_vk_resolve_mode_flags(ResolveModeFlags flags) VERA_NOEXCEPT
{
	// vr::ResolveModeFlags is VERA_VK_ABI_COMPATIBLE with vk::ResolveModeFlags
	return std::bit_cast<vk::ResolveModeFlagBits>(flags);
}

static vk::AttachmentLoadOp to_vk_attachment_load_op(LoadOp op) VERA_NOEXCEPT
{
	// vr::LoadOp is VERA_VK_ABI_COMPATIBLE with vk::AttachmentLoadOp
	return std::bit_cast<vk::AttachmentLoadOp>(op);
}

static vk::AttachmentStoreOp to_vk_attachment_store_op(StoreOp op) VERA_NOEXCEPT
{
	// vr::StoreOp is VERA_VK_ABI_COMPATIBLE with vk::AttachmentStoreOp
	return std::bit_cast<vk::AttachmentStoreOp>(op);
}

static vk::AccessFlags to_vk_access_flags(AccessFlags flags)
{
	// vr::AccessFlags is VERA_VK_ABI_COMPATIBLE with vk::AccessFlags
	return std::bit_cast<vk::AccessFlags>(flags);
}

static vk::BufferUsageFlags to_vk_buffer_usage_flags(BufferUsageFlags flags) VERA_NOEXCEPT
{
	// vr::BufferUsageFlags is VERA_VK_ABI_COMPATIBLE with vk::BufferUsageFlags
	return std::bit_cast<vk::BufferUsageFlags>(flags);
}

static vk::ImageLayout to_vk_image_layout(TextureLayout layout)
{
	// vr::TextureLayout is VERA_VK_ABI_COMPATIBLE with vk::ImageLayout
	return std::bit_cast<vk::ImageLayout>(layout);
}

static vk::ImageAspectFlags to_vk_image_aspect_flags(TextureAspectFlags flags)
{
	// vr::TextureAspectFlags is VERA_VK_ABI_COMPATIBLE with vk::ImageAspectFlags
	return std::bit_cast<vk::ImageAspectFlags>(flags);
}

static vk::ImageUsageFlags to_vk_image_usage_flags(TextureUsageFlags flags)
{
	vk::ImageUsageFlags result;

	if (flags.has(TextureUsageFlagBits::TransferSrc))
		result |= vk::ImageUsageFlagBits::eTransferSrc;
	if (flags.has(TextureUsageFlagBits::TransferDst))
		result |= vk::ImageUsageFlagBits::eTransferDst;
	if (flags.has(TextureUsageFlagBits::Sampled))
		result |= vk::ImageUsageFlagBits::eSampled;
	if (flags.has(TextureUsageFlagBits::Storage))
		result |= vk::ImageUsageFlagBits::eStorage;
	if (flags.has(TextureUsageFlagBits::ColorAttachment))
		result |= vk::ImageUsageFlagBits::eColorAttachment;
	if (flags.has(TextureUsageFlagBits::DepthStencilAttachment))
		result |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
	if (flags.has(TextureUsageFlagBits::TransientAttachment))
		result |= vk::ImageUsageFlagBits::eTransientAttachment;
	if (flags.has(TextureUsageFlagBits::InputAttachment))
		result |= vk::ImageUsageFlagBits::eInputAttachment;
	if (flags.has(TextureUsageFlagBits::HostTransfer))
		result |= vk::ImageUsageFlagBits::eHostTransfer;
	VERA_ASSERT_MSG(!flags.has(TextureUsageFlagBits::FrameBuffer), "invalid image usage flag bit");

	return result;
}

static vk::ImageViewType to_vk_image_view_type(TextureViewType type)
{
	// vr::TextureViewType is VERA_VK_ABI_COMPATIBLE with vk::ImageViewType
	return std::bit_cast<vk::ImageViewType>(type);
}

static vk::ComponentSwizzle to_vk_component_swizzle(ComponentSwizzle swizzle)
{
	// vr::ComponentSwizzle is VERA_VK_ABI_COMPATIBLE with vk::ComponentSwizzle
	return std::bit_cast<vk::ComponentSwizzle>(swizzle);
}

static vk::Format to_vk_format(Format format) VERA_NOEXCEPT
{
	switch (format) {
	case Format::Unknown:              return vk::Format::eUndefined;
	
	// 1 channel
	case Format::R8Unorm:              return vk::Format::eR8Unorm;
	case Format::R8Snorm:              return vk::Format::eR8Snorm;
	case Format::R8Uscaled:            return vk::Format::eR8Uscaled;
	case Format::R8Sscaled:            return vk::Format::eR8Sscaled;
	case Format::R8Uint:               return vk::Format::eR8Uint;
	case Format::R8Sint:               return vk::Format::eR8Sint;
	case Format::R8Srgb:               return vk::Format::eR8Srgb;
	case Format::R16Unorm:             return vk::Format::eR16Unorm;
	case Format::R16Snorm:             return vk::Format::eR16Snorm;
	case Format::R16Uscaled:           return vk::Format::eR16Uscaled;
	case Format::R16Sscaled:           return vk::Format::eR16Sscaled;
	case Format::R16Uint:              return vk::Format::eR16Uint;
	case Format::R16Sint:              return vk::Format::eR16Sint;
	case Format::R16Float:             return vk::Format::eR16Sfloat;
	case Format::R32Uint:              return vk::Format::eR32Uint;
	case Format::R32Sint:              return vk::Format::eR32Sint;
	case Format::R32Float:             return vk::Format::eR32Sfloat;
	case Format::R64Uint:              return vk::Format::eR64Uint;
	case Format::R64Sint:              return vk::Format::eR64Sint;
	case Format::R64Float:             return vk::Format::eR64Sfloat;

	// 2 channel
	case Format::RG8Unorm:             return vk::Format::eR8G8Unorm;
	case Format::RG8Snorm:             return vk::Format::eR8G8Snorm;
	case Format::RG8Uscaled:           return vk::Format::eR8G8Uscaled;
	case Format::RG8Sscaled:           return vk::Format::eR8G8Sscaled;
	case Format::RG8Uint:              return vk::Format::eR8G8Uint;
	case Format::RG8Sint:              return vk::Format::eR8G8Sint;
	case Format::RG8Srgb:              return vk::Format::eR8G8Srgb;
	case Format::RG16Unorm:            return vk::Format::eR16G16Unorm;
	case Format::RG16Snorm:            return vk::Format::eR16G16Snorm;
	case Format::RG16Uscaled:          return vk::Format::eR16G16Uscaled;
	case Format::RG16Sscaled:          return vk::Format::eR16G16Sscaled;
	case Format::RG16Uint:             return vk::Format::eR16G16Uint;
	case Format::RG16Sint:             return vk::Format::eR16G16Sint;
	case Format::RG16Float:            return vk::Format::eR16G16Sfloat;
	case Format::RG32Uint:             return vk::Format::eR32G32Uint;
	case Format::RG32Sint:             return vk::Format::eR32G32Sint;
	case Format::RG32Float:            return vk::Format::eR32G32Sfloat;
	case Format::RG64Uint:             return vk::Format::eR64G64Uint;
	case Format::RG64Sint:             return vk::Format::eR64G64Sint;
	case Format::RG64Float:            return vk::Format::eR64G64Sfloat;

	// 3 channel
	case Format::RGB8Unorm:            return vk::Format::eR8G8B8Unorm;
	case Format::RGB8Snorm:            return vk::Format::eR8G8B8Snorm;
	case Format::RGB8Uscaled:          return vk::Format::eR8G8B8Uscaled;
	case Format::RGB8Sscaled:          return vk::Format::eR8G8B8Sscaled;
	case Format::RGB8Uint:             return vk::Format::eR8G8B8Uint;
	case Format::RGB8Sint:             return vk::Format::eR8G8B8Sint;
	case Format::RGB8Srgb:             return vk::Format::eR8G8B8Srgb;
	case Format::BGR8Unorm:            return vk::Format::eB8G8R8Unorm;
	case Format::BGR8Snorm:            return vk::Format::eB8G8R8Snorm;
	case Format::BGR8Uscaled:          return vk::Format::eB8G8R8Uscaled;
	case Format::BGR8Sscaled:          return vk::Format::eB8G8R8Sscaled;
	case Format::BGR8Uint:             return vk::Format::eB8G8R8Uint;
	case Format::BGR8Sint:             return vk::Format::eB8G8R8Sint;
	case Format::BGR8Srgb:             return vk::Format::eB8G8R8Srgb;
	case Format::RGB16Unorm:           return vk::Format::eR16G16B16Unorm;
	case Format::RGB16Snorm:           return vk::Format::eR16G16B16Snorm;
	case Format::RGB16Uscaled:         return vk::Format::eR16G16B16Uscaled;
	case Format::RGB16Sscaled:         return vk::Format::eR16G16B16Sscaled;
	case Format::RGB16Uint:            return vk::Format::eR16G16B16Uint;
	case Format::RGB16Sint:            return vk::Format::eR16G16B16Sint;
	case Format::RGB16Float:           return vk::Format::eR16G16B16Sfloat;
	case Format::RGB32Uint:            return vk::Format::eR32G32B32Uint;
	case Format::RGB32Sint:            return vk::Format::eR32G32B32Sint;
	case Format::RGB32Float:           return vk::Format::eR32G32B32Sfloat;
	case Format::RGB64Uint:            return vk::Format::eR64G64B64Uint;
	case Format::RGB64Sint:            return vk::Format::eR64G64B64Sint;
	case Format::RGB64Float:           return vk::Format::eR64G64B64Sfloat;

	// 4 channel
	case Format::RGBA8Unorm:           return vk::Format::eR8G8B8A8Unorm;
	case Format::RGBA8Snorm:           return vk::Format::eR8G8B8A8Snorm;
	case Format::RGBA8Uscaled:         return vk::Format::eR8G8B8A8Uscaled;
	case Format::RGBA8Sscaled:         return vk::Format::eR8G8B8A8Sscaled;
	case Format::RGBA8Uint:            return vk::Format::eR8G8B8A8Uint;
	case Format::RGBA8Sint:            return vk::Format::eR8G8B8A8Sint;
	case Format::RGBA8Srgb:            return vk::Format::eR8G8B8A8Srgb;
	case Format::BGRA8Unorm:           return vk::Format::eB8G8R8A8Unorm;
	case Format::BGRA8Snorm:           return vk::Format::eB8G8R8A8Snorm;
	case Format::BGRA8Uscaled:         return vk::Format::eB8G8R8A8Uscaled;
	case Format::BGRA8Sscaled:         return vk::Format::eB8G8R8A8Sscaled;
	case Format::BGRA8Uint:            return vk::Format::eB8G8R8A8Uint;
	case Format::BGRA8Sint:            return vk::Format::eB8G8R8A8Sint;
	case Format::BGRA8Srgb:            return vk::Format::eB8G8R8A8Srgb;
	case Format::RGBA16Unorm:          return vk::Format::eR16G16B16A16Unorm;
	case Format::RGBA16Snorm:          return vk::Format::eR16G16B16A16Snorm;
	case Format::RGBA16Uscaled:        return vk::Format::eR16G16B16A16Uscaled;
	case Format::RGBA16Sscaled:        return vk::Format::eR16G16B16A16Sscaled;
	case Format::RGBA16Uint:           return vk::Format::eR16G16B16A16Uint;
	case Format::RGBA16Sint:           return vk::Format::eR16G16B16A16Sint;
	case Format::RGBA16Float:          return vk::Format::eR16G16B16A16Sfloat;
	case Format::RGBA32Uint:           return vk::Format::eR32G32B32A32Uint;
	case Format::RGBA32Sint:           return vk::Format::eR32G32B32A32Sint;
	case Format::RGBA32Float:          return vk::Format::eR32G32B32A32Sfloat;
	case Format::RGBA64Uint:           return vk::Format::eR64G64B64A64Uint;
	case Format::RGBA64Sint:           return vk::Format::eR64G64B64A64Sint;
	case Format::RGBA64Float:          return vk::Format::eR64G64B64A64Sfloat;

	// Pack 8bit
	case Format::RG4UnormPack8:        return vk::Format::eR4G4UnormPack8;

	// Pack 16bit
	case Format::RGBA4UnormPack16:     return vk::Format::eR4G4B4A4UnormPack16;
	case Format::BGRA4UnormPack16:     return vk::Format::eB4G4R4A4UnormPack16;
	case Format::R5G6B5UnormPack16:    return vk::Format::eR5G6B5UnormPack16;
	case Format::B5G6R5UnormPack16:    return vk::Format::eB5G6R5UnormPack16;
	case Format::R5G5B5A1UnormPack16:  return vk::Format::eR5G5B5A1UnormPack16;
	case Format::B5G5R5A1UnormPack16:  return vk::Format::eB5G5R5A1UnormPack16;
	case Format::A1R5G5B5UnormPack16:  return vk::Format::eA1R5G5B5UnormPack16;
	case Format::A4R4G4B4UnormPack16:  return vk::Format::eA4R4G4B4UnormPack16;
	case Format::A4B4G4R4UnormPack16:  return vk::Format::eA4B4G4R4UnormPack16;
	case Format::A1B5G5R5UnormPack16:  return vk::Format::eA1B5G5R5UnormPack16;
	
	// Pack 32bit
	case Format::ABGR8UnormPack32:     return vk::Format::eA8B8G8R8UnormPack32;
	case Format::ABGR8SnormPack32:     return vk::Format::eA8B8G8R8SnormPack32;
	case Format::ABGR8UscaledPack32:   return vk::Format::eA8B8G8R8UscaledPack32;
	case Format::ABGR8SscaledPack32:   return vk::Format::eA8B8G8R8SscaledPack32;
	case Format::ABGR8UintPack32:      return vk::Format::eA8B8G8R8UintPack32;
	case Format::ABGR8SintPack32:      return vk::Format::eA8B8G8R8SintPack32;
	case Format::ABGR8SrgbPack32:      return vk::Format::eA8B8G8R8SrgbPack32;
	case Format::A2RGB10UnormPack32:   return vk::Format::eA2R10G10B10UnormPack32;
	case Format::A2RGB10SnormPack32:   return vk::Format::eA2R10G10B10SnormPack32;
	case Format::A2RGB10UscaledPack32: return vk::Format::eA2R10G10B10UscaledPack32;
	case Format::A2RGB10SscaledPack32: return vk::Format::eA2R10G10B10SscaledPack32;
	case Format::A2RGB10UintPack32:    return vk::Format::eA2R10G10B10UintPack32;
	case Format::A2RGB10SintPack32:    return vk::Format::eA2R10G10B10SintPack32;
	case Format::A2BGR10UnormPack32:   return vk::Format::eA2B10G10R10UnormPack32;
	case Format::A2BGR10SnormPack32:   return vk::Format::eA2B10G10R10SnormPack32;
	case Format::A2BGR10UscaledPack32: return vk::Format::eA2B10G10R10UscaledPack32;
	case Format::A2BGR10SscaledPack32: return vk::Format::eA2B10G10R10SscaledPack32;
	case Format::A2BGR10UintPack32:    return vk::Format::eA2B10G10R10UintPack32;
	case Format::A2BGR10SintPack32:    return vk::Format::eA2B10G10R10SintPack32;

	// Depth only
	case Format::D16Unorm:             return vk::Format::eD16Unorm;
	case Format::X8D24Unorm:           return vk::Format::eX8D24UnormPack32;
	case Format::D32Float:             return vk::Format::eD32Sfloat;
	
	// Stencil only
	case Format::S8Uint:               return vk::Format::eS8Uint;
	
	// Depth and Stencil
	case Format::D16UnormS8Uint:       return vk::Format::eD16UnormS8Uint;
	case Format::D24UnormS8Uint:       return vk::Format::eD24UnormS8Uint;
	case Format::D32FloatS8Uint:       return vk::Format::eD32SfloatS8Uint;

	// Alpha
	case Format::A8Unorm:              return vk::Format::eA8Unorm;
	}

	return vk::Format::eUndefined;
}

static vk::IndexType to_vk_index_type(IndexType type) VERA_NOEXCEPT
{
	switch (type) {
	case IndexType::UInt8:	return vk::IndexType::eUint8;
	case IndexType::UInt16:	return vk::IndexType::eUint16;
	case IndexType::UInt32: return vk::IndexType::eUint32;
	}

	VERA_ASSERT_MSG(false, "invalid index type");
	return {};
}

static vk::Rect2D to_vk_rect2d(const irect2d& rect) VERA_NOEXCEPT
{
	return {
		vk::Offset2D{ rect.x, rect.y },
		vk::Extent2D{ rect.width, rect.height }
	};
}

VERA_NAMESPACE_END