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

ShaderVariable::ShaderVariable(
	ShaderParameterImpl*         impl,
	const ReflectionNode*        node,
	ShaderParameterBlockStorage* block,
	uint32_t                     offset
) :
	m_impl(impl),
	m_node(node),
	m_block(block),
	m_offset(offset) {}

bool ShaderVariable::isRoot() const VERA_NOEXCEPT
{
	return m_node->type == ReflectionNodeType::Root;
}

bool ShaderVariable::isArray() const VERA_NOEXCEPT
{
	return
		m_node->type == ReflectionNodeType::DescriptorArray ||
		m_node->type == ReflectionNodeType::Array;
}

bool ShaderVariable::isStruct() const VERA_NOEXCEPT
{
	return
		m_node->type == ReflectionNodeType::DescriptorBlock ||
		m_node->type == ReflectionNodeType::PushConstant ||
		m_node->type == ReflectionNodeType::Struct;
}

ShaderVariable ShaderVariable::at(std::string_view name) const
{
	if (empty())
		throw Exception("attempt to access member of an empty variable");

	const ReflectionNameMap* name_map = nullptr;
	
	if (m_node->hasProperty(ReflectionPropertyFlagBits::NameMap)) {
		name_map = &m_node->getNameMap();
	} else if (m_node->hasProperty(ReflectionPropertyFlagBits::Block)) {
		name_map = &m_node->getBlock()->getNameMap();
	} else {
		throw Exception("variable does not have any member");
	}

	if (auto it = name_map->find(name); it != name_map->end()) {
		const ReflectionNode* member_node = it->second;
		
		if (m_node->type == ReflectionNodeType::Struct) {
			return ShaderVariable(
				m_impl,
				it->second,
				m_block,
				m_offset + member_node->getOffset());

		} else if (m_node->type == ReflectionNodeType::Root) {
			ShaderParameterBlockStorage* block = nullptr;

			if (member_node->type == ReflectionNodeType::PushConstant) {
				block = &m_impl->pushConstantStorage;
			} else {
				auto& set_state = m_impl->setStates[member_node->getSet()];
				auto  it        = set_state.bindingRanges.find(member_node->getBinding());

				if (it == set_state.bindingRanges.end())
					throw Exception("invalid descriptor array member access");

				block = &set_state.blockStorages[it->second.blockRange.first()];
			}

			return { m_impl, it->second, block, 0 };
		} else {
			return { m_impl, it->second, m_block, 0 };
		}
	}

	throw Exception("cannot find member named '{}'", name);
}

ShaderVariable ShaderVariable::at(uint32_t idx) const
{
	if (empty())
		throw Exception("attempt to access member of an empty variable");
	if (!m_node->hasProperty(ReflectionPropertyFlagBits::ElementNode))
		throw Exception("variable is not an array");
	if (idx >= m_node->getElementCount())
		throw Exception("array index out of bounds");

	if (m_node->type == ReflectionNodeType::Array) {
		return {
			m_impl,
			m_node->getElementNode(),
			m_block,
			m_offset + idx * m_node->getStride()
		};
	} else /* m_node->type == ReflectionNodeType::DescriptorArray */ {
		return {
			m_impl,
			m_node->getElementNode(),
			m_block + idx,
			0
		};
	}
}

ShaderVariable ShaderVariable::operator[](std::string_view name) const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(!empty(), "attempt to access member of an empty variable");

	const ReflectionNameMap* name_map = nullptr;
	
	if (m_node->hasProperty(ReflectionPropertyFlagBits::NameMap)) {
		name_map = &m_node->getNameMap();
	} else if (m_node->hasProperty(ReflectionPropertyFlagBits::Block)) {
		name_map = &m_node->getBlock()->getNameMap();
	} else {
		VERA_ERROR_MSG("variable does not have any member");
	}

	if (auto it = name_map->find(name); it != name_map->end()) {
		const ReflectionNode* member_node = it->second;
		
		if (m_node->type == ReflectionNodeType::Struct) {
			return ShaderVariable(
				m_impl,
				it->second,
				m_block,
				m_offset + member_node->getOffset());

		} else if (m_node->type == ReflectionNodeType::Root) {
			ShaderParameterBlockStorage* block = nullptr;

			if (member_node->type == ReflectionNodeType::PushConstant) {
				block = &m_impl->pushConstantStorage;
			} else {
				auto& set_state = m_impl->setStates[member_node->getSet()];
				auto  it        = set_state.bindingRanges.find(member_node->getBinding());

				VERA_ASSERT_MSG(it != set_state.bindingRanges.end(),
					"invalid descriptor array member access");

				block = &set_state.blockStorages[it->second.blockRange.first()];
			}

			return { m_impl, it->second, block, 0 };
		} else {
			return { m_impl, it->second, m_block, 0 };
		}
	}

	VERA_ERROR_MSG("cannot find member");
}

ShaderVariable ShaderVariable::operator[](uint32_t idx) const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(!empty(), "attempt to access member of an empty variable");
	VERA_ASSERT_MSG(m_node->hasProperty(ReflectionPropertyFlagBits::ElementNode), "variable is not an array");
	VERA_ASSERT_MSG(idx < m_node->getElementCount(), "array index out of bounds");

	if (m_node->type == ReflectionNodeType::Array) {
		return {
			m_impl,
			m_node->getElementNode(),
			m_block,
			m_offset + idx * m_node->getStride()
		};
	} else /* m_node->type == ReflectionNodeType::DescriptorArray */ {
		return {
			m_impl,
			m_node->getElementNode(),
			m_block + idx,
			0
		};
	}
}

void ShaderVariable::setSampler(obj<Sampler> sampler)
{
}

void ShaderVariable::setTextureView(obj<TextureView> texture_view)
{
}

void ShaderVariable::setBufferView(obj<BufferView> buffer_view)
{
}

void ShaderVariable::setBuffer(obj<Buffer> buffer, size_t offset, size_t range)
{
}

void setValue(const float value)
{

}

void setValue(const float4& value)
{

}

bool ShaderVariable::empty() const VERA_NOEXCEPT
{
	return m_impl == nullptr;
}

VERA_NAMESPACE_END
