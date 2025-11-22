#pragma once

#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/enum_types.h"
#include "../../include/vera/util/array_view.h"
#include "../../include/vera/util/version.h"
#include <spirv-headers/spirv.hpp>
#include <memory_resource>
#include <vector>
#include <bitset>
#include <utility>
#include <assert.h>


VERA_NAMESPACE_BEGIN

//  31      24 23     16 15      8 7        0
// |-- zero --|-- row --|-- col --|-- type --|
#define MAKE_BASIC_TYPE(type, row, col)           \
	(static_cast<uint32_t>(type) |                \
	((static_cast<uint32_t>(row) & 0xFF) << 16) | \
	((static_cast<uint32_t>(col) & 0xFF) << 8))

enum class SpvBasicType VERA_ENUM
{
	Unknown    = 0,
	Void       = 1,
	Bool       = 2,
	Int8       = 3,
	UInt8      = 4,
	Int16      = 5,
	UInt16     = 6,
	Int32      = 7,
	UInt32     = 8,
	Int64      = 9,
	UInt64     = 10,
	Float16    = 11,	
	Float32    = 12,
	Float64    = 13,
	Float8E4M3 = 12,
	Float8E5M2 = 13,

	// vector types
	Bool_2    = MAKE_BASIC_TYPE(Bool, 2, 1),
	Bool_3    = MAKE_BASIC_TYPE(Bool, 3, 1),
	Bool_4    = MAKE_BASIC_TYPE(Bool, 4, 1),
	Int8_2    = MAKE_BASIC_TYPE(Int8, 2, 1),
	Int8_3    = MAKE_BASIC_TYPE(Int8, 3, 1),
	Int8_4    = MAKE_BASIC_TYPE(Int8, 4, 1),
	UInt8_2   = MAKE_BASIC_TYPE(UInt8, 2, 1),
	UInt8_3   = MAKE_BASIC_TYPE(UInt8, 3, 1),
	UInt8_4   = MAKE_BASIC_TYPE(UInt8, 4, 1),
	Int16_2   = MAKE_BASIC_TYPE(Int16, 2, 1),
	Int16_3   = MAKE_BASIC_TYPE(Int16, 3, 1),
	Int16_4   = MAKE_BASIC_TYPE(Int16, 4, 1),
	UInt16_2  = MAKE_BASIC_TYPE(UInt16, 2, 1),
	UInt16_3  = MAKE_BASIC_TYPE(UInt16, 3, 1),
	UInt16_4  = MAKE_BASIC_TYPE(UInt16, 4, 1),
	Int32_2   = MAKE_BASIC_TYPE(Int32, 2, 1),
	Int32_3   = MAKE_BASIC_TYPE(Int32, 3, 1),
	Int32_4   = MAKE_BASIC_TYPE(Int32, 4, 1),
	UInt32_2  = MAKE_BASIC_TYPE(UInt32, 2, 1),
	UInt32_3  = MAKE_BASIC_TYPE(UInt32, 3, 1),
	UInt32_4  = MAKE_BASIC_TYPE(UInt32, 4, 1),
	Int64_2   = MAKE_BASIC_TYPE(Int64, 2, 1),
	Int64_3   = MAKE_BASIC_TYPE(Int64, 3, 1),
	Int64_4   = MAKE_BASIC_TYPE(Int64, 4, 1),
	UInt64_2  = MAKE_BASIC_TYPE(UInt64, 2, 1),
	UInt64_3  = MAKE_BASIC_TYPE(UInt64, 3, 1),
	UInt64_4  = MAKE_BASIC_TYPE(UInt64, 4, 1),
	Float16_2 = MAKE_BASIC_TYPE(Float16, 2, 1),
	Float16_3 = MAKE_BASIC_TYPE(Float16, 3, 1),
	Float16_4 = MAKE_BASIC_TYPE(Float16, 4, 1),
	Float32_2 = MAKE_BASIC_TYPE(Float32, 2, 1),
	Float32_3 = MAKE_BASIC_TYPE(Float32, 3, 1),
	Float32_4 = MAKE_BASIC_TYPE(Float32, 4, 1),
	Float64_2 = MAKE_BASIC_TYPE(Float64, 2, 1),
	Float64_3 = MAKE_BASIC_TYPE(Float64, 3, 1),
	Float64_4 = MAKE_BASIC_TYPE(Float64, 4, 1),

