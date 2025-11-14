#pragma once

#include "descriptor_set_layout.h"
#include "../util/array_view.h"
#include <variant>

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class TextureLayout VERA_ENUM;

class Device;
class DescriptorPool;
class DescriptorSetLayout;
class Sampler;
class TextureView;
class Buffer;
class BufferView;

struct DescriptorSamplerInfo
{
	obj<Sampler> sampler;
};

struct DescriptorCombinedTextureSamplerInfo
{
	obj<Sampler>     sampler;
	obj<TextureView> textureView;
	TextureLayout    layout = TextureLayout::Undefined;
};

struct DescriptorTextureInfo
{
	obj<TextureView> textureView;
	TextureLayout    layout = TextureLayout::Undefined;
};

struct DescriptorBufferViewInfo
{
	obj<BufferView> bufferView;
};

struct DescriptorBufferInfo
{
	obj<Buffer> buffer;
	size_t      offset;
	size_t      range;
};

struct DescriptorWrite
{
	using DescriptorWriteInfo = std::variant<
		DescriptorSamplerInfo,
		DescriptorCombinedTextureSamplerInfo,
		DescriptorTextureInfo,
		DescriptorBufferViewInfo,
		DescriptorBufferInfo>;

	uint32_t            binding;
	uint32_t            arrayElement = 0;
	DescriptorWriteInfo info;
};

class DescriptorSet : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(DescriptorSet)
	friend class DescriptorPool;
public:
	~DescriptorSet();

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;
	VERA_NODISCARD obj<DescriptorPool> getDescriptorPool() VERA_NOEXCEPT;
	VERA_NODISCARD obj<DescriptorSetLayout> getDescriptorSetLayout() VERA_NOEXCEPT;

	void write(uint32_t binding, const DescriptorSamplerInfo& info, uint32_t array_element = 0);
	void write(uint32_t binding, array_view<DescriptorSamplerInfo> infos, uint32_t array_element = 0);
	void write(uint32_t binding, const DescriptorCombinedTextureSamplerInfo& info, uint32_t array_element = 0);
	void write(uint32_t binding, array_view<DescriptorCombinedTextureSamplerInfo> infos, uint32_t array_element = 0);
	void write(uint32_t binding, const DescriptorTextureInfo& info, uint32_t array_element = 0);
	void write(uint32_t binding, array_view<DescriptorTextureInfo> infos, uint32_t array_element = 0);
	void write(uint32_t binding, const DescriptorBufferViewInfo& info, uint32_t array_element = 0);
	void write(uint32_t binding, array_view<DescriptorBufferViewInfo> infos, uint32_t array_element = 0);
	void write(uint32_t binding, const DescriptorBufferInfo& info, uint32_t array_element = 0);
	void write(uint32_t binding, array_view<DescriptorBufferInfo> infos, uint32_t array_element = 0);
	void write(array_view<DescriptorWrite> writes);

	VERA_NODISCARD bool isValid() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END