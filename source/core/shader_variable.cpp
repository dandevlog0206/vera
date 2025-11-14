#include "../../include/vera/core/shader_variable.h"
#include "../spirv/reflection_desc.h"
#include "../impl/shader_parameter_impl.h"

#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

template <class T>
static constexpr auto primitive_type_v = ReflectionPrimitiveType::Unknown;
template <> static constexpr auto primitive_type_v<bool2>     = ReflectionPrimitiveType::Bool2;
template <> static constexpr auto primitive_type_v<bool3>     = ReflectionPrimitiveType::Bool3;
template <> static constexpr auto primitive_type_v<bool4>     = ReflectionPrimitiveType::Bool4;
template <> static constexpr auto primitive_type_v<char2>     = ReflectionPrimitiveType::Char2;
template <> static constexpr auto primitive_type_v<char3>     = ReflectionPrimitiveType::Char3;
template <> static constexpr auto primitive_type_v<char4>     = ReflectionPrimitiveType::Char4;
template <> static constexpr auto primitive_type_v<uchar2>    = ReflectionPrimitiveType::UChar2;
template <> static constexpr auto primitive_type_v<uchar3>    = ReflectionPrimitiveType::UChar3;
template <> static constexpr auto primitive_type_v<uchar4>    = ReflectionPrimitiveType::UChar4;
template <> static constexpr auto primitive_type_v<short2>    = ReflectionPrimitiveType::Short2;
template <> static constexpr auto primitive_type_v<short3>    = ReflectionPrimitiveType::Short3;
template <> static constexpr auto primitive_type_v<short4>    = ReflectionPrimitiveType::Short4;
template <> static constexpr auto primitive_type_v<ushort2>   = ReflectionPrimitiveType::UShort2;
template <> static constexpr auto primitive_type_v<ushort3>   = ReflectionPrimitiveType::UShort3;
template <> static constexpr auto primitive_type_v<ushort4>   = ReflectionPrimitiveType::UShort4;
template <> static constexpr auto primitive_type_v<int2>      = ReflectionPrimitiveType::Int2;
template <> static constexpr auto primitive_type_v<int3>      = ReflectionPrimitiveType::Int3;
template <> static constexpr auto primitive_type_v<int4>      = ReflectionPrimitiveType::Int4;
template <> static constexpr auto primitive_type_v<uint2>     = ReflectionPrimitiveType::UInt2;
template <> static constexpr auto primitive_type_v<uint3>     = ReflectionPrimitiveType::UInt3;
template <> static constexpr auto primitive_type_v<uint4>     = ReflectionPrimitiveType::UInt4;
template <> static constexpr auto primitive_type_v<long2>     = ReflectionPrimitiveType::Long2;
template <> static constexpr auto primitive_type_v<long3>     = ReflectionPrimitiveType::Long3;
template <> static constexpr auto primitive_type_v<long4>     = ReflectionPrimitiveType::Long4;
template <> static constexpr auto primitive_type_v<ulong2>    = ReflectionPrimitiveType::ULong2;
template <> static constexpr auto primitive_type_v<ulong3>    = ReflectionPrimitiveType::ULong3;
template <> static constexpr auto primitive_type_v<ulong4>    = ReflectionPrimitiveType::ULong4;
template <> static constexpr auto primitive_type_v<float2>    = ReflectionPrimitiveType::Float2;
template <> static constexpr auto primitive_type_v<float3>    = ReflectionPrimitiveType::Float3;
template <> static constexpr auto primitive_type_v<float4>    = ReflectionPrimitiveType::Float4;
template <> static constexpr auto primitive_type_v<float2x2>  = ReflectionPrimitiveType::Float2x2;
template <> static constexpr auto primitive_type_v<float2x3>  = ReflectionPrimitiveType::Float2x3;
template <> static constexpr auto primitive_type_v<float2x4>  = ReflectionPrimitiveType::Float2x4;
template <> static constexpr auto primitive_type_v<float3x2>  = ReflectionPrimitiveType::Float3x2;
template <> static constexpr auto primitive_type_v<float3x3>  = ReflectionPrimitiveType::Float3x3;
template <> static constexpr auto primitive_type_v<float3x4>  = ReflectionPrimitiveType::Float3x4;
template <> static constexpr auto primitive_type_v<float4x2>  = ReflectionPrimitiveType::Float4x2;
template <> static constexpr auto primitive_type_v<float4x3>  = ReflectionPrimitiveType::Float4x3;
template <> static constexpr auto primitive_type_v<float4x4>  = ReflectionPrimitiveType::Float4x4;
template <> static constexpr auto primitive_type_v<double2>   = ReflectionPrimitiveType::Double2;
template <> static constexpr auto primitive_type_v<double3>   = ReflectionPrimitiveType::Double3;
template <> static constexpr auto primitive_type_v<double4>   = ReflectionPrimitiveType::Double4;
template <> static constexpr auto primitive_type_v<double2x2> = ReflectionPrimitiveType::Double2x2;
template <> static constexpr auto primitive_type_v<double2x3> = ReflectionPrimitiveType::Double2x3;
template <> static constexpr auto primitive_type_v<double2x4> = ReflectionPrimitiveType::Double2x4;
template <> static constexpr auto primitive_type_v<double3x2> = ReflectionPrimitiveType::Double3x2;
template <> static constexpr auto primitive_type_v<double3x3> = ReflectionPrimitiveType::Double3x3;
template <> static constexpr auto primitive_type_v<double3x4> = ReflectionPrimitiveType::Double3x4;
template <> static constexpr auto primitive_type_v<double4x2> = ReflectionPrimitiveType::Double4x2;
template <> static constexpr auto primitive_type_v<double4x3> = ReflectionPrimitiveType::Double4x3;
template <> static constexpr auto primitive_type_v<double4x4> = ReflectionPrimitiveType::Double4x4;

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

