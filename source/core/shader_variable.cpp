#include "../../include/vera/core/shader_variable.h"
#include "../spirv/reflection_node.h"
#include "../impl/shader_parameter_impl.h"

#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
#include "../../include/vera/core/reflection.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

template <class T>
static constexpr auto primitive_type_v = ReflectionPrimitiveType::Unknown;
template <> static constexpr auto primitive_type_v<bool>       = ReflectionPrimitiveType::Bool;
template <> static constexpr auto primitive_type_v<int8_t>     = ReflectionPrimitiveType::Char;
template <> static constexpr auto primitive_type_v<uint8_t>    = ReflectionPrimitiveType::UChar;
template <> static constexpr auto primitive_type_v<int16_t>    = ReflectionPrimitiveType::Short;
template <> static constexpr auto primitive_type_v<uint16_t>   = ReflectionPrimitiveType::UShort;
template <> static constexpr auto primitive_type_v<int32_t>    = ReflectionPrimitiveType::Int;
template <> static constexpr auto primitive_type_v<uint32_t>   = ReflectionPrimitiveType::UInt;
template <> static constexpr auto primitive_type_v<int64_t>    = ReflectionPrimitiveType::Long;
template <> static constexpr auto primitive_type_v<uint64_t>   = ReflectionPrimitiveType::ULong;
template <> static constexpr auto primitive_type_v<float>      = ReflectionPrimitiveType::Float;
template <> static constexpr auto primitive_type_v<double>     = ReflectionPrimitiveType::Double;
template <> static constexpr auto primitive_type_v<bool3>      = ReflectionPrimitiveType::Bool3;
template <> static constexpr auto primitive_type_v<bool4>      = ReflectionPrimitiveType::Bool4;
template <> static constexpr auto primitive_type_v<char2>      = ReflectionPrimitiveType::Char2;
template <> static constexpr auto primitive_type_v<char3>      = ReflectionPrimitiveType::Char3;
template <> static constexpr auto primitive_type_v<char4>      = ReflectionPrimitiveType::Char4;
template <> static constexpr auto primitive_type_v<uchar2>     = ReflectionPrimitiveType::UChar2;
template <> static constexpr auto primitive_type_v<uchar3>     = ReflectionPrimitiveType::UChar3;
template <> static constexpr auto primitive_type_v<uchar4>     = ReflectionPrimitiveType::UChar4;
template <> static constexpr auto primitive_type_v<short2>     = ReflectionPrimitiveType::Short2;
template <> static constexpr auto primitive_type_v<short3>     = ReflectionPrimitiveType::Short3;
template <> static constexpr auto primitive_type_v<short4>     = ReflectionPrimitiveType::Short4;
template <> static constexpr auto primitive_type_v<ushort2>    = ReflectionPrimitiveType::UShort2;
template <> static constexpr auto primitive_type_v<ushort3>    = ReflectionPrimitiveType::UShort3;
template <> static constexpr auto primitive_type_v<ushort4>    = ReflectionPrimitiveType::UShort4;
template <> static constexpr auto primitive_type_v<int2>       = ReflectionPrimitiveType::Int2;
template <> static constexpr auto primitive_type_v<int3>       = ReflectionPrimitiveType::Int3;
template <> static constexpr auto primitive_type_v<int4>       = ReflectionPrimitiveType::Int4;
template <> static constexpr auto primitive_type_v<uint2>      = ReflectionPrimitiveType::UInt2;
template <> static constexpr auto primitive_type_v<uint3>      = ReflectionPrimitiveType::UInt3;
template <> static constexpr auto primitive_type_v<uint4>      = ReflectionPrimitiveType::UInt4;
template <> static constexpr auto primitive_type_v<long2>      = ReflectionPrimitiveType::Long2;
template <> static constexpr auto primitive_type_v<long3>      = ReflectionPrimitiveType::Long3;
template <> static constexpr auto primitive_type_v<long4>      = ReflectionPrimitiveType::Long4;
template <> static constexpr auto primitive_type_v<ulong2>     = ReflectionPrimitiveType::ULong2;
template <> static constexpr auto primitive_type_v<ulong3>     = ReflectionPrimitiveType::ULong3;
template <> static constexpr auto primitive_type_v<ulong4>     = ReflectionPrimitiveType::ULong4;
template <> static constexpr auto primitive_type_v<float2>     = ReflectionPrimitiveType::Float2;
template <> static constexpr auto primitive_type_v<float3>     = ReflectionPrimitiveType::Float3;
template <> static constexpr auto primitive_type_v<float4>     = ReflectionPrimitiveType::Float4;
template <> static constexpr auto primitive_type_v<double2>    = ReflectionPrimitiveType::Double2;
template <> static constexpr auto primitive_type_v<double3>    = ReflectionPrimitiveType::Double3;
template <> static constexpr auto primitive_type_v<double4>    = ReflectionPrimitiveType::Double4;
template <> static constexpr auto primitive_type_v<rfloat2x2>  = ReflectionPrimitiveType::RFloat2x2;
template <> static constexpr auto primitive_type_v<rfloat2x3>  = ReflectionPrimitiveType::RFloat2x3;
template <> static constexpr auto primitive_type_v<rfloat2x4>  = ReflectionPrimitiveType::RFloat2x4;
template <> static constexpr auto primitive_type_v<rfloat3x2>  = ReflectionPrimitiveType::RFloat3x2;
template <> static constexpr auto primitive_type_v<rfloat3x3>  = ReflectionPrimitiveType::RFloat3x3;
template <> static constexpr auto primitive_type_v<rfloat3x4>  = ReflectionPrimitiveType::RFloat3x4;
template <> static constexpr auto primitive_type_v<rfloat4x2>  = ReflectionPrimitiveType::RFloat4x2;
template <> static constexpr auto primitive_type_v<rfloat4x3>  = ReflectionPrimitiveType::RFloat4x3;
template <> static constexpr auto primitive_type_v<rfloat4x4>  = ReflectionPrimitiveType::RFloat4x4;
template <> static constexpr auto primitive_type_v<rdouble2x2> = ReflectionPrimitiveType::RDouble2x2;
template <> static constexpr auto primitive_type_v<rdouble2x3> = ReflectionPrimitiveType::RDouble2x3;
template <> static constexpr auto primitive_type_v<rdouble2x4> = ReflectionPrimitiveType::RDouble2x4;
template <> static constexpr auto primitive_type_v<rdouble3x2> = ReflectionPrimitiveType::RDouble3x2;
template <> static constexpr auto primitive_type_v<rdouble3x3> = ReflectionPrimitiveType::RDouble3x3;
template <> static constexpr auto primitive_type_v<rdouble3x4> = ReflectionPrimitiveType::RDouble3x4;
template <> static constexpr auto primitive_type_v<rdouble4x2> = ReflectionPrimitiveType::RDouble4x2;
template <> static constexpr auto primitive_type_v<rdouble4x3> = ReflectionPrimitiveType::RDouble4x3;
template <> static constexpr auto primitive_type_v<rdouble4x4> = ReflectionPrimitiveType::RDouble4x4;
template <> static constexpr auto primitive_type_v<cfloat2x2>  = ReflectionPrimitiveType::CFloat2x2;
template <> static constexpr auto primitive_type_v<cfloat2x3>  = ReflectionPrimitiveType::CFloat2x3;
template <> static constexpr auto primitive_type_v<cfloat2x4>  = ReflectionPrimitiveType::CFloat2x4;
template <> static constexpr auto primitive_type_v<cfloat3x2>  = ReflectionPrimitiveType::CFloat3x2;
template <> static constexpr auto primitive_type_v<cfloat3x3>  = ReflectionPrimitiveType::CFloat3x3;
template <> static constexpr auto primitive_type_v<cfloat3x4>  = ReflectionPrimitiveType::CFloat3x4;
template <> static constexpr auto primitive_type_v<cfloat4x2>  = ReflectionPrimitiveType::CFloat4x2;
template <> static constexpr auto primitive_type_v<cfloat4x3>  = ReflectionPrimitiveType::CFloat4x3;
template <> static constexpr auto primitive_type_v<cfloat4x4>  = ReflectionPrimitiveType::CFloat4x4;
template <> static constexpr auto primitive_type_v<cdouble2x2> = ReflectionPrimitiveType::CDouble2x2;
template <> static constexpr auto primitive_type_v<cdouble2x3> = ReflectionPrimitiveType::CDouble2x3;
template <> static constexpr auto primitive_type_v<cdouble2x4> = ReflectionPrimitiveType::CDouble2x4;
template <> static constexpr auto primitive_type_v<cdouble3x2> = ReflectionPrimitiveType::CDouble3x2;
template <> static constexpr auto primitive_type_v<cdouble3x3> = ReflectionPrimitiveType::CDouble3x3;
template <> static constexpr auto primitive_type_v<cdouble3x4> = ReflectionPrimitiveType::CDouble3x4;
template <> static constexpr auto primitive_type_v<cdouble4x2> = ReflectionPrimitiveType::CDouble4x2;
template <> static constexpr auto primitive_type_v<cdouble4x3> = ReflectionPrimitiveType::CDouble4x3;
template <> static constexpr auto primitive_type_v<cdouble4x4> = ReflectionPrimitiveType::CDouble4x4;

