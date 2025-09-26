#pragma once

#include "format.h"
#include "../math/matrix_types.h"
#include "../math/vector_types.h"

#define VERA_VERTEX_DESCRIPTOR_BEGIN(vertex_name) \
private:                                    \
	friend class VertexInputDescriptor;     \
	using this_type = vertex_name;          \
	static constexpr std::initializer_list<::vr::VertexInputAttribute> __get_attributes() { return { 

#define VERA_VERTEX_DESCRIPTOR_END }; }

#define VERA_VERTEX_ATTRIBUTE(id, name) \
	{ id, offsetof(this_type, name), ::vr::priv::vertex_format_v<decltype(name)> }

VERA_NAMESPACE_BEGIN

enum class VertexFormat VERA_ENUM
{
	Unknown,
	Char,
	Char2,
	Char3,
	Char4,
	UChar,
	UChar2,
	UChar3,
	UChar4,
	Short,
	Short2,
	Short3,
	Short4,
	UShort,
	UShort2,
	UShort3,
	UShort4,
	Int,
	Int2,
	Int3,
	Int4,
	UInt,
	UInt2,
	UInt3,
	UInt4,
	Long,
	Long2,
	Long3,
	Long4,
	ULong,
	ULong2,
	ULong3,
	ULong4,
	Float,
	Float2,
	Float3,
	Float4,
	Float2x2,
	Float2x3,
	Float2x4,
	Float3x2,
	Float3x3,
	Float3x4,
	Float4x2,
	Float4x3,
	Float4x4,
	Double,
	Double2,
	Double3,
	Double4,
	Double2x2,
	Double2x3,
	Double2x4,
	Double3x2,
	Double3x3,
	Double3x4,
	Double4x2,
	Double4x3,
	Double4x4
};

struct VertexInputAttribute
{
	uint32_t     id;
	uint32_t     offset;
	VertexFormat format;
};

class VertexInputDescriptor
{
public:
	VertexInputDescriptor() = default;

	template <class VertexType>
	VertexInputDescriptor(const VertexType& type) :
		m_size(sizeof(VertexType)),
		m_attribute_count(VertexType::__get_attributes().size()),
		m_attributes(VertexType::__get_attributes().begin()) {
	}

	const VertexInputAttribute& operator[](size_t idx) const
	{
		VERA_ASSERT(idx < m_attribute_count);
		return m_attributes[idx];
	}

	const VertexInputAttribute* attributeData() const
	{
		return m_attributes;
	}

	uint32_t attributeSize() const
	{
		return m_attribute_count;
	}

	uint32_t vertexSize() const
	{
		return m_size;
	}

	bool empty() const
	{
		return !m_attributes;
	}

private:
	uint32_t                    m_size;
	uint32_t                    m_attribute_count;
	const VertexInputAttribute* m_attributes;
};

VERA_PRIV_NAMESPACE_BEGIN

template <class Type>
static constexpr auto vertex_format_v = VertexFormat::Unknown;
template <> static constexpr auto vertex_format_v<char>      = VertexFormat::Char;
template <> static constexpr auto vertex_format_v<char2>     = VertexFormat::Char2;
template <> static constexpr auto vertex_format_v<char3>     = VertexFormat::Char3;
template <> static constexpr auto vertex_format_v<char4>     = VertexFormat::Char4;
template <> static constexpr auto vertex_format_v<uint8_t>   = VertexFormat::UChar;
template <> static constexpr auto vertex_format_v<uchar2>    = VertexFormat::UChar2;
template <> static constexpr auto vertex_format_v<uchar3>    = VertexFormat::UChar3;
template <> static constexpr auto vertex_format_v<uchar4>    = VertexFormat::UChar4;
template <> static constexpr auto vertex_format_v<short>     = VertexFormat::Short;
template <> static constexpr auto vertex_format_v<short2>    = VertexFormat::Short2;
template <> static constexpr auto vertex_format_v<short3>    = VertexFormat::Short3;
template <> static constexpr auto vertex_format_v<short4>    = VertexFormat::Short4;
template <> static constexpr auto vertex_format_v<uint16_t>  = VertexFormat::UShort;
template <> static constexpr auto vertex_format_v<ushort2>   = VertexFormat::UShort2;
template <> static constexpr auto vertex_format_v<ushort3>   = VertexFormat::UShort3;
template <> static constexpr auto vertex_format_v<ushort4>   = VertexFormat::UShort4;
template <> static constexpr auto vertex_format_v<int>       = VertexFormat::Int;
template <> static constexpr auto vertex_format_v<int2>      = VertexFormat::Int2;
template <> static constexpr auto vertex_format_v<int3>      = VertexFormat::Int3;
template <> static constexpr auto vertex_format_v<int4>      = VertexFormat::Int4;
template <> static constexpr auto vertex_format_v<uint32_t>  = VertexFormat::UInt;
template <> static constexpr auto vertex_format_v<uint2>     = VertexFormat::UInt2;
template <> static constexpr auto vertex_format_v<uint3>     = VertexFormat::UInt3;
template <> static constexpr auto vertex_format_v<uint4>     = VertexFormat::UInt4;
template <> static constexpr auto vertex_format_v<long>      = VertexFormat::Long;
template <> static constexpr auto vertex_format_v<long2>     = VertexFormat::Long2;
template <> static constexpr auto vertex_format_v<long3>     = VertexFormat::Long3;
template <> static constexpr auto vertex_format_v<long4>     = VertexFormat::Long4;
template <> static constexpr auto vertex_format_v<uint64_t>  = VertexFormat::ULong;
template <> static constexpr auto vertex_format_v<ulong2>    = VertexFormat::ULong2;
template <> static constexpr auto vertex_format_v<ulong3>    = VertexFormat::ULong3;
template <> static constexpr auto vertex_format_v<ulong4>    = VertexFormat::ULong4;
template <> static constexpr auto vertex_format_v<float>     = VertexFormat::Float;
template <> static constexpr auto vertex_format_v<float2>    = VertexFormat::Float2;
template <> static constexpr auto vertex_format_v<float3>    = VertexFormat::Float3;
template <> static constexpr auto vertex_format_v<float4>    = VertexFormat::Float4;
template <> static constexpr auto vertex_format_v<float2x2>  = VertexFormat::Float2x2;
template <> static constexpr auto vertex_format_v<float2x3>  = VertexFormat::Float2x3;
template <> static constexpr auto vertex_format_v<float2x4>  = VertexFormat::Float2x4;
template <> static constexpr auto vertex_format_v<float3x2>  = VertexFormat::Float3x2;
template <> static constexpr auto vertex_format_v<float3x3>  = VertexFormat::Float3x3;
template <> static constexpr auto vertex_format_v<float3x4>  = VertexFormat::Float3x4;
template <> static constexpr auto vertex_format_v<float4x2>  = VertexFormat::Float4x2;
template <> static constexpr auto vertex_format_v<float4x3>  = VertexFormat::Float4x3;
template <> static constexpr auto vertex_format_v<float4x4>  = VertexFormat::Float4x4;
template <> static constexpr auto vertex_format_v<double>    = VertexFormat::Double;
template <> static constexpr auto vertex_format_v<double2>   = VertexFormat::Double2;
template <> static constexpr auto vertex_format_v<double3>   = VertexFormat::Double3;
template <> static constexpr auto vertex_format_v<double4>   = VertexFormat::Double4;
template <> static constexpr auto vertex_format_v<double2x2> = VertexFormat::Double2x2;
template <> static constexpr auto vertex_format_v<double2x3> = VertexFormat::Double2x3;
template <> static constexpr auto vertex_format_v<double2x4> = VertexFormat::Double2x4;
template <> static constexpr auto vertex_format_v<double3x2> = VertexFormat::Double3x2;
template <> static constexpr auto vertex_format_v<double3x3> = VertexFormat::Double3x3;
template <> static constexpr auto vertex_format_v<double3x4> = VertexFormat::Double3x4;
template <> static constexpr auto vertex_format_v<double4x2> = VertexFormat::Double4x2;
template <> static constexpr auto vertex_format_v<double4x3> = VertexFormat::Double4x3;
template <> static constexpr auto vertex_format_v<double4x4> = VertexFormat::Double4x4;

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END