bool ShaderVariable::isRoot() const VERA_NOEXCEPT
{
	return m_node && m_node->getType() == ReflectionType::Root;
}

bool ShaderVariable::isPushConstant() const VERA_NOEXCEPT
{
	return m_node && m_node->getType() == ReflectionType::PushConstant;
}

bool ShaderVariable::isDescriptor() const VERA_NOEXCEPT
{
	return
		m_node && m_node->getPropertyFlags().has(ReflectionPropertyFlagBits::DescriptorType);
}

DescriptorType ShaderVariable::getDescriptorType() const VERA_NOEXCEPT
{
	if (isDescriptor())
		return m_node->getDescriptorType();

	return DescriptorType::Unknown;
}

ShaderVariable ShaderVariable::operator[](std::string_view name) VERA_NOEXCEPT
{
	if (m_node->getPropertyFlags().has(ReflectionPropertyFlagBits::NameMap)) {
		
	}
	
	VERA_ERROR_MSG("variable is not subscriptable with name");
}

ShaderVariable ShaderVariable::operator[](uint32_t idx) VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(!isRoot(), "root variable is not subscriptable");

	if (m_node->getType() == ReflectionType::DescriptorArray) {

	} else if (m_node->getType() == ReflectionType::Array) {
	}

	VERA_ERROR_MSG("variable is not subscriptable");
}

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

void ShaderVariable::setValue(const float2x2& value)
{
}

void ShaderVariable::setValue(const float2x3& value)
{
}

void ShaderVariable::setValue(const float2x4& value)
{
}

void ShaderVariable::setValue(const float3x2& value)
{
}

void ShaderVariable::setValue(const float3x3& value)
{
}

void ShaderVariable::setValue(const float3x4& value)
{
}

void ShaderVariable::setValue(const float4x2& value)
{
}

void ShaderVariable::setValue(const float4x3& value)
{
}

void ShaderVariable::setValue(const float4x4& value)
{
}

void ShaderVariable::setValue(const double2x2& value)
{
}

void ShaderVariable::setValue(const double2x3& value)
{
}

void ShaderVariable::setValue(const double2x4& value)
{
}

void ShaderVariable::setValue(const double3x2& value)
{
}

void ShaderVariable::setValue(const double3x3& value)
{
}

void ShaderVariable::setValue(const double3x4& value)
{
}

void ShaderVariable::setValue(const double4x2& value)
{
}

void ShaderVariable::setValue(const double4x3& value)
{
}

void ShaderVariable::setValue(const double4x4& value)
{
}

bool ShaderVariable::empty() const VERA_NOEXCEPT
{
	return m_impl == nullptr;
}

VERA_NAMESPACE_END
