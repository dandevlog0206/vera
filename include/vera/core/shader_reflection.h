#pragma once

#include "core_object.h"
#include "reflection.h"
#include "../math/vector_types.h"
#include "../util/version.h"
#include "../util/array_view.h"
#include <string_view>


VERA_NAMESPACE_BEGIN

class Device;
class Shader;

class ShaderReflection : public CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderReflection)
public:
	static VERA_NODISCARD obj<ShaderReflection> create(obj<Device> device, cref<Shader> shader);
	static VERA_NODISCARD obj<ShaderReflection> create(obj<Device> device, array_view<uint32_t> spirv_code);
	~ShaderReflection() VERA_NOEXCEPT override;

	VERA_NODISCARD Version getSpirvVersion() const VERA_NOEXCEPT;
	VERA_NODISCARD ShaderStageFlags getStageFlags() const VERA_NOEXCEPT;
	VERA_NODISCARD std::string_view getEntryPointName() const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<const ReflectionInterfaceVariable*> enumerateInterfaceVariables() const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<const ReflectionInterfaceVariable*> enumerateInputVariables() const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<const ReflectionInterfaceVariable*> enumerateOutputVariables() const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<const ReflectionDescriptorBinding*> enumerateDescriptorBindings() const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<const ReflectionDescriptorBinding*> enumerateDescriptorBindings(uint32_t set) const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<const ReflectionDescriptorSet*> enumerateDescriptorSets() const VERA_NOEXCEPT;
	VERA_NODISCARD const ReflectionDescriptorBinding* getDescriptorBinding(uint32_t set, uint32_t binding) const VERA_NOEXCEPT;
	VERA_NODISCARD const ReflectionDescriptorSet* getDescriptorSet(uint32_t set) const VERA_NOEXCEPT;

	VERA_NODISCARD const ReflectionBlockVariable* getPushConstantBlock() const VERA_NOEXCEPT;

	VERA_NODISCARD uint3 getLocalSize() const VERA_NOEXCEPT;

	array_view<ReflectionSpecConstant> enumerateSpecConstants() const VERA_NOEXCEPT;

	VERA_NODISCARD hash_t hash() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END
