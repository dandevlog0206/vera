#pragma once

#include "shader.h"
#include "../util/array_view.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class Device;
class DescriptorSetLayout;
class Shader;

enum class PipelineBindPoint VERA_ENUM
{
	Unknown,
	Graphics,
	Compute
};

struct PushConstantRange
{
	uint32_t         offset     = {};
	uint32_t         size       = {};
	ShaderStageFlags stageFlags = {};
};

struct PipelineLayoutCreateInfo
{
	std::vector<obj<DescriptorSetLayout>> descriptorSetLayouts = {};
	std::vector<PushConstantRange>        pushConstantRanges   = {};
	PipelineBindPoint                     pipelineBindPoint    = {};
};

class PipelineLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(PipelineLayout)
public:
	static obj<PipelineLayout> create(obj<Device> device, array_view<const_ref<Shader>> shaders);
	static obj<PipelineLayout> create(obj<Device> device, const PipelineLayoutCreateInfo& info);
	~PipelineLayout();

	VERA_NODISCARD obj<Device> getDevice();

	VERA_NODISCARD uint32_t getResourceLayoutCount() const;
	VERA_NODISCARD obj<DescriptorSetLayout> getDescriptorSetLayout(uint32_t set) const;
	VERA_NODISCARD array_view<ref<DescriptorSetLayout>> getDescriptorSetLayouts() const;

	VERA_NODISCARD array_view<PushConstantRange> getPushConstantRanges() const;

	VERA_NODISCARD PipelineBindPoint getPipelineBindPoint() const;
	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const;

	VERA_NODISCARD bool hasReflection() const VERA_NOEXCEPT;

	VERA_NODISCARD bool isCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;
	VERA_NODISCARD bool isResourceCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;
	VERA_NODISCARD bool isPushConstantCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT;

	size_t hash() const;
};

VERA_NAMESPACE_END