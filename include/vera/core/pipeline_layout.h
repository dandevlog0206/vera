#pragma once

#include "device.h"
#include "shader_layout.h"
#include "descriptor_set_layout.h"
#include "shader_reflection.h"
#include "../util/array_view.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class ShaderReflection;

struct PipelineLayoutCreateInfo
{
	std::vector<obj<DescriptorSetLayout>> descriptorSetLayouts = {};
	std::vector<PushConstantRange>        pushConstantRanges   = {};
};

class PipelineLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(PipelineLayout)
public:
	static obj<PipelineLayout> create(obj<Device> device, array_view<obj<Shader>> shaders);
	static obj<PipelineLayout> create(obj<Device> device, obj<ShaderReflection> shader_reflection);
	static obj<PipelineLayout> create(obj<Device> device, const PipelineLayoutCreateInfo& info);
	static obj<PipelineLayout> create(
		obj<Device>                          device,
		array_view<obj<DescriptorSetLayout>> set_layouts,
		array_view<PushConstantRange>        pc_ranges);
	~PipelineLayout();

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;
	VERA_NODISCARD obj<DescriptorSetLayout> getDescriptorSetLayout(uint32_t set) VERA_NOEXCEPT;
	VERA_NODISCARD obj<ShaderReflection> getShaderReflection() VERA_NOEXCEPT;

	VERA_NODISCARD uint32_t getDescriptorSetLayoutCount() const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<ref<DescriptorSetLayout>> getDescriptorSetLayouts() const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<PushConstantRange> getPushConstantRanges() const VERA_NOEXCEPT;

	VERA_NODISCARD bool isCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;
	VERA_NODISCARD bool isDescriptorSetLayoutCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;
	VERA_NODISCARD bool isPushConstantCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;

	VERA_NODISCARD size_t hash() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END