	// matrix types
	Bool_2x2 = MAKE_BASIC_TYPE(Bool, 2, 2),
	Bool_2x3 = MAKE_BASIC_TYPE(Bool, 2, 3),
	Bool_2x4 = MAKE_BASIC_TYPE(Bool, 2, 4),
	Bool_3x2 = MAKE_BASIC_TYPE(Bool, 3, 2),
	Bool_3x3 = MAKE_BASIC_TYPE(Bool, 3, 3),
	Bool_3x4 = MAKE_BASIC_TYPE(Bool, 3, 4),
	Bool_4x2 = MAKE_BASIC_TYPE(Bool, 4, 2),
	Bool_4x3 = MAKE_BASIC_TYPE(Bool, 4, 3),
	Bool_4x4 = MAKE_BASIC_TYPE(Bool, 4, 4),

	Int8_2x2    = MAKE_BASIC_TYPE(Int8, 2, 2),
	Int8_2x3    = MAKE_BASIC_TYPE(Int8, 2, 3),
	Int8_2x4    = MAKE_BASIC_TYPE(Int8, 2, 4),
	Int8_3x2    = MAKE_BASIC_TYPE(Int8, 3, 2),
	Int8_3x3    = MAKE_BASIC_TYPE(Int8, 3, 3),
	Int8_3x4    = MAKE_BASIC_TYPE(Int8, 3, 4),
	Int8_4x2    = MAKE_BASIC_TYPE(Int8, 4, 2),
	Int8_4x3    = MAKE_BASIC_TYPE(Int8, 4, 3),
	Int8_4x4    = MAKE_BASIC_TYPE(Int8, 4, 4),

	UInt8_2x2   = MAKE_BASIC_TYPE(UInt8, 2, 2),
	UInt8_2x3   = MAKE_BASIC_TYPE(UInt8, 2, 3),
	UInt8_2x4   = MAKE_BASIC_TYPE(UInt8, 2, 4),
	UInt8_3x2   = MAKE_BASIC_TYPE(UInt8, 3, 2),
	UInt8_3x3   = MAKE_BASIC_TYPE(UInt8, 3, 3),
	UInt8_3x4   = MAKE_BASIC_TYPE(UInt8, 3, 4),
	UInt8_4x2   = MAKE_BASIC_TYPE(UInt8, 4, 2),
	UInt8_4x3   = MAKE_BASIC_TYPE(UInt8, 4, 3),
	UInt8_4x4   = MAKE_BASIC_TYPE(UInt8, 4, 4),

	Int16_2x2   = MAKE_BASIC_TYPE(Int16, 2, 2),
	Int16_2x3   = MAKE_BASIC_TYPE(Int16, 2, 3),
	Int16_2x4   = MAKE_BASIC_TYPE(Int16, 2, 4),
	Int16_3x2   = MAKE_BASIC_TYPE(Int16, 3, 2),
	Int16_3x3   = MAKE_BASIC_TYPE(Int16, 3, 3),
	Int16_3x4   = MAKE_BASIC_TYPE(Int16, 3, 4),
	Int16_4x2   = MAKE_BASIC_TYPE(Int16, 4, 2),
	Int16_4x3   = MAKE_BASIC_TYPE(Int16, 4, 3),
	Int16_4x4   = MAKE_BASIC_TYPE(Int16, 4, 4),

	UInt16_2x2  = MAKE_BASIC_TYPE(UInt16, 2, 2),
	UInt16_2x3  = MAKE_BASIC_TYPE(UInt16, 2, 3),
	UInt16_2x4  = MAKE_BASIC_TYPE(UInt16, 2, 4),
	UInt16_3x2  = MAKE_BASIC_TYPE(UInt16, 3, 2),
	UInt16_3x3  = MAKE_BASIC_TYPE(UInt16, 3, 3),
	UInt16_3x4  = MAKE_BASIC_TYPE(UInt16, 3, 4),
	UInt16_4x2  = MAKE_BASIC_TYPE(UInt16, 4, 2),
	UInt16_4x3  = MAKE_BASIC_TYPE(UInt16, 4, 3),
	UInt16_4x4  = MAKE_BASIC_TYPE(UInt16, 4, 4),

	Int32_2x2   = MAKE_BASIC_TYPE(Int32, 2, 2),
	Int32_2x3   = MAKE_BASIC_TYPE(Int32, 2, 3),
	Int32_2x4   = MAKE_BASIC_TYPE(Int32, 2, 4),
	Int32_3x2   = MAKE_BASIC_TYPE(Int32, 3, 2),
	Int32_3x3   = MAKE_BASIC_TYPE(Int32, 3, 3),
	Int32_3x4   = MAKE_BASIC_TYPE(Int32, 3, 4),
	Int32_4x2   = MAKE_BASIC_TYPE(Int32, 4, 2),
	Int32_4x3   = MAKE_BASIC_TYPE(Int32, 4, 3),
	Int32_4x4   = MAKE_BASIC_TYPE(Int32, 4, 4),

