#pragma once

#include "core_object.h"
#include "../math/vector_types.h"
#include "../math/matrix_types.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

enum class TextureLayout VERA_ENUM;

class Device;
class ShaderReflection;
class Sampler;
class TextureView;
class Buffer;
class BufferView;
class ShaderStorageData;
struct ReflectionDesc;

class ShaderVariable
{
	friend class ShaderStorage;
	ShaderVariable(
		ref<ShaderStorage> storage,
		ShaderStorageData* data,
		ReflectionDesc* desc,
		uint32_t           offset);
public:
	VERA_NODISCARD ShaderVariable operator[](std::string_view name) VERA_NOEXCEPT;
	VERA_NODISCARD ShaderVariable operator[](uint32_t idx) VERA_NOEXCEPT;

	VERA_NODISCARD ShaderVariable at(std::string_view name);
	VERA_NODISCARD ShaderVariable at(uint32_t idx);

	template <class T>
	void operator=(const T& value);
	void operator=(obj<Sampler> obj);
	void operator=(obj<TextureView> obj);
	void operator=(obj<BufferView> obj);
	void operator=(obj<Buffer> obj);

	void setSampler(obj<Sampler> sampler);
	void setTextureView(obj<TextureView> texture_view);
	void setBufferView(obj<BufferView> buffer_view);
	void setBuffer(obj<Buffer> buffer);

private:
	ref<ShaderStorage> m_storage;
	ShaderStorageData* m_data;
	ReflectionDesc*    m_desc;
	uint32_t           m_offset;
};

class ShaderStorage : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderStorage)
public:
	static obj<ShaderStorage> create(obj<ShaderReflection> reflection);
	~ShaderStorage();

	obj<Device> getDevice() VERA_NOEXCEPT;
	obj<ShaderReflection> getShaderReflection() VERA_NOEXCEPT;

	VERA_NODISCARD ShaderVariable accessVariable(std::string_view name);

	void setSampler(
		const ShaderVariable& variable,
		obj<Sampler>          sampler);

	void setTextureView(
		const ShaderVariable& variable,
		obj<TextureView>      texture_view,
		TextureLayout         texture_layout); // how to deal with invalidation?
	
	void setBufferView(
		const ShaderVariable& variable,
		obj<BufferView>       buffer_view);

	void setBuffer(
		const ShaderVariable& variable,
		obj<Buffer>           buffer,
		size_t                offset = 0,
		size_t                range  = 0);

	// primitive types
	void setPrimitive(const ShaderVariable& variable, const bool value);
	void setPrimitive(const ShaderVariable& variable, const int8_t value);
	void setPrimitive(const ShaderVariable& variable, const uint8_t value);
	void setPrimitive(const ShaderVariable& variable, const int16_t value);
	void setPrimitive(const ShaderVariable& variable, const uint16_t value);
	void setPrimitive(const ShaderVariable& variable, const int32_t value);
	void setPrimitive(const ShaderVariable& variable, const uint32_t value);
	void setPrimitive(const ShaderVariable& variable, const int64_t value);
	void setPrimitive(const ShaderVariable& variable, const uint64_t value);
	void setPrimitive(const ShaderVariable& variable, const float value);
	void setPrimitive(const ShaderVariable& variable, const double value);

	// vector types
	void setPrimitive(const ShaderVariable& variable, const bool2& value);
	void setPrimitive(const ShaderVariable& variable, const bool3& value);
	void setPrimitive(const ShaderVariable& variable, const bool4& value);
	void setPrimitive(const ShaderVariable& variable, const char2& value);
	void setPrimitive(const ShaderVariable& variable, const char3& value);
	void setPrimitive(const ShaderVariable& variable, const char4& value);
	void setPrimitive(const ShaderVariable& variable, const uchar2& value);
	void setPrimitive(const ShaderVariable& variable, const uchar3& value);
	void setPrimitive(const ShaderVariable& variable, const uchar4& value);
	void setPrimitive(const ShaderVariable& variable, const short2& value);
	void setPrimitive(const ShaderVariable& variable, const short3& value);
	void setPrimitive(const ShaderVariable& variable, const short4& value);
	void setPrimitive(const ShaderVariable& variable, const ushort2& value);
	void setPrimitive(const ShaderVariable& variable, const ushort3& value);
	void setPrimitive(const ShaderVariable& variable, const ushort4& value);
	void setPrimitive(const ShaderVariable& variable, const int2& value);
	void setPrimitive(const ShaderVariable& variable, const int3& value);
	void setPrimitive(const ShaderVariable& variable, const int4& value);
	void setPrimitive(const ShaderVariable& variable, const uint2& value);
	void setPrimitive(const ShaderVariable& variable, const uint3& value);
	void setPrimitive(const ShaderVariable& variable, const uint4& value);
	void setPrimitive(const ShaderVariable& variable, const long2& value);
	void setPrimitive(const ShaderVariable& variable, const long3& value);
	void setPrimitive(const ShaderVariable& variable, const long4& value);
	void setPrimitive(const ShaderVariable& variable, const ulong2& value);
	void setPrimitive(const ShaderVariable& variable, const ulong3& value);
	void setPrimitive(const ShaderVariable& variable, const ulong4& value);
	void setPrimitive(const ShaderVariable& variable, const float2& value);
	void setPrimitive(const ShaderVariable& variable, const float3& value);
	void setPrimitive(const ShaderVariable& variable, const float4& value);
	void setPrimitive(const ShaderVariable& variable, const double2& value);
	void setPrimitive(const ShaderVariable& variable, const double3& value);
	void setPrimitive(const ShaderVariable& variable, const double4& value);

	// matrix types
	void setPrimitive(const ShaderVariable& variable, const float2x2& value);
	void setPrimitive(const ShaderVariable& variable, const float2x3& value);
	void setPrimitive(const ShaderVariable& variable, const float2x4& value);
	void setPrimitive(const ShaderVariable& variable, const float3x2& value);
	void setPrimitive(const ShaderVariable& variable, const float3x3& value);
	void setPrimitive(const ShaderVariable& variable, const float3x4& value);
	void setPrimitive(const ShaderVariable& variable, const float4x2& value);
	void setPrimitive(const ShaderVariable& variable, const float4x3& value);
	void setPrimitive(const ShaderVariable& variable, const float4x4& value);
	void setPrimitive(const ShaderVariable& variable, const double2x2& value);
	void setPrimitive(const ShaderVariable& variable, const double2x3& value);
	void setPrimitive(const ShaderVariable& variable, const double2x4& value);
	void setPrimitive(const ShaderVariable& variable, const double3x2& value);
	void setPrimitive(const ShaderVariable& variable, const double3x3& value);
	void setPrimitive(const ShaderVariable& variable, const double3x4& value);
	void setPrimitive(const ShaderVariable& variable, const double4x2& value);
	void setPrimitive(const ShaderVariable& variable, const double4x3& value);
	void setPrimitive(const ShaderVariable& variable, const double4x4& value);

	void tryWrite();
};

template <class T>
void ShaderVariable::operator=(const T& value)
{
	m_storage->setPrimitive(*this, value);
}

VERA_NAMESPACE_END