#pragma once

#include "core_object.h"
#include "format.h"
#include "../util/extent.h"

VERA_NAMESPACE_BEGIN

class Device;
class DeviceMemory;
class TextureView;

enum class TextureType VERA_ENUM
{
	Texture1D,
	Texture2D,
	Texture3D,
	TextureCube,
	Texture2DMultisample,
};

enum class ImageLayout VERA_ENUM
{
	Undefined,
	General,
	ColorAttachmentOptimal,
	DepthStencilAttachmentOptimal,
	DepthStencilReadOnlyOptimal,
	ShaderReadOnlyOptimal,
	TransferSrcOptimal,
	TransferDstOptimal,
	Preinitialized,
	DepthReadOnlyStencilAttachmentOptimal,
	DepthAttachmentStencilReadOnlyOptimal,
	DepthAttachmentOptimal,
	DepthReadOnlyOptimal,
	StencilAttachmentOptimal,
	StencilReadOnlyOptimal,
	ReadOnlyOptimal,
	AttachmentOptimal,
	RenderingLocalRead,
	PresentSrc
};

struct TextureCreateInfo
{
	TextureType       type;
	Format            format;
	uint32_t          width;
	uint32_t          height;
	uint32_t          depth;
	uint32_t          arraySize;
	uint32_t          mipLevels;
	uint32_t          sampleCount;
	// ResourceBindFlags bindFlags;
};

class Texture : protected CoreObject
{
	VERA_CORE_OBJECT(Texture)
public:
	static ref<Texture> create(ref<Device> device, const TextureCreateInfo& info);
	~Texture();

	ref<DeviceMemory> getDeviceMemory();

	ref<TextureView> getTextureView();

	uint32_t width() const;
	uint32_t height() const;
	uint32_t depth() const;
	extent3d extent() const;
};

VERA_NAMESPACE_END