#pragma once

#include "core_object.h"
#include "../util/array_view.h"

VERA_NAMESPACE_BEGIN

class Device;
class ShaderReflection;

class ProgramReflection : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ProgramReflection)
public:
	static VERA_NODISCARD obj<ProgramReflection> create(
		obj<Device>                       device,
		array_view<obj<ShaderReflection>> shader_reflections);
	~ProgramReflection() VERA_NOEXCEPT override;

	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const;

	VERA_NODISCARD hash_t hash() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END