ShaderVariable::ShaderVariable(ShaderParameterImpl* impl) :
	m_impl(impl),
	m_node(nullptr),
	m_array_idx(0),
	m_offset(0) {}

ShaderVariable::ShaderVariable(
	ShaderParameterImpl*  impl,
	const ReflectionNode* node
) :
	m_impl(impl),
	m_node(node),
	m_array_idx(0),
	m_offset(0) {}

ShaderVariable::ShaderVariable(
	ShaderParameterImpl*  impl,
	const ReflectionNode* node,
	uint32_t              array_idx
) :
	m_impl(impl),
	m_node(node),
	m_array_idx(array_idx),
	m_offset(0) {}

ShaderVariable::ShaderVariable(
	ShaderParameterImpl*  impl,
	const ReflectionNode* node,
	uint32_t              array_idx,
	uint32_t              offset
) :
	m_impl(impl),
	m_node(node),
	m_array_idx(array_idx),
	m_offset(offset) {}

void ShaderVariable::setTextureView(obj<TextureView> texture_view, TextureLayout layout)
{

}

void ShaderVariable::setValue(const bool value)
{
}

void ShaderVariable::setValue(const int8_t value)
{
}

void ShaderVariable::setValue(const uint8_t value)
{
}

void ShaderVariable::setValue(const int16_t value)
{
}

