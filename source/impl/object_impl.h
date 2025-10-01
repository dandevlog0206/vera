#pragma once

#include "../internal/include_vulkan.h"
#include "../../include/vera/core/core_object.h"
#include "../../include/vera/core/exception.h"
#include "../../include/vera/graphics/format.h"
#include "../../include/vera/util/rect.h"

VERA_NAMESPACE_BEGIN

// list of core object
class Buffer;
class BufferView;
class Context;
class Device;
class DeviceMemory;
class Fence;
class FrameBuffer;
class Pipeline;
class PipelineLayout;
class CommandBuffer;
class RenderContext;
class ResourceLayout;
class Sampler;
class Semaphore;
class Shader;
class ShaderReflection;
class Swapchain;
class Texture;
class TextureView;

vk::Instance& get_vk_instance(ref<Context> context);
const vk::Device& get_vk_device(const_ref<Device> device);
vk::Device& get_vk_device(ref<Device> device);
vk::Fence& get_vk_fence(ref<Fence> fence);
vk::PipelineLayout& get_vk_pipeline_layout(ref<PipelineLayout> pipeline_layout);
vk::CommandBuffer& get_vk_command_buffer(ref<CommandBuffer> render_command);
vk::DescriptorSetLayout& get_descriptor_set_layout(ref<ResourceLayout> resource_layout);
vk::Sampler& get_vk_sampler(ref<Sampler> sampler);
vk::Semaphore& get_vk_semaphore(ref<Semaphore> semaphore);
vk::Image& get_vk_image(ref<Texture> texture);
vk::ImageView& get_vk_image_view(ref<TextureView> texture_view);

static vk::Format to_vk_format(Format format)
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

	// Depth
	case Format::D16Unorm:             return vk::Format::eD16Unorm;
	case Format::X8D24Unorm:           return vk::Format::eX8D24UnormPack32;
	case Format::D32Float:             return vk::Format::eD32Sfloat;
	
	// Stencil 
	case Format::S8Uint:               return vk::Format::eS8Uint;
	
	// Depth Stencil
	case Format::D16UnormS8Uint:       return vk::Format::eD16UnormS8Uint;
	case Format::D24UnormS8Uint:       return vk::Format::eD24UnormS8Uint;
	case Format::D32FloatS8Uint:       return vk::Format::eD32SfloatS8Uint;

	// Alpha
	case Format::A8Unorm:              return vk::Format::eA8Unorm;
	}

	return vk::Format::eUndefined;
}

static vk::Rect2D to_vk_rect2d(const irect2d& rect)
{
	return {
		vk::Offset2D{ rect.x, rect.y },
		vk::Extent2D{ rect.width, rect.height }
	};
}

VERA_NAMESPACE_END