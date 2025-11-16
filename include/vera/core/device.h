#pragma once

#include "core_object.h"
#include "enum_types.h"
#include "../util/array_view.h"
#include "../util/flag.h"
#include <string_view>
#include <string>
#include <vector>

VERA_NAMESPACE_BEGIN

class Context;
class Sampler;
class Texture;
class TextureView;
class Buffer;
class BufferView;

struct DeviceFaultAddressInfo
{
	DeviceFaultAddressType type;
	uint64_t               address;
	uint64_t               addressPrecision;
};

struct DeviceFaultVenderInfo
{
	char     description[256];
	uint64_t vendorID;
	uint64_t deviceID;
};

class DeviceFaultInfo
{
public:
	std::string                         description;
	std::vector<DeviceFaultAddressInfo> addressInfos;
	std::vector<DeviceFaultVenderInfo>  vendorInfos;
	std::vector<std::byte>              vendorBinaryData;

	void saveVendorBinaryToFile(std::string_view path) const;
};

struct DeviceMemoryType
{
	uint32_t            heapID;
	size_t              size;
	MemoryHeapFlags     heapFlags;
	MemoryPropertyFlags propertyFlags;
};

struct DeviceCreateInfo
{
	std::vector<std::string_view> deviceLayers          = {};
	std::vector<std::string_view> deviceExtensions      = {};

	uint32_t                      deviceID              = 0;

	Format                        colorFormat           = Format::RGBA8Unorm;
	Format                        depthFormat           = Format::D32Float;

	bool                          enablePipelineCache   = true;
	std::string_view              pipelineCacheFilePath = {};
};

class Device : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Device)
public:
	static obj<Device> create(obj<Context> context, const DeviceCreateInfo& info = {});
	~Device();

	VERA_NODISCARD obj<Context> getContext() VERA_NOEXCEPT;

	VERA_NODISCARD obj<Sampler> getDefaultSampler() VERA_NOEXCEPT;
	VERA_NODISCARD obj<Texture> getDefaultTexture() VERA_NOEXCEPT;
	VERA_NODISCARD obj<TextureView> getDefaultTextureView() VERA_NOEXCEPT;
	VERA_NODISCARD obj<Buffer> getDefaultBuffer() VERA_NOEXCEPT;
	VERA_NODISCARD obj<BufferView> getDefaultBufferView() VERA_NOEXCEPT;

	VERA_NODISCARD bool isFeatureEnabled(DeviceFeatureType feature) const VERA_NOEXCEPT;

	array_view<DeviceMemoryType> enumerateMemoryTypes() const;
	
	DeviceFaultInfo getDeviceFaultInfo() const;

	void waitIdle() const;
};

VERA_NAMESPACE_END