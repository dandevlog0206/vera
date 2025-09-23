#pragma once

#include "../core/core_object.h"
#include "../math/vector_types.h"
#include "../math/matrix_types.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

struct ReflectionDesc;
struct ShaderStorage;
class ShaderReflection;
class Sampler;
class Texture;
class Buffer;

class ShaderVariable
{
	friend class ShaderParameter;
	ShaderVariable(ShaderStorage* storage, ReflectionDesc* desc, uint32_t offset);
public:
	ShaderVariable operator[](std::string_view name);
	ShaderVariable operator[](size_t idx);

	template <class T>
	void operator=(const T& value)
	{
		setValue(value);
	}

	// primitive types
	void setValue(bool value);
	void setValue(int8_t value);
	void setValue(uint8_t value);
	void setValue(int16_t value);
	void setValue(uint16_t value);
	void setValue(int32_t value);
	void setValue(uint32_t value);
	void setValue(int64_t value);
	void setValue(uint64_t value);
	void setValue(float value);
	void setValue(double value);

	// vector types
	void setValue(const bool2& value);
	void setValue(const bool3& value);
	void setValue(const bool4& value);
	void setValue(const char2& value);
	void setValue(const char3& value);
	void setValue(const char4& value);
	void setValue(const uchar2& value);
	void setValue(const uchar3& value);
	void setValue(const uchar4& value);
	void setValue(const short2& value);
	void setValue(const short3& value);
	void setValue(const short4& value);
	void setValue(const ushort2& value);
	void setValue(const ushort3& value);
	void setValue(const ushort4& value);
	void setValue(const int2& value);
	void setValue(const int3& value);
	void setValue(const int4& value);
	void setValue(const uint2& value);
	void setValue(const uint3& value);
	void setValue(const uint4& value);
	void setValue(const long2& value);
	void setValue(const long3& value);
	void setValue(const long4& value);
	void setValue(const ulong2& value);
	void setValue(const ulong3& value);
	void setValue(const ulong4& value);
	void setValue(const float2& value);
	void setValue(const float3& value);
	void setValue(const float4& value);
	void setValue(const double2& value);
	void setValue(const double3& value);
	void setValue(const double4& value);

	// matrix types
	void setValue(const float2x2& value);
	void setValue(const float2x3& value);
	void setValue(const float2x4& value);
	void setValue(const float3x2& value);
	void setValue(const float3x3& value);
	void setValue(const float3x4& value);
	void setValue(const float4x2& value);
	void setValue(const float4x3& value);
	void setValue(const float4x4& value);
	void setValue(const double2x2& value);
	void setValue(const double2x3& value);
	void setValue(const double2x4& value);
	void setValue(const double3x2& value);
	void setValue(const double3x3& value);
	void setValue(const double3x4& value);
	void setValue(const double4x2& value);
	void setValue(const double4x3& value);
	void setValue(const double4x4& value);

	void operator=(ref<Sampler> obj);
	void setSampler(ref<Sampler> sampler);
	ref<Sampler> getSampler();

	void operator=(ref<Texture> obj);
	void setTexture(ref<Texture> texture);
	ref<Texture> getTexture();

	void operator=(ref<Buffer> obj);
	void setBuffer(ref<Buffer> buffer);
	ref<Buffer> getBuffer();

private:
	ShaderStorage*  m_storage;
	ReflectionDesc* m_desc;
	uint32_t        m_offset;
};

VERA_NAMESPACE_END