	UInt32_2x2  = MAKE_BASIC_TYPE(UInt32, 2, 2),
	UInt32_2x3  = MAKE_BASIC_TYPE(UInt32, 2, 3),
	UInt32_2x4  = MAKE_BASIC_TYPE(UInt32, 2, 4),
	UInt32_3x2  = MAKE_BASIC_TYPE(UInt32, 3, 2),
	UInt32_3x3  = MAKE_BASIC_TYPE(UInt32, 3, 3),
	UInt32_3x4  = MAKE_BASIC_TYPE(UInt32, 3, 4),
	UInt32_4x2  = MAKE_BASIC_TYPE(UInt32, 4, 2),
	UInt32_4x3  = MAKE_BASIC_TYPE(UInt32, 4, 3),
	UInt32_4x4  = MAKE_BASIC_TYPE(UInt32, 4, 4),

	Int64_2x2   = MAKE_BASIC_TYPE(Int64, 2, 2),
	Int64_2x3   = MAKE_BASIC_TYPE(Int64, 2, 3),
	Int64_2x4   = MAKE_BASIC_TYPE(Int64, 2, 4),
	Int64_3x2   = MAKE_BASIC_TYPE(Int64, 3, 2),
	Int64_3x3   = MAKE_BASIC_TYPE(Int64, 3, 3),
	Int64_3x4   = MAKE_BASIC_TYPE(Int64, 3, 4),
	Int64_4x2   = MAKE_BASIC_TYPE(Int64, 4, 2),
	Int64_4x3   = MAKE_BASIC_TYPE(Int64, 4, 3),
	Int64_4x4   = MAKE_BASIC_TYPE(Int64, 4, 4),

	UInt64_2x2  = MAKE_BASIC_TYPE(UInt64, 2, 2),
	UInt64_2x3  = MAKE_BASIC_TYPE(UInt64, 2, 3),
	UInt64_2x4  = MAKE_BASIC_TYPE(UInt64, 2, 4),
	UInt64_3x2  = MAKE_BASIC_TYPE(UInt64, 3, 2),
	UInt64_3x3  = MAKE_BASIC_TYPE(UInt64, 3, 3),
	UInt64_3x4  = MAKE_BASIC_TYPE(UInt64, 3, 4),
	UInt64_4x2  = MAKE_BASIC_TYPE(UInt64, 4, 2),
	UInt64_4x3  = MAKE_BASIC_TYPE(UInt64, 4, 3),
	UInt64_4x4  = MAKE_BASIC_TYPE(UInt64, 4, 4),

	Float16_2x2 = MAKE_BASIC_TYPE(Float16, 2, 2),
	Float16_2x3 = MAKE_BASIC_TYPE(Float16, 2, 3),
	Float16_2x4 = MAKE_BASIC_TYPE(Float16, 2, 4),
	Float16_3x2 = MAKE_BASIC_TYPE(Float16, 3, 2),
	Float16_3x3 = MAKE_BASIC_TYPE(Float16, 3, 3),
	Float16_3x4 = MAKE_BASIC_TYPE(Float16, 3, 4),
	Float16_4x2 = MAKE_BASIC_TYPE(Float16, 4, 2),
	Float16_4x3 = MAKE_BASIC_TYPE(Float16, 4, 3),
	Float16_4x4 = MAKE_BASIC_TYPE(Float16, 4, 4),

	Float32_2x2 = MAKE_BASIC_TYPE(Float32, 2, 2),
	Float32_2x3 = MAKE_BASIC_TYPE(Float32, 2, 3),
	Float32_2x4 = MAKE_BASIC_TYPE(Float32, 2, 4),
	Float32_3x2 = MAKE_BASIC_TYPE(Float32, 3, 2),
	Float32_3x3 = MAKE_BASIC_TYPE(Float32, 3, 3),
	Float32_3x4 = MAKE_BASIC_TYPE(Float32, 3, 4),
	Float32_4x2 = MAKE_BASIC_TYPE(Float32, 4, 2),
	Float32_4x3 = MAKE_BASIC_TYPE(Float32, 4, 3),
	Float32_4x4 = MAKE_BASIC_TYPE(Float32, 4, 4),

