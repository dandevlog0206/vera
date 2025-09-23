#pragma once

#include "core_object.h"
#include "../util/flag.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

class Device;
class Shader;

enum class ShaderStageFlagBits VERA_FLAG_BITS
{
	Vertex                 = 1 << 0,
	TessellationControl    = 1 << 1,
	TessellationEvaluation = 1 << 2,
	Geometry               = 1 << 3,
	Fragment               = 1 << 4,
	Compute                = 1 << 5,
} VERA_ENUM_FLAGS(ShaderStageFlagBits, ShaderStageFlags)

class Shader : protected CoreObject
{
	VERA_CORE_OBJECT(Shader)
public:
	static ref<Shader> create(ref<Device> device, std::string_view path);
	static ref<Shader> create(ref<Device> device, const uint32_t* spirv_code, size_t size_in_byte);
	~Shader();

	ref<Device> getDevice();
	
	ShaderStageFlags getShaderStageFlags() const;

	size_t hash() const;
};

VERA_NAMESPACE_END