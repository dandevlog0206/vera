#pragma once

#include "core_object.h"
#include "../util/flag.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

class Device;
class ResourceLayout;

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
	VERA_CORE_OBJECT_INIT(Shader)
public:
	static obj<Shader> create(obj<Device> device, std::string_view path);
	static obj<Shader> create(obj<Device> device, const uint32_t* spirv_code, size_t size_in_byte);
	~Shader();

	obj<Device> getDevice();
	obj<ResourceLayout> getResourceLayout(uint32_t set);
	
	ShaderStageFlags getShaderStageFlags() const;

	size_t hash() const;
};

VERA_NAMESPACE_END