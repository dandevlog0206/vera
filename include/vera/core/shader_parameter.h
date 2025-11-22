#pragma once

#include "shader.h"
#include "shader_variable.h"

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class TextureLayout VERA_ENUM;

class Device;
class PipelineLayout;
class DescriptorPool;

struct DescriptorIndex
{
	uint32_t set        = 0;
	uint32_t binding    = 0;
	uint32_t arrayIndex = 0;
};

class ShaderParameter : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderParameter)
public:
	static obj<ShaderParameter> create(obj<PipelineLayout> pipeline_layout, obj<DescriptorPool> descriptor_pool = {});
	~ShaderParameter() VERA_NOEXCEPT override;

	obj<Device> getDevice() VERA_NOEXCEPT;
	obj<PipelineLayout> getPipelineLayout() VERA_NOEXCEPT;
	obj<DescriptorPool> getDescriptorPool() VERA_NOEXCEPT;

	VERA_NODISCARD ShaderVariable getRootVariable() VERA_NOEXCEPT;

	void reset();

	void setSampler(
		const DescriptorIndex& idx,
		obj<Sampler>           sampler);

	void setSampler(
		uint32_t     set,
		uint32_t     binding,
		uint32_t     array_idx,
		obj<Sampler> sampler);

	void setTextureView(
		uint32_t         set,
		uint32_t         binding,
		uint32_t         array_idx,
		obj<TextureView> texture_view);

	void setTextureView(
		uint32_t         set,
		uint32_t         binding,
		uint32_t         array_idx,
		obj<TextureView> texture_view,
		TextureLayout    texture_layout);

	void setBufferView(
		uint32_t        set,
		uint32_t        binding,
		uint32_t        array_idx,
		obj<BufferView> buffer_view);

	void setBuffer(
		uint32_t    set,
		uint32_t    binding,
		uint32_t    array_idx,
		obj<Buffer> buffer,
		size_t      offset = 0,
		size_t      range  = 0);

	void pushConstants(
		ShaderStageFlags stage_flags,
		const void*      data,
		size_t           size,
		size_t           offset = 0);
};

VERA_NAMESPACE_END