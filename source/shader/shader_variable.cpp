#include "../../include/vera/shader/shader_variable.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/resource_layout_impl.h"
#include "../impl/sampler_impl.h"
#include "../impl/texture_impl.h"
#include "../impl/buffer_impl.h"

#include "../../include/vera/shader/shader_parameter.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/buffer.h"

VERA_NAMESPACE_BEGIN

template <class T>
static constexpr auto primitive_type_v = ReflectionPrimitiveType::Unknown;
template <> static constexpr auto primitive_type_v<bool2>     = ReflectionPrimitiveType::Bool_2;
template <> static constexpr auto primitive_type_v<bool3>     = ReflectionPrimitiveType::Bool_3;
template <> static constexpr auto primitive_type_v<bool4>     = ReflectionPrimitiveType::Bool_4;
template <> static constexpr auto primitive_type_v<char2>     = ReflectionPrimitiveType::Int8_2;
template <> static constexpr auto primitive_type_v<char3>     = ReflectionPrimitiveType::Int8_3;
template <> static constexpr auto primitive_type_v<char4>     = ReflectionPrimitiveType::Int8_4;
template <> static constexpr auto primitive_type_v<uchar2>    = ReflectionPrimitiveType::UInt8_2;
template <> static constexpr auto primitive_type_v<uchar3>    = ReflectionPrimitiveType::UInt8_3;
template <> static constexpr auto primitive_type_v<uchar4>    = ReflectionPrimitiveType::UInt8_4;
template <> static constexpr auto primitive_type_v<short2>    = ReflectionPrimitiveType::Int16_2;
template <> static constexpr auto primitive_type_v<short3>    = ReflectionPrimitiveType::Int16_3;
template <> static constexpr auto primitive_type_v<short4>    = ReflectionPrimitiveType::Int16_4;
template <> static constexpr auto primitive_type_v<ushort2>   = ReflectionPrimitiveType::UInt16_2;
template <> static constexpr auto primitive_type_v<ushort3>   = ReflectionPrimitiveType::UInt16_3;
template <> static constexpr auto primitive_type_v<ushort4>   = ReflectionPrimitiveType::UInt16_4;
template <> static constexpr auto primitive_type_v<int2>      = ReflectionPrimitiveType::Int32_2;
template <> static constexpr auto primitive_type_v<int3>      = ReflectionPrimitiveType::Int32_3;
template <> static constexpr auto primitive_type_v<int4>      = ReflectionPrimitiveType::Int32_4;
template <> static constexpr auto primitive_type_v<uint2>     = ReflectionPrimitiveType::UInt32_2;
template <> static constexpr auto primitive_type_v<uint3>     = ReflectionPrimitiveType::UInt32_3;
template <> static constexpr auto primitive_type_v<uint4>     = ReflectionPrimitiveType::UInt32_4;
template <> static constexpr auto primitive_type_v<long2>     = ReflectionPrimitiveType::Int64_2;
template <> static constexpr auto primitive_type_v<long3>     = ReflectionPrimitiveType::Int64_3;
template <> static constexpr auto primitive_type_v<long4>     = ReflectionPrimitiveType::Int64_4;
template <> static constexpr auto primitive_type_v<ulong2>    = ReflectionPrimitiveType::UInt64_2;
template <> static constexpr auto primitive_type_v<ulong3>    = ReflectionPrimitiveType::UInt64_3;
template <> static constexpr auto primitive_type_v<ulong4>    = ReflectionPrimitiveType::UInt64_4;
template <> static constexpr auto primitive_type_v<float2>    = ReflectionPrimitiveType::Float_2;
template <> static constexpr auto primitive_type_v<float3>    = ReflectionPrimitiveType::Float_3;
template <> static constexpr auto primitive_type_v<float4>    = ReflectionPrimitiveType::Float_4;
template <> static constexpr auto primitive_type_v<float2x2>  = ReflectionPrimitiveType::Float_2x2;
template <> static constexpr auto primitive_type_v<float2x3>  = ReflectionPrimitiveType::Float_2x3;
template <> static constexpr auto primitive_type_v<float2x4>  = ReflectionPrimitiveType::Float_2x4;
template <> static constexpr auto primitive_type_v<float3x2>  = ReflectionPrimitiveType::Float_3x2;
template <> static constexpr auto primitive_type_v<float3x3>  = ReflectionPrimitiveType::Float_3x3;
template <> static constexpr auto primitive_type_v<float3x4>  = ReflectionPrimitiveType::Float_3x4;
template <> static constexpr auto primitive_type_v<float4x2>  = ReflectionPrimitiveType::Float_4x2;
template <> static constexpr auto primitive_type_v<float4x3>  = ReflectionPrimitiveType::Float_4x3;
template <> static constexpr auto primitive_type_v<float4x4>  = ReflectionPrimitiveType::Float_4x4;
template <> static constexpr auto primitive_type_v<double2>   = ReflectionPrimitiveType::Double_2;
template <> static constexpr auto primitive_type_v<double3>   = ReflectionPrimitiveType::Double_3;
template <> static constexpr auto primitive_type_v<double4>   = ReflectionPrimitiveType::Double_4;
template <> static constexpr auto primitive_type_v<double2x2> = ReflectionPrimitiveType::Double_2x2;
template <> static constexpr auto primitive_type_v<double2x3> = ReflectionPrimitiveType::Double_2x3;
template <> static constexpr auto primitive_type_v<double2x4> = ReflectionPrimitiveType::Double_2x4;
template <> static constexpr auto primitive_type_v<double3x2> = ReflectionPrimitiveType::Double_3x2;
template <> static constexpr auto primitive_type_v<double3x3> = ReflectionPrimitiveType::Double_3x3;
template <> static constexpr auto primitive_type_v<double3x4> = ReflectionPrimitiveType::Double_3x4;
template <> static constexpr auto primitive_type_v<double4x2> = ReflectionPrimitiveType::Double_4x2;
template <> static constexpr auto primitive_type_v<double4x3> = ReflectionPrimitiveType::Double_4x3;
template <> static constexpr auto primitive_type_v<double4x4> = ReflectionPrimitiveType::Double_4x4;