	Float64_2x2 = MAKE_BASIC_TYPE(Float64, 2, 2),
	Float64_2x3 = MAKE_BASIC_TYPE(Float64, 2, 3),
	Float64_2x4 = MAKE_BASIC_TYPE(Float64, 2, 4),
	Float64_3x2 = MAKE_BASIC_TYPE(Float64, 3, 2),
	Float64_3x3 = MAKE_BASIC_TYPE(Float64, 3, 3),
	Float64_3x4 = MAKE_BASIC_TYPE(Float64, 3, 4),
	Float64_4x2 = MAKE_BASIC_TYPE(Float64, 4, 2),
	Float64_4x3 = MAKE_BASIC_TYPE(Float64, 4, 3),
	Float64_4x4 = MAKE_BASIC_TYPE(Float64, 4, 4),
};

class spv_inst
{
public:
	VERA_INLINE spv_inst() VERA_NOEXCEPT {};

	VERA_INLINE spv_inst(const uint32_t* ptr) VERA_NOEXCEPT :
		m_ptr(ptr) {}

	VERA_INLINE spv::Op op() const VERA_NOEXCEPT
	{
		return static_cast<spv::Op>(m_ptr[0] & 0xFFFF);
	}

	VERA_INLINE uint32_t length() const VERA_NOEXCEPT
	{
		return (m_ptr[0] >> 16) & 0xFFFF;
	}

	VERA_INLINE bool get_bool(uint32_t offset) const VERA_NOEXCEPT
	{
		return static_cast<bool>(m_ptr[offset]);
	}

	VERA_INLINE uint32_t get_u32(uint32_t offset) const VERA_NOEXCEPT
	{
		return m_ptr[offset];
	}

	template <class T>
	VERA_INLINE array_view<T> get_array(uint32_t offset, uint32_t count = UINT32_MAX) const VERA_NOEXCEPT
	{
		if (count == UINT32_MAX) count = length() - offset;
		return array_view<T>(reinterpret_cast<const T*>(&m_ptr[offset]), count);
	}

	template <class T>
	VERA_INLINE T get_enum(uint32_t offset) const VERA_NOEXCEPT
	{
		static_assert(std::is_enum<T>::value, "T must be an enum type");
		static_assert(sizeof(T) <= sizeof(uint32_t), "enum size must be less than or equal to uint32_t");
		
		return static_cast<T>(m_ptr[offset]);
	}

	VERA_INLINE spv::Id get_id(uint32_t offset) const VERA_NOEXCEPT
	{
		return static_cast<spv::Id>(m_ptr[offset]);
	}

	VERA_INLINE std::string_view get_string(uint32_t offset) const VERA_NOEXCEPT
	{
		return reinterpret_cast<const char*>(&m_ptr[offset]);
	}

	VERA_INLINE std::string_view get_string(uint32_t offset, uint32_t& end_offset) const VERA_NOEXCEPT
	{
		const char* str = reinterpret_cast<const char*>(&m_ptr[offset]);
		size_t      len = strlen(str);

		end_offset = offset + static_cast<uint32_t>((len + 4) / 4);

		return std::string_view(str, len);
	}

	VERA_INLINE spv_inst& operator++() VERA_NOEXCEPT
	{
		if (auto len = length(); len != 0) {
			m_ptr += len;
			return *this;
		}

		throw Exception("invalid SPIR-V instruction with length 0");
	}

	VERA_INLINE spv_inst operator++(int) VERA_NOEXCEPT
	{
		if (auto len = length(); len != 0) {
			spv_inst temp = *this;
			m_ptr += len;
			return temp;
		}

		throw Exception("invalid SPIR-V instruction with length 0");
	}

	VERA_INLINE auto operator<=>(const spv_inst& other) const VERA_NOEXCEPT
	{
		return m_ptr <=> other.m_ptr;
	}

	VERA_INLINE bool operator==(const spv_inst& other) const VERA_NOEXCEPT
	{
		return m_ptr == other.m_ptr;
	}

	VERA_INLINE bool operator!=(const spv_inst& other) const VERA_NOEXCEPT
	{
		return m_ptr != other.m_ptr;
	}

private:
	const uint32_t* m_ptr;
};

class spv_decoration
{
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4200) // nonstandard extension used: zero-sized array in struct/union
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif

	struct deco_node
	{
		const deco_node* next;
		spv::Decoration  deco;
		uint32_t         values[];
	};

#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

