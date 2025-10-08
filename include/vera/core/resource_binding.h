#pragma once

#include "core_object.h"
#include "../util/array_view.h"
#include "../util/hash.h"

VERA_NAMESPACE_BEGIN

enum class ResourceType VERA_ENUM;
enum class TextureLayout VERA_ENUM;

class ResourceBindingPool;
class ResourceLayout;
class Sampler;
class TextureView;
class Buffer;
class BufferView;

class ResourceBindingInfo
{
public:
	ResourceBindingInfo() VERA_NOEXCEPT;
	ResourceBindingInfo(const ResourceBindingInfo& rhs) VERA_NOEXCEPT;
	~ResourceBindingInfo();

	ResourceBindingInfo& operator=(const ResourceBindingInfo& rhs) VERA_NOEXCEPT;

	VERA_NODISCARD bool operator<(const ResourceBindingInfo& rhs) const VERA_NOEXCEPT;

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
		uint32_t    offset;
		uint32_t    range;
	};

	ResourceType resourceType;
	uint32_t     dstBinding;
	uint32_t     dstArrayElement;

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

class ResourceBinding : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ResourceBinding)
	friend class BindingPool;
public:
	~ResourceBinding();

	VERA_NODISCARD ref<ResourceBindingPool> getResourceBindingPool() VERA_NOEXCEPT;
	VERA_NODISCARD const_ref<ResourceLayout> getResourceLayout() VERA_NOEXCEPT;

	const ResourceBindingInfo& getBindingInfo(uint32_t binding, uint32_t array_element) VERA_NOEXCEPT;
	void setBindingInfo(const ResourceBindingInfo& info);
	void setBindingInfo(array_view<ResourceBindingInfo> infos);

	// update using previously set infos, if not allocated reallocate one
	void update();

	void makeCached();

	VERA_NODISCARD bool isCached() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isValid() const VERA_NOEXCEPT;
	VERA_NODISCARD bool isDestroyed() const VERA_NOEXCEPT;

	VERA_NODISCARD hash_t hashValue() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END