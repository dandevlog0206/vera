#pragma once

#include "core_object.h"
#include "../util/array_view.h"

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class TextureLayout VERA_ENUM;

class DescriptorPool;
class DescriptorSetLayout;
class Sampler;
class TextureView;
class Buffer;
class BufferView;

class DescriptorBindingInfo
{
public:
	DescriptorBindingInfo() VERA_NOEXCEPT;
	DescriptorBindingInfo(const DescriptorBindingInfo& rhs) VERA_NOEXCEPT;
	~DescriptorBindingInfo();

	DescriptorBindingInfo& operator=(const DescriptorBindingInfo& rhs) VERA_NOEXCEPT;

	VERA_NODISCARD bool operator<(const DescriptorBindingInfo& rhs) const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

	VERA_NODISCARD hash_t hash() const VERA_NOEXCEPT;

	struct SamplerBindingInfo
	{
		ref<Sampler> sampler;
	};

	struct CombinedImageSamplerBindingInfo
	{
		ref<Sampler>     sampler;
		ref<TextureView> textureView;
		TextureLayout    textureLayout;
	};

	struct TextureBindingInfo
	{
		ref<TextureView> textureView;
		TextureLayout    textureLayout;
	};

	struct TexelBufferBindingInfo
	{
		ref<BufferView> bufferView;
	};

	struct BufferBindingInfo
	{
		ref<Buffer> buffer;
		size_t      offset;
		size_t      range;
	};

	DescriptorType descriptorType;
	uint32_t       dstBinding;
	uint32_t       dstArrayElement;

	union {
		SamplerBindingInfo              sampler;
		CombinedImageSamplerBindingInfo combinedImageSampler;
		TextureBindingInfo              sampledImage;
		TextureBindingInfo              storageImage;
		TexelBufferBindingInfo          uniformTexelBuffer;
		TexelBufferBindingInfo          storageTexelBuffer;
		BufferBindingInfo               uniformBuffer;
		BufferBindingInfo               storageBuffer;
		BufferBindingInfo               uniformBufferDynamic;
		BufferBindingInfo               storageBufferDynamic;
		TextureBindingInfo              inputAttachment;
	};
};

class DescriptorSet : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(DescriptorSet)
	friend class DescriptorPool;
public:
	~DescriptorSet();

	VERA_NODISCARD ref<DescriptorPool> getDescriptorPool() VERA_NOEXCEPT;
	VERA_NODISCARD const_ref<DescriptorSetLayout> getDescriptorSetLayout() VERA_NOEXCEPT;

	const DescriptorBindingInfo& getDescriptorBindingInfo(uint32_t binding, uint32_t array_element) VERA_NOEXCEPT;
	void setDescriptorBindingInfo(const DescriptorBindingInfo& info);
	void setDescriptorBindingInfo(array_view<DescriptorBindingInfo> infos);

	// update using previously set infos, if not allocated reallocate one
	void update();

	void makeCached();

	VERA_NODISCARD bool isCached() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isValid() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isDestroyed() const VERA_NOEXCEPT;

	VERA_NODISCARD hash_t hashValue() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END