public:
	spv_decoration() VERA_NOEXCEPT :
		m_mask(),
		m_head(nullptr),
		m_spec_id(0),
		m_array_stride(0),
		m_matrix_stride(0),
		m_builtin(spv::BuiltInMax),
		m_location(0),
		m_binding(0),
		m_desc_set(0),
		m_offset(0) {}

	template <class... Args>
	VERA_INLINE void set(spv::Decoration deco) VERA_NOEXCEPT
	{
		m_mask.set(deco_index(deco));
	}

	VERA_INLINE void set(spv::Decoration deco, std::pmr::memory_resource* memory, uint32_t value_or_id) VERA_NOEXCEPT
	{
		switch (deco) {
		case spv::DecorationSpecId: {
			m_spec_id = value_or_id;
		} break;
		case spv::DecorationArrayStride: {
			m_array_stride = value_or_id;
		} break;
		case spv::DecorationMatrixStride: {
			m_matrix_stride = value_or_id;
		} break;
		case spv::DecorationLocation: {
			m_location = value_or_id;
		} break;
		case spv::DecorationIndex: {
			m_index = value_or_id;
		} break;
		case spv::DecorationBinding: {
			m_binding = value_or_id;
		} break;
		case spv::DecorationDescriptorSet: {
			m_desc_set = value_or_id;
		} break;
		case spv::DecorationOffset: {
			m_offset = value_or_id;
		} break;
		case spv::DecorationAlignment: {
			m_alignment = value_or_id;
		} break;
		// uint32_T value decorations
		case spv::DecorationStream:
		case spv::DecorationComponent:
		case spv::DecorationXfbBuffer:
		case spv::DecorationXfbStride:
		case spv::DecorationInputAttachmentIndex:
		case spv::DecorationMaxByteOffset:
		case spv::DecorationSecondaryViewportRelativeNV:
		// id type decorations
		case spv::DecorationUniformId:
		case spv::DecorationAlignmentId:
		case spv::DecorationMaxByteOffsetId:
		case spv::DecorationNodeSharesPayloadLimitsWithAMDX:
		case spv::DecorationNodeMaxPayloadsAMDX:
		case spv::DecorationPayloadNodeNameAMDX:
		case spv::DecorationPayloadNodeBaseIndexAMDX:
		case spv::DecorationPayloadNodeArraySizeAMDX:
		case spv::DecorationCounterBuffer: {
			auto* new_node = allocate_node(memory, 1);
			new_node->deco      = deco;
			new_node->values[0] = value_or_id;
		}
		}

		m_mask.set(deco_index(deco));
	}

	VERA_INLINE void set(spv::Decoration deco, std::pmr::memory_resource* memory, spv::BuiltIn builtin) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationBuiltIn, "decoration type mismatch");
		m_builtin = builtin;
		m_mask.set(deco_index(deco));
	}

	VERA_INLINE void set(spv::Decoration deco, std::pmr::memory_resource* memory, spv::FunctionParameterAttribute func_param_attr) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationFuncParamAttr, "decoration type mismatch");
		auto* new_node = allocate_node(memory, 1);
		new_node->deco      = deco;
		new_node->values[0] = static_cast<uint32_t>(func_param_attr);
		m_mask.set(deco_index(deco));

	}

	VERA_INLINE void set(spv::Decoration deco, std::pmr::memory_resource* memory, spv::FPRoundingMode fp_rounding_mode) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationFPRoundingMode, "decoration type mismatch");
		auto* new_node = allocate_node(memory, 1);
		new_node->deco      = deco;
		new_node->values[0] = static_cast<uint32_t>(fp_rounding_mode);
		m_mask.set(deco_index(deco));
	}

	VERA_INLINE void set(spv::Decoration deco, std::pmr::memory_resource* memory, spv::FPFastMathModeMask fp_fast_math_mode) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationFPFastMathMode, "decoration type mismatch");
		auto* new_node = allocate_node(memory, 1);
		new_node->deco      = deco;
		new_node->values[0] = static_cast<uint32_t>(fp_fast_math_mode);
		m_mask.set(deco_index(deco));
	}

	VERA_INLINE void set(spv::Decoration deco, std::pmr::memory_resource* memory, std::string_view name, spv::LinkageType linkage_type) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationLinkageAttributes, "decoration type mismatch");
		auto*    new_node = allocate_node(memory, 1 + (name.length() + 4) / 4);
		new_node->deco      = deco;
		new_node->values[0] = static_cast<uint32_t>(linkage_type);
		memcpy(&new_node->values[1], name.data(), name.length() * 4);
		m_mask.set(deco_index(deco));
	}

	VERA_INLINE void set(spv::Decoration deco, std::pmr::memory_resource* memory, std::string_view name) VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationUserSemantic, "decoration type mismatch");
		auto*    new_node = allocate_node(memory, (name.length() + 4) / 4);
		new_node->deco      = deco;
		memcpy(&new_node->values[0], name.data(), name.length() * 4);
		m_mask.set(deco_index(deco));
	}

	VERA_INLINE bool has(spv::Decoration deco) const VERA_NOEXCEPT
	{
		return m_mask.test(deco_index(deco));
	}

	template <class T>
	VERA_NODISCARD VERA_INLINE T get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		static_assert("unsupported decoration value type");
	}

	template <>
	VERA_NODISCARD VERA_INLINE uint32_t get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(has(deco), "decoration value is not set");

		switch (deco) {
		case spv::DecorationSpecId:        return m_spec_id;
		case spv::DecorationArrayStride:   return m_array_stride;
		case spv::DecorationMatrixStride:  return m_matrix_stride;
		case spv::DecorationLocation:      return m_location;
		case spv::DecorationIndex:         return m_index;
		case spv::DecorationBinding:       return m_binding;
		case spv::DecorationDescriptorSet: return m_desc_set;
		case spv::DecorationOffset:        return m_offset;
		case spv::DecorationAlignment:     return m_alignment;

		case spv::DecorationStream:
		case spv::DecorationComponent:
		case spv::DecorationXfbBuffer:
		case spv::DecorationXfbStride:
		case spv::DecorationInputAttachmentIndex:
		case spv::DecorationMaxByteOffset:
		case spv::DecorationSecondaryViewportRelativeNV:
			return find_node(deco)->values[0];

			// ID type decorations
		case spv::DecorationUniformId:
		case spv::DecorationAlignmentId:
		case spv::DecorationMaxByteOffsetId:
		case spv::DecorationNodeSharesPayloadLimitsWithAMDX:
		case spv::DecorationNodeMaxPayloadsAMDX:
		case spv::DecorationPayloadNodeNameAMDX:
		case spv::DecorationPayloadNodeBaseIndexAMDX:
		case spv::DecorationPayloadNodeArraySizeAMDX:
		case spv::DecorationCounterBuffer:
			return find_node(deco)->values[0];
		}

		VERA_ERROR_MSG("unsupported decoration value");
	}

	template <>
	VERA_NODISCARD VERA_INLINE spv::BuiltIn get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationBuiltIn, "decoration value is not of type BuiltIn");
		VERA_ASSERT_MSG(has(spv::DecorationBuiltIn), "decoration value is not set");
		return m_builtin;
	}

	template <>
	VERA_NODISCARD VERA_INLINE spv::FunctionParameterAttribute get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationFuncParamAttr, "decoration value is not of type FunctionParameterAttribute");
		VERA_ASSERT_MSG(has(spv::DecorationFuncParamAttr), "decoration value is not set");

		// find out more
		return static_cast<spv::FunctionParameterAttribute>(find_node(deco)->values[0]);
	}

	template <>
	VERA_NODISCARD VERA_INLINE spv::FPRoundingMode get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationFPRoundingMode, "decoration value is not of type FPRoundingMode");
		VERA_ASSERT_MSG(has(spv::DecorationFPRoundingMode), "decoration value is not set");
		return static_cast<spv::FPRoundingMode>(find_node(deco)->values[0]);
	}

	template <>
	VERA_NODISCARD VERA_INLINE spv::FPFastMathModeMask get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationFPFastMathMode, "decoration value is not of type FPFastMathModeMask");
		VERA_ASSERT_MSG(has(spv::DecorationFPFastMathMode), "decoration value is not set");
		return static_cast<spv::FPFastMathModeMask>(find_node(deco)->values[0]);
	}

	template <>
	VERA_NODISCARD VERA_INLINE std::string_view get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(has(deco), "decoration value is not set");
		
		auto* node = find_node(deco);
		
		switch (deco) {
		case spv::DecorationLinkageAttributes:
			return reinterpret_cast<const char*>(&node->values[1]);
		case spv::DecorationUserSemantic:
			return reinterpret_cast<const char*>(&node->values[0]);
		}

		VERA_ERROR_MSG("unsupported decoration value");
	}

	template <>
	VERA_NODISCARD VERA_INLINE spv::LinkageType get_value(spv::Decoration deco) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(deco == spv::DecorationLinkageAttributes, "decoration value is not of type LinkageType");
		VERA_ASSERT_MSG(has(spv::DecorationLinkageAttributes), "decoration value is not set");
		return static_cast<spv::LinkageType>(find_node(deco)->values[0]);
	}

