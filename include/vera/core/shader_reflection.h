#pragma once

#include "core_object.h"
#include "enum_types.h"
#include "../util/array_view.h"

VERA_NAMESPACE_BEGIN

class PipelineLayout;
class Shader;
class ReflectionNode;

class ShaderReflectionBuilder
{
public:
};

class ReflectionVariable
{
public:
	ReflectionNode* m_node;
};

class ShaderReflection : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderReflection)
public:
	static VERA_NODISCARD obj<ShaderReflection> create(const ShaderReflectionBuilder& builder);
	static VERA_NODISCARD obj<ShaderReflection> create(const_ref<PipelineLayout> pipeline_layout);
	static VERA_NODISCARD obj<ShaderReflection> create(array_view<const_ref<Shader>> shaders);
	static VERA_NODISCARD obj<ShaderReflection> create(const_ref<Shader> shader);
	static VERA_NODISCARD obj<ShaderReflection> create(const uint32_t* spirv, size_t size_in_byte);
	~ShaderReflection() VERA_NOEXCEPT;

	ShaderStageFlags getShaderStageFlags() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END