void ShaderVariable::setValue(const uint16_t value)
{
}

void ShaderVariable::setValue(const int32_t value)
{
}

void ShaderVariable::setValue(const uint32_t value)
{
}

void ShaderVariable::setValue(const int64_t value)
{
}

void ShaderVariable::setValue(const uint64_t value)
{
}

void ShaderVariable::setValue(const float value)
{
}

void ShaderVariable::setValue(const double value)
{
}

void ShaderVariable::setValue(const bool2& value)
{
}

void ShaderVariable::setValue(const bool3& value)
{
}

void ShaderVariable::setValue(const bool4& value)
{
}

void ShaderVariable::setValue(const char2& value)
{
}

void ShaderVariable::setValue(const char3& value)
{
}

void ShaderVariable::setValue(const char4& value)
{
}

void ShaderVariable::setValue(const uchar2& value)
{
}

void ShaderVariable::setValue(const uchar3& value)
{
}

void ShaderVariable::setValue(const uchar4& value)
{
}

void ShaderVariable::setValue(const short2& value)
{
}

void ShaderVariable::setValue(const short3& value)
{
}

void ShaderVariable::setValue(const short4& value)
{
}

void ShaderVariable::setValue(const ushort2& value)
{
}

void ShaderVariable::setValue(const ushort3& value)
{
}

void ShaderVariable::setValue(const ushort4& value)
{
}

void ShaderVariable::setValue(const int2& value)
{
}

void ShaderVariable::setValue(const int3& value)
{
}

void ShaderVariable::setValue(const int4& value)
{
}

void ShaderVariable::setValue(const uint2& value)
{
}

void ShaderVariable::setValue(const uint3& value)
{
}

void ShaderVariable::setValue(const uint4& value)
{
}

void ShaderVariable::setValue(const long2& value)
{
}

void ShaderVariable::setValue(const long3& value)
{
}

void ShaderVariable::setValue(const long4& value)
{
}

void ShaderVariable::setValue(const ulong2& value)
{
}

void ShaderVariable::setValue(const ulong3& value)
{
}

void ShaderVariable::setValue(const ulong4& value)
{
}

void ShaderVariable::setValue(const float2& value)
{
}

void ShaderVariable::setValue(const float3& value)
{
}

void ShaderVariable::setValue(const float4& value)
{
}

void ShaderVariable::setValue(const double2& value)
{
}

void ShaderVariable::setValue(const double3& value)
{
}

void ShaderVariable::setValue(const double4& value)
{
}

bool ShaderVariable::empty() const VERA_NOEXCEPT
{
	return m_impl == nullptr;
}

VERA_NAMESPACE_END
