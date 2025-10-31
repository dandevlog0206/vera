#pragma once

#include "core_object.h"
#include "../util/flag.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

class Device;

VERA_VK_ABI_COMPATIBLE enum class ShaderStageFlagBits VERA_FLAG_BITS
{
	Vertex                 = 1 << 0,
	TessellationControl    = 1 << 1,
	TessellationEvaluation = 1 << 2,
	Geometry               = 1 << 3,
	Fragment               = 1 << 4,
	Compute                = 1 << 5,
	Task                   = 1 << 6,
	Mesh                   = 1 << 7,
	RayGen                 = 1 << 8,
	AnyHit                 = 1 << 9,
	ClosestHit             = 1 << 10,
	Miss                   = 1 << 11,
	Intersection           = 1 << 12,
	Callable               = 1 << 13,
} VERA_ENUM_FLAGS(ShaderStageFlagBits, ShaderStageFlags)

class Shader : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Shader)
public:
	static obj<Shader> create(obj<Device> device, std::string_view path);
	static obj<Shader> create(obj<Device> device, const uint32_t* spirv_code, size_t size_in_byte);
	~Shader();

	obj<Device> getDevice();
	
	ShaderStageFlags getShaderStageFlags() const;

	size_t hash() const;
};

VERA_NAMESPACE_END