private:
	VERA_INLINE size_t deco_index(spv::Decoration deco) const VERA_NOEXCEPT
	{
		if (deco < 48)
			return static_cast<size_t>(deco);

		switch (deco) {
		case spv::DecorationNoSignedWrap:                    return 48;
		case spv::DecorationNoUnsignedWrap:                  return 49;
		case spv::DecorationWeightTextureQCOM:               return 50;
		case spv::DecorationBlockMatchTextureQCOM:           return 51;
		case spv::DecorationBlockMatchSamplerQCOM:           return 52;
		case spv::DecorationExplicitInterpAMD:               return 53;
		case spv::DecorationNodeSharesPayloadLimitsWithAMDX: return 54;
		case spv::DecorationNodeMaxPayloadsAMDX:             return 55;
		case spv::DecorationTrackFinishWritingAMDX:          return 56;
		case spv::DecorationPayloadNodeNameAMDX:             return 57;
		case spv::DecorationPayloadNodeBaseIndexAMDX:        return 58;
		case spv::DecorationPayloadNodeSparseArrayAMDX:      return 59;
		case spv::DecorationPayloadNodeArraySizeAMDX:        return 60;
		case spv::DecorationPayloadDispatchIndirectAMDX:     return 61;
		case spv::DecorationOverrideCoverageNV:              return 62;
		case spv::DecorationPassthroughNV:                   return 63;
		case spv::DecorationViewportRelativeNV:              return 64;
		case spv::DecorationSecondaryViewportRelativeNV:     return 65;
		case spv::DecorationPerPrimitiveEXT:                 return 66;
		case spv::DecorationPerViewNV:                       return 67;
		case spv::DecorationPerTaskNV:                       return 68;
		case spv::DecorationPerVertexKHR:                    return 69;
		case spv::DecorationNonUniform:                      return 70;
		case spv::DecorationRestrictPointer:                 return 71;
		case spv::DecorationAliasedPointer:                  return 72;
		case spv::DecorationHitObjectShaderRecordBufferNV:   return 73;
		case spv::DecorationBindlessSamplerNV:               return 74;
		case spv::DecorationBindlessImageNV:                 return 75;
		case spv::DecorationBoundSamplerNV:                  return 76;
		case spv::DecorationBoundImageNV:                    return 77;
		}

		VERA_ERROR_MSG("unsupported decoration value");
	}

	VERA_INLINE deco_node* allocate_node(std::pmr::memory_resource* memory, size_t value_count)
	{
		auto* ptr = memory->allocate(sizeof(deco_node) + sizeof(uint32_t) * value_count, alignof(deco_node));
		auto* node = new (ptr) deco_node();
		node->next = m_head;

		m_head = node;

		return node;
	}

	VERA_INLINE const deco_node* find_node(spv::Decoration deco) const VERA_NOEXCEPT
	{
		for (auto* node = m_head; node != nullptr; node = node->next)
			if (node->deco == deco)
				return node;

		return nullptr;
	}

	std::bitset<128> m_mask;
	const deco_node* m_head;
	uint32_t         m_spec_id;
	uint32_t         m_array_stride;
	uint32_t         m_matrix_stride;
	spv::BuiltIn     m_builtin;
	uint32_t         m_location;
	uint32_t         m_index;
	uint32_t         m_binding;
	uint32_t         m_desc_set;
	uint32_t         m_offset;
	uint32_t         m_alignment;
};

