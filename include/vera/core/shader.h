#pragma once

#include "device.h"
#include "shader_layout.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

class ShaderReflection;

class Shader : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Shader)
public:
	static obj<Shader> create(obj<Device> device, std::string_view path);
	static obj<Shader> create(obj<Device> device, std::vector<uint32_t>&& spirv_code);
	static obj<Shader> create(obj<Device> device, array_view<uint32_t> spirv_code);
	~Shader();

	VERA_NODISCARD obj<Device> getDevice() const VERA_NOEXCEPT;
	VERA_NODISCARD obj<ShaderLayout> getShaderLayout() const VERA_NOEXCEPT;

	VERA_NODISCARD size_t hash() const;
};

VERA_NAMESPACE_END