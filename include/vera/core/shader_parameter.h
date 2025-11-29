#pragma once

#include "shader.h"
#include "shader_variable.h"

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;
enum class TextureLayout VERA_ENUM;

class Device;
class ProgramReflection;
class PipelineLayout;
class DescriptorPool;

struct DescriptorIndex
{
	uint32_t set        = 0;
	uint32_t binding    = 0;
	uint32_t arrayIndex = 0;
};

class ShaderParameter : public CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderParameter)
public:
	static obj<ShaderParameter> create(
		obj<Device>            device,
		obj<ProgramReflection> program_reflection,
		obj<DescriptorPool>    descriptor_pool = {});
	~ShaderParameter() VERA_NOEXCEPT override;

	obj<Device> getDevice() VERA_NOEXCEPT;
	obj<PipelineLayout> getPipelineLayout() VERA_NOEXCEPT;
	obj<DescriptorPool> getDescriptorPool() VERA_NOEXCEPT;

	VERA_NODISCARD ShaderVariable getRootVariable() VERA_NOEXCEPT;

	void setBindless(uint32_t set, uint32_t binding, bool enable = true);
	VERA_NODISCARD bool isBindless(uint32_t set, uint32_t binding) const VERA_NOEXCEPT;

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

	void pushConstant(
		const void* data,
		size_t      size,
		size_t      offset = 0);

	void nextFrame();

	void reset();
};

VERA_NAMESPACE_END