class SpvNodeMeta
{
public:
	std::string_view name;
	spv_decoration   decoration;
};

class SpvNode
{
public:
	spv::Op      op;
	spv::Id      id;
	SpvNodeMeta* meta;

	template <class T>
	VERA_NODISCARD VERA_INLINE const T* as() const VERA_NOEXCEPT
	{
		static_assert(std::is_base_of<SpvNode, T>::value, "T must be derived from SpvNode");
		return static_cast<const T*>(this);
	}

	template <class T>
	VERA_NODISCARD VERA_INLINE T* as() VERA_NOEXCEPT
	{
		static_assert(std::is_base_of<SpvNode, T>::value, "T must be derived from SpvNode");
		return static_cast<T*>(this);
	}
};

class SpvExtInstNode : public SpvNode
{
public:
	std::string_view extInstName;
};

class SpvEntryPointNode : public SpvNode
{
public:
	spv::ExecutionModel	executionModel;
	std::string_view    entryPointName;
	array_view<spv::Id> interfaceIds;
};

class SpvStringNode : public SpvNode
{
public:
	std::string_view string;
};

class SpvTypeNode : public SpvNode
{
public:

};

class SpvBasicTypeNode : public SpvTypeNode
{
public:
	struct IntegerTraits
	{
		uint32_t width;
		bool     signedness;
	};

