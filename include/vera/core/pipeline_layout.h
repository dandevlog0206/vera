#pragma once

#include "device.h"
#include "descriptor_set_layout.h"
#include "shader_reflection.h"
#include "../util/array_view.h"
#include "../util/small_vector.h"

VERA_NAMESPACE_BEGIN

class ShaderReflection;
class ProgramReflection;

struct PushConstantRange
{
	ShaderStageFlags stageFlags = {};
	uint32_t         offset     = 0;
	uint32_t         size       = 0;
};

struct PipelineLayoutCreateInfo
{
	small_vector<obj<DescriptorSetLayout>> descriptorSetLayouts = {};
	small_vector<PushConstantRange>        pushConstantRanges   = {};
};

class PipelineLayout : public CoreObject
{
	VERA_CORE_OBJECT_INIT(PipelineLayout)
public:
	static obj<PipelineLayout> create(obj<Device> device, array_view<cref<Shader>> shaders);
	static obj<PipelineLayout> create(obj<Device> device, cref<ProgramReflection> program_reflection);
	static obj<PipelineLayout> create(obj<Device> device, array_view<cref<ShaderReflection>> shader_reflections);
	static obj<PipelineLayout> create(obj<Device> device, const PipelineLayoutCreateInfo& info);
	~PipelineLayout() VERA_NOEXCEPT override;

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;

	VERA_NODISCARD uint32_t getDescriptorSetLayoutCount() const VERA_NOEXCEPT;
	VERA_NODISCARD obj<DescriptorSetLayout> getDescriptorSetLayout(uint32_t set) VERA_NOEXCEPT;
	VERA_NODISCARD array_view<ref<DescriptorSetLayout>> enumerateDescriptorSetLayouts() const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<PushConstantRange> getPushConstantRanges() const VERA_NOEXCEPT;

	VERA_NODISCARD bool isCompatible(cref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;
	VERA_NODISCARD bool isDescriptorSetLayoutCompatible(cref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;
	VERA_NODISCARD bool isPushConstantCompatible(cref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;

	VERA_NODISCARD size_t hash() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END