#pragma once

#include "core_object.h"
#include "enum_types.h"
#include "../math/vector_types.h"
#include "../util/array_view.h"
#include <string_view>
#include <vector>
#include <array>

VERA_NAMESPACE_BEGIN

class PipelineLayout;
class Shader;
class ReflectionNode;

class ShaderReflectionBuilder
{
public:
};

struct ShaderReflectionDescriptorDesc
{
	ShaderStageFlags        stageFlags;
	DescriptorType          type;
	uint32_t                set;
	uint32_t                binding;
	uint32_t                descriptorCount;
	uint32_t                arrayDimensions;
	std::array<uint32_t, 8> arraySizes;
};

class ReflectionVariable
{
	friend class ShaderReflection;
	ReflectionVariable(
		const ReflectionNode* node,
		ShaderStageFlags      stage_mask,
		uint32_t              offset) VERA_NOEXCEPT;
public:
	ReflectionVariable() VERA_NOEXCEPT;

	VERA_NODISCARD ReflectionVariable operator[](uint32_t idx) const VERA_NOEXCEPT;
	VERA_NODISCARD ReflectionVariable operator[](std::string_view name) const VERA_NOEXCEPT;
	VERA_NODISCARD ReflectionVariable at(uint32_t idx) const VERA_NOEXCEPT;
	VERA_NODISCARD ReflectionVariable at(std::string_view name) const VERA_NOEXCEPT;

	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const VERA_NOEXCEPT;

	VERA_NODISCARD ReflectionVariableType getType() const VERA_NOEXCEPT;

	// Array
	VERA_NODISCARD bool isSubscriptible() const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t getElementCount() const VERA_NOEXCEPT;

	VERA_NODISCARD bool hasMembers() const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	const ReflectionNode* m_node;
	ShaderStageFlags      m_stage_mask;
	uint32_t              m_offset;
};

class ShaderReflection : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderReflection)
public:
	static VERA_NODISCARD obj<ShaderReflection> create(ref<Shader> shader);
	~ShaderReflection() VERA_NOEXCEPT;

	VERA_NODISCARD ReflectionTargetFlags getTargetFlags() const VERA_NOEXCEPT;
	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const;

	VERA_NODISCARD ReflectionVariable getVariable(std::string_view name);
	VERA_NODISCARD ReflectionVariable getDescriptorVariable(uint32_t set, uint32_t binding) const VERA_NOEXCEPT;
	VERA_NODISCARD ReflectionVariable getPushConstantVariable(ShaderStageFlags flags) const VERA_NOEXCEPT;

	VERA_NODISCARD std::vector<ReflectionVariable> enumerateVariables() const VERA_NOEXCEPT;
	VERA_NODISCARD std::vector<ReflectionVariable> enumerateDescriptorVariables() const VERA_NOEXCEPT;
	VERA_NODISCARD std::vector<ReflectionVariable> enumeratePushConstantVariables() const VERA_NOEXCEPT;

	VERA_NODISCARD uint3 getLocalSize() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END