static uint32_t get_offset(ReflectionDesc& desc)
{
	switch (desc.type) {
	case ReflectionType::Primitive: return static_cast<ReflectionPrimitiveDesc*>(&desc)->offset;
	case ReflectionType::Array:     return static_cast<ReflectionArrayDesc*>(&desc)->offset;
	case ReflectionType::Struct:    return static_cast<ReflectionStructDesc*>(&desc)->offset;
	}
	return 0;
}

template <class StorageType>
static void resize_resource_array_storage(StorageType& storage, size_t idx)
{
	if (storage.storages.size() <= idx) {
		if (storage.elementCount == UINT32_MAX)
			storage.storages.resize(idx + 1);
		else
			throw Exception("resource array subscript out of range");
	}
}

template <class MemberDesc>
static int32_t find_member(const MemberDesc& desc, const char* name)
{
	int32_t lo = 0;
	int32_t hi = desc.memberCount - 1;

	while (lo <= hi) {
		int32_t mid = lo + (hi - lo) / 2;
		int32_t cmp = strcmp(desc.members[mid]->name, name);

		if (cmp == 0) {
			return mid;
		} else if (cmp < 0) {
			lo = mid + 1;
		} else /* 0 < cmp */ {
			hi = mid - 1;
		}
	}

	return -1;
}

template <class T>
static void store_scalar_impl(ShaderStorageData* storage_ptr, ReflectionDesc* desc_ptr, uint32_t offset, const T& value)
{
	VERA_ASSERT(desc_ptr && desc_ptr->type == ReflectionType::Primitive);
	VERA_ASSERT(storage_ptr && (storage_ptr->storageType == ShaderStorageDataType::BufferBlock ||
		storage_ptr->storageType == ShaderStorageDataType::PushConstant));

	auto& desc    = *static_cast<ReflectionPrimitiveDesc*>(desc_ptr);
	auto& storage = *static_cast<BlockStorage*>(storage_ptr);
	auto* data    = storage.blockStorage.data() + offset;

	// automatic type casting
	switch (desc.primitiveType) {
	case ReflectionPrimitiveType::Bool:
		*reinterpret_cast<bool*>(data) = static_cast<bool>(value);
		break;
	case ReflectionPrimitiveType::Int8:
		*reinterpret_cast<int8_t*>(data) = static_cast<int8_t>(value);
		break;
	case ReflectionPrimitiveType::UInt8:
		*reinterpret_cast<uint8_t*>(data) = static_cast<uint8_t>(value);
		break;
	case ReflectionPrimitiveType::Int16:
		*reinterpret_cast<int16_t*>(data) = static_cast<int16_t>(value);
		break;
	case ReflectionPrimitiveType::UInt16:
		*reinterpret_cast<uint16_t*>(data) = static_cast<uint16_t>(value);
		break;
	case ReflectionPrimitiveType::Int32:
		*reinterpret_cast<int32_t*>(data) = static_cast<int32_t>(value);
		break;
	case ReflectionPrimitiveType::UInt32:
		*reinterpret_cast<uint32_t*>(data) = static_cast<uint32_t>(value);
		break;
	case ReflectionPrimitiveType::Int64:
		*reinterpret_cast<int64_t*>(data) = static_cast<int64_t>(value);
		break;
	case ReflectionPrimitiveType::UInt64:
		*reinterpret_cast<uint64_t*>(data) = static_cast<uint64_t>(value);
		break;
	case ReflectionPrimitiveType::Float:
		*reinterpret_cast<float*>(data) = static_cast<float>(value);
		break;
	case ReflectionPrimitiveType::Double:
		*reinterpret_cast<double*>(data) = static_cast<double>(value);
		break;
	}
}