	struct FloatTraits
	{
		uint32_t        width;
		spv::FPEncoding encoding;
	};

	struct MatrixTraits
	{
		uint32_t columnCount;
	};

	SpvBasicType basicType;
};

class SpvArrayTypeNode : public SpvTypeNode
{
public:
	spv::Id    elementTypeId;
	spv::Id    lengthId;
	uint32_t   lengthValue;
};

class SpvStructTypeMember
{
public:
	spv::Id          id;
	std::string_view name;
	spv_decoration   decoration;
};

class SpvStructTypeNode : public SpvTypeNode
{
public:
	std::pmr::vector<SpvStructTypeMember> members;
};

class SpvImageTypeNode : public SpvTypeNode
{
public:
	spv::Id              elementTypeId;
	spv::Dim             dim;
	uint32_t             depth;
	bool                 arrayed;
	bool                 multisampled;
	uint32_t             sampled;
	spv::ImageFormat     imageFormat;
	spv::AccessQualifier accessQualifier;
};

class SpvVariableNode : public SpvNode
{
public:
};

class SpvParser
{
public:
	SpvParser();
	// takes ownership of spirv_code
	SpvParser(std::vector<uint32_t>&& spirv_code);
	// does not take ownership of spirv_code
	SpvParser(array_view<uint32_t> spirv_code);
	~SpvParser();

	std::pmr::monotonic_buffer_resource memory;
	std::pmr::monotonic_buffer_resource tempMemory;

	std::vector<uint32_t>               spirvStorage;
	array_view<uint32_t>                spirvCode;

	Version                             version;
	uint32_t                            generatorMagic;
	std::vector<spv::Capability>        capabilities;
	std::vector<std::string_view>       extensionNames;
	spv::MemoryModel                    memoryModel;
	std::string                         source;
	std::vector<SpvNode*>               nodes;
	std::vector<SpvTypeNode*>           typeNodes;

private:
	template <class T>
	VERA_NODISCARD VERA_INLINE T* alloc(size_t count = 1)
	{
		auto* ptr = reinterpret_cast<T*>(memory.allocate(sizeof(T) * count, alignof(T)));
		std::uninitialized_default_construct_n(ptr, count);
		return ptr;
	}

	template <class T>
	VERA_NODISCARD VERA_INLINE T* alloc_temp(size_t count = 1)
	{
		auto* ptr = reinterpret_cast<T*>(tempMemory.allocate(sizeof(T) * count, alignof(T)));
		std::uninitialized_default_construct_n(ptr, count);
		return ptr;
	}

	VERA_NODISCARD VERA_INLINE std::string_view alloc_string(std::string_view str)
	{
		auto* ptr = alloc<char>(str.size() + 1);
		std::memcpy(ptr, str.data(), str.size());
		ptr[str.size()] = '\0';
		return std::string_view(ptr, str.size());
	}

	void parse();
	void parseNodes(spv_inst first, spv_inst last);
	void parseDebugSection(spv_inst first, spv_inst last);
	void parseAnnotationSection(spv_inst first, spv_inst last);

	void setNode(SpvNode* new_node);
	void setTypeNode(SpvTypeNode* new_node);

	template <class T>
	VERA_NODISCARD VERA_INLINE const T* findNode(spv::Id id) const VERA_NOEXCEPT
	{
		return static_cast<const T*>(nodes[id]);
	}

	template <class T>
	VERA_NODISCARD VERA_INLINE T* findNode(spv::Id id) VERA_NOEXCEPT
	{
		return static_cast<const T*>(nodes[id]);
	}

	void setDecoration(spv_inst inst);
	void parseIntType(SpvBasicTypeNode* node, spv_inst inst);
	void parseFloatType(SpvBasicTypeNode* node, spv_inst inst);
	void parseVectorType(SpvBasicTypeNode* node, spv_inst inst);
	void parseMatrixType(SpvBasicTypeNode* node, spv_inst inst);
	void parseImageType(SpvImageTypeNode* node, spv_inst inst);
};

VERA_NAMESPACE_END
