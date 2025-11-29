#pragma once

#include "core_object.h"
#include "reflection.h"
#include "../util/array_view.h"

VERA_NAMESPACE_BEGIN

class Device;
class Pipeline;
class ShaderReflection;

class ProgramReflection : public CoreObject
{
	VERA_CORE_OBJECT_INIT(ProgramReflection)
public:
	static VERA_NODISCARD obj<ProgramReflection> create(obj<Device> device, obj<Pipeline> pipeline);
	static VERA_NODISCARD obj<ProgramReflection> create(
		obj<Device>                       device,
		array_view<obj<ShaderReflection>> shader_reflections);
	~ProgramReflection() VERA_NOEXCEPT override;

	VERA_NODISCARD obj<Device> getDevice() const VERA_NOEXCEPT;

	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const VERA_NOEXCEPT;
	VERA_NODISCARD PipelineBindPoint getPipelineBindPoint() const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<obj<ShaderReflection>> enumerateShaderReflections() const VERA_NOEXCEPT;
	VERA_NODISCARD obj<ShaderReflection> getShaderReflection(ShaderStageFlags stage_flag) const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<ReflectionEntryPoint> enumerateEntryPoints() const VERA_NOEXCEPT;
	VERA_NODISCARD const ReflectionEntryPoint* getEntryPoint(ShaderStageFlags stage_flag) const VERA_NOEXCEPT;

	VERA_NODISCARD hash_t hash() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END