template <class Type>
static void store_primitive_impl(ShaderStorageData* storage_ptr, ReflectionDesc* desc_ptr, uint32_t offset, const Type& value)
{
	VERA_ASSERT(desc_ptr && desc_ptr->type == ReflectionType::Primitive);
	VERA_ASSERT(storage_ptr->storageType == ShaderStorageDataType::BufferBlock ||
		storage_ptr->storageType == ShaderStorageDataType::PushConstant);

	auto& desc    = *static_cast<ReflectionPrimitiveDesc*>(desc_ptr);
	auto& storage = *static_cast<BlockStorage*>(storage_ptr);

	VERA_ASSERT(desc.primitiveType == primitive_type_v<Type>);

	// GLSL and GLM have different matrix layout
	if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Float_2x3) {
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 0])  = value[0];
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 16]) = value[1];
	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Float_3x3) {
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 0])  = value[0];
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 16]) = value[1];
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 32]) = value[2];
	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Float_4x3) {
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 0])  = value[0];
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 16]) = value[1];
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 32]) = value[2];
		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 48]) = value[3];
	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Double_2x3) {
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 0])  = value[0];
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 16]) = value[1];
	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Double_3x3) {
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 0])  = value[0];
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 16]) = value[1];
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 32]) = value[2];
	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Double_4x3) {
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 0])  = value[0];
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 16]) = value[1];
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 32]) = value[2];
		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 48]) = value[3];
	} else {
		*reinterpret_cast<Type*>(&storage.blockStorage[offset]) = value;
	}
}

ShaderVariable::ShaderVariable(ShaderStorageData* storage, ReflectionDesc* desc, uint32_t offset) :
	m_storage(storage),
	m_desc(desc),
	m_offset(offset) {}

ShaderVariable ShaderVariable::operator[](std::string_view name)
{

}

ShaderVariable ShaderVariable::operator[](size_t idx)
{

}

void ShaderVariable::operator=(obj<Sampler> obj)
{
	setSampler(std::move(obj));
}

void ShaderVariable::operator=(obj<Texture> obj)
{
	setTexture(std::move(obj));
}

void ShaderVariable::operator=(obj<Buffer> obj)
{
	setBuffer(std::move(obj));
}

void ShaderVariable::setValue(bool value)
{
	store_scalar_impl<bool>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(int8_t value)
{
	store_scalar_impl<int8_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(uint8_t value)
{
	store_scalar_impl<uint8_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(int16_t value)
{
	store_scalar_impl<int16_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(uint16_t value)
{
	store_scalar_impl<uint16_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(int32_t value)
{
	store_scalar_impl<int32_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(uint32_t value)
{
	store_scalar_impl<uint32_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(int64_t value)
{
	store_scalar_impl<int64_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(uint64_t value)
{
	store_scalar_impl<uint64_t>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(float value)
{
	store_scalar_impl<float>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(double value)
{
	store_scalar_impl<double>(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const bool2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const bool3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const bool4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const char2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const char3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const char4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const uchar2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const uchar3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const uchar4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const short2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const short3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const short4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const ushort2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const ushort3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const ushort4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const int2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const int3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const int4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const uint2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const uint3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const uint4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const long2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const long3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const long4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const ulong2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const ulong3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const ulong4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float2x2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float2x3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float2x4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float3x2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float3x3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float3x4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float4x2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float4x3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const float4x4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double2x2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double2x3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double2x4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double3x2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double3x3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double3x4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double4x2& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double4x3& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setValue(const double4x4& value)
{
	store_primitive_impl(m_storage, m_desc, m_offset, value);
}

void ShaderVariable::setSampler(obj<Sampler> sampler)
{

}

obj<Sampler> ShaderVariable::getSampler()
{

}

void ShaderVariable::setTexture(obj<Texture> texture)
{

}

obj<Texture> ShaderVariable::getTexture()
{

}

void ShaderVariable::setBuffer(obj<Buffer> buffer)
{

}

obj<Buffer> ShaderVariable::getBuffer()
{

}

VERA_NAMESPACE_END