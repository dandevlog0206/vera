#pragma once

#include "device.h"
#include "shader_reflection.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

class ShaderReflection;

class Shader : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Shader)
public:
	static obj<Shader> create(obj<Device> device, std::string_view path);
	static obj<Shader> create(obj<Device> device, const uint32_t* spirv_code, size_t size_in_byte);
	~Shader();

	VERA_NODISCARD obj<Device> getDevice() VERA_NOEXCEPT;
	VERA_NODISCARD obj<ShaderReflection> getShaderReflection() VERA_NOEXCEPT;

	VERA_NODISCARD size_t hash() const;
};

VERA_NAMESPACE_END