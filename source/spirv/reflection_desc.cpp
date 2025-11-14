#include "reflection_desc.h"

#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/logger.h"
#include "../../include/vera/util/static_vector.h"
#include <spirv_reflect.h>

#define MAX_SHADER_STAGE_COUNT 16
#define INITIAL_MONOTONIC_CHUNK_SIZE VERA_KIB(2)

#define VALIDATE_NODE_LAYOUT(node_type, member, prop) offsetof(node_type, member) == reflection_property_offset_v<prop>
#define SET_BINDING_FMT(desc_node) " set={}, binding={}", (desc_node).set, (desc_node).binding
#define COMBINE_SET_BINDING(set, binding) ((static_cast<uint64_t>(set) << 32) | static_cast<uint64_t>(binding))

VERA_NAMESPACE_BEGIN

// compile time node layout validation
struct NodeLayoutValidation
{
	using enum ReflectionPropertyFlagBits;

	static_assert(sizeof(nullptr) == 8); // assuming 64-bit pointers
	static_assert(sizeof(array_view<int>) == 16);

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionRootNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionRootNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionRootNode, memberNodes, MemberNodes));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionRootNode, nameMap, NameMap));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, nameList, NameList));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, descriptorType, DescriptorType));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, set, Set));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, binding, Binding));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, nameList, NameList));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, descriptorType, DescriptorType));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, set, Set));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, binding, Binding));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, elementNode, ElementNode));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, elementCount, ElementCount));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, stride, Stride));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, nameList, NameList));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, descriptorType, DescriptorType));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, set, Set));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, binding, Binding));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, memberNodes, MemberNodes));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, nameMap, NameMap));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, nameList, NameList));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, memberNodes, MemberNodes));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, nameMap, NameMap));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, nameList, NameList));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, offset, Offset));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, paddedSize, PaddedSize));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, memberNodes, MemberNodes));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, nameMap, NameMap));
	
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, nameList, NameList));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, offset, Offset));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, paddedSize, PaddedSize));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, elementNode, ElementNode));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, elementCount, ElementCount));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, stride, Stride));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, nameList, NameList));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, offset, Offset));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, paddedSize, PaddedSize));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, primitiveType, PrimitiveType));
};

typedef const ReflectionRootMemberNode* ReflectionRootMemberNodePtr;
typedef const ReflectionBlockMemberNode* ReflectionBlockMemberNodePtr;
typedef const ReflectionBlockMemberNode* ReflectionBlockNodePtr;
typedef const ReflectionDescriptorNode* ReflectionDescriptorNodePtr;
typedef const ReflectionPushConstantNode* ReflectionPushConstantNodePtr;

typedef static_vector<const ReflectionRootMemberNodePtr*, MAX_SHADER_STAGE_COUNT> PerStageRootMemberNodeIteratorArray;
typedef static_vector<const ReflectionBlockMemberNodePtr*, MAX_SHADER_STAGE_COUNT> PerStageBlockMemberNodeIteratorArray;
typedef static_vector<const ReflectionDescriptorNodePtr*, MAX_SHADER_STAGE_COUNT> PerStageDescriptorNodeIteratorArray;
typedef static_vector<const ReflectionRootNode*, MAX_SHADER_STAGE_COUNT>  PerStageRootNodeArray;
typedef static_vector<const ReflectionRootMemberNode*, MAX_SHADER_STAGE_COUNT>  PerStageRootMemberNodeArray;
typedef static_vector<const ReflectionBlockMemberNode*, MAX_SHADER_STAGE_COUNT> PerStageBlockMemberNodeArray;
typedef static_vector<const ReflectionDescriptorNode*, MAX_SHADER_STAGE_COUNT> PerStageDescriptorNodeArray;
typedef static_vector<const ReflectionPushConstantNode*, MAX_SHADER_STAGE_COUNT>  PerStagePushConstantNodeArray;

typedef std::pmr::polymorphic_allocator<const ReflectionRootMemberNode*> ReflectionRootMemberAllocator;

struct ReflectionContext
{
	std::pmr::memory_resource* memory;
	std::pmr::memory_resource* tempMemory;
	ShaderStageFlags           stageFlags;
};

static const char* get_spv_reflect_result_string(SpvReflectResult result)
{
	switch (result) {
	case SPV_REFLECT_RESULT_SUCCESS:
		return "SPV_REFLECT_RESULT_SUCCESS";
	case SPV_REFLECT_RESULT_NOT_READY:
		return "SPV_REFLECT_RESULT_NOT_READY";
	case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED:
		return "SPV_REFLECT_RESULT_ERROR_PARSE_FAILED";
	case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED:
		return "SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED";
	case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED:
		return "SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED";
	case SPV_REFLECT_RESULT_ERROR_NULL_POINTER:
		return "SPV_REFLECT_RESULT_ERROR_NULL_POINTER";
	case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR:
		return "SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR";
	case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH:
		return "SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH";
	case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND:
		return "SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE";
	case SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED:
		return "SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED";
	}

	VERA_ERROR_MSG("invalid spirv reflect result");
}

static const char* get_shader_stage_string(ShaderStageFlagBits stage)
{
	switch (stage) {
	case ShaderStageFlagBits::Vertex:                 return "Vertex";
	case ShaderStageFlagBits::TessellationControl:    return "TessellationControl";
	case ShaderStageFlagBits::TessellationEvaluation: return "TessellationEvaluation";
	case ShaderStageFlagBits::Geometry:               return "Geometry";
	case ShaderStageFlagBits::Fragment:               return "Fragment";
	case ShaderStageFlagBits::Compute:                return "Compute";
	case ShaderStageFlagBits::Task:                   return "Task";
	case ShaderStageFlagBits::Mesh:                   return "Mesh";
	case ShaderStageFlagBits::RayGen:                 return "RayGen";
	case ShaderStageFlagBits::AnyHit:                 return "AnyHit";
	case ShaderStageFlagBits::ClosestHit:             return "ClosestHit";
	case ShaderStageFlagBits::Miss:                   return "Miss";
	case ShaderStageFlagBits::Intersection:           return "Intersection";
	case ShaderStageFlagBits::Callable:               return "Callable";
	}

	VERA_ERROR_MSG("invalid shader stage");
}

static DescriptorType to_descriptor_type(SpvReflectDescriptorType type)
{
	switch (type) {
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
		return DescriptorType::Sampler;
	case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		return DescriptorType::CombinedTextureSampler;
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		return DescriptorType::SampledTexture;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		return DescriptorType::StorageTexture;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		return DescriptorType::UniformTexelBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		return DescriptorType::StorageTexelBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		return DescriptorType::UniformBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		return DescriptorType::StorageBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		return DescriptorType::UniformBufferDynamic;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return DescriptorType::StorageBufferDynamic;
	case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		return DescriptorType::InputAttachment;
	}

	VERA_ERROR_MSG("invalid resource type");
}

static ShaderStageFlagBits to_shader_stage(SpvReflectShaderStageFlagBits stage)
{
	switch (stage) {
	case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
		return ShaderStageFlagBits::Vertex;
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return ShaderStageFlagBits::TessellationControl;
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return ShaderStageFlagBits::TessellationEvaluation;
	case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
		return ShaderStageFlagBits::Geometry;
	case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
		return ShaderStageFlagBits::Fragment;
	case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
		return ShaderStageFlagBits::Compute;
	case SPV_REFLECT_SHADER_STAGE_TASK_BIT_EXT:
		return ShaderStageFlagBits::Task;
	case SPV_REFLECT_SHADER_STAGE_MESH_BIT_EXT:
		return ShaderStageFlagBits::Mesh;
	case SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR:
		return ShaderStageFlagBits::RayGen;
	case SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR:
		return ShaderStageFlagBits::AnyHit;
	case SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
		return ShaderStageFlagBits::ClosestHit;
	case SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR:
		return ShaderStageFlagBits::Miss;
	case SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR:
		return ShaderStageFlagBits::Intersection;
	case SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR:
		return ShaderStageFlagBits::Callable;
	}

	VERA_ERROR_MSG("invalid shader stage");
}

static bool is_unsized_array(const SpvReflectBindingArrayTraits& traits)
{
	return 0 < traits.dims_count && traits.dims[traits.dims_count - 1] <= 1;
}

static bool is_unsized_array(const SpvReflectArrayTraits& traits)
{
	return 0 < traits.dims_count && traits.dims[traits.dims_count - 1] <= 1;
}

static bool is_block_variable(const SpvReflectDescriptorBinding& binding)
{
	return binding.block.name && !(binding.block.members && !binding.block.members->name);
}

static bool is_pc_range_intersect(const PushConstantRange& lhs, const PushConstantRange& rhs)
{
	uint32_t lhs_end = lhs.offset + lhs.size;
	uint32_t rhs_end = rhs.offset + rhs.size;
	return !(lhs_end <= rhs.offset || rhs_end <= lhs.offset);
}

static uint32_t get_array_stride(const SpvReflectBindingArrayTraits& traits, uint32_t dim)
{
	uint32_t result = 1;

	for (uint32_t i = dim + 1; i < traits.dims_count; ++i)
		result *= traits.dims[i];

	return result;
}

static uint32_t get_array_stride(const SpvReflectArrayTraits& traits, uint32_t dim)
{
	uint32_t result = traits.stride;

	for (uint32_t i = dim + 1; i < traits.dims_count; ++i)
		result *= traits.dims[i];

	return result;
}

static PushConstantRange get_pc_range(const SpvReflectBlockVariable& block, ShaderStageFlags stage_flags)
{
	return PushConstantRange{
		.offset     = block.offset,
		.size       = block.padded_size,
		.stageFlags = stage_flags
	};
}

static ReflectionPrimitiveType reflect_matrix_type(const SpvReflectTypeDescription& desc)
{
	switch (desc.traits.numeric.scalar.width) {
	case 32:
		switch (desc.traits.numeric.matrix.row_count) {
		case 2:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Float2x2;
			case 3: return ReflectionPrimitiveType::Float2x3;
			case 4: return ReflectionPrimitiveType::Float2x4;
			}
			break;
		case 3:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Float3x2;
			case 3: return ReflectionPrimitiveType::Float3x3;
			case 4: return ReflectionPrimitiveType::Float3x4;
			}
			break;
		case 4:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Float4x2;
			case 3: return ReflectionPrimitiveType::Float4x3;
			case 4: return ReflectionPrimitiveType::Float4x4;
			}
			break;
		};
		break;
	case 64:
		switch (desc.traits.numeric.matrix.row_count) {
		case 2:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Double2x2;
			case 3: return ReflectionPrimitiveType::Double2x3;
			case 4: return ReflectionPrimitiveType::Double2x4;
			}
			break;
		case 3:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Double3x2;
			case 3: return ReflectionPrimitiveType::Double3x3;
			case 4: return ReflectionPrimitiveType::Double3x4;
			}
			break;
		case 4:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Double4x2;
			case 3: return ReflectionPrimitiveType::Double4x3;
			case 4: return ReflectionPrimitiveType::Double4x4;
			}
			break;
		};
		break;
	}

	throw Exception("failed to reflect matrix type");
}

static ReflectionPrimitiveType reflect_vector_type(const SpvReflectTypeDescription& desc)
{
	if (desc.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT) {
		switch (desc.traits.numeric.scalar.width) {
		case 32:
			switch (desc.traits.numeric.vector.component_count) {
			case 2: return ReflectionPrimitiveType::Float2;
			case 3: return ReflectionPrimitiveType::Float3;
			case 4: return ReflectionPrimitiveType::Float4;
			}
			break;
		case 64:
			switch (desc.traits.numeric.vector.component_count) {
			case 2: return ReflectionPrimitiveType::Double2;
			case 3: return ReflectionPrimitiveType::Double3;
			case 4: return ReflectionPrimitiveType::Double4;
			}
			break;
		}
	} else if (desc.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT) {
		if (desc.traits.numeric.scalar.signedness) {
			switch (desc.traits.numeric.scalar.width) {
			case 8:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Char2;
				case 3: return ReflectionPrimitiveType::Char3;
				case 4: return ReflectionPrimitiveType::Char4;
				}
				break;
			case 16:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Short2;
				case 3: return ReflectionPrimitiveType::Short3;
				case 4: return ReflectionPrimitiveType::Short4;
				}
				break;
			case 32:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Int2;
				case 3: return ReflectionPrimitiveType::Int3;
				case 4: return ReflectionPrimitiveType::Int4;
				}
				break;
			case 64:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Long2;
				case 3: return ReflectionPrimitiveType::Long3;
				case 4: return ReflectionPrimitiveType::Long4;
				}
				break;
			}
		} else {
			switch (desc.traits.numeric.scalar.width) {
			case 8:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UChar2;
				case 3: return ReflectionPrimitiveType::UChar3;
				case 4: return ReflectionPrimitiveType::UChar4;
				}
				break;
			case 16:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UShort2;
				case 3: return ReflectionPrimitiveType::UShort3;
				case 4: return ReflectionPrimitiveType::UShort4;
				}
				break;
			case 32:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UInt2;
				case 3: return ReflectionPrimitiveType::UInt3;
				case 4: return ReflectionPrimitiveType::UInt4;
				}
				break;
			case 64:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::ULong2;
				case 3: return ReflectionPrimitiveType::ULong3;
				case 4: return ReflectionPrimitiveType::ULong4;
				}
				break;
			}
		}
	} else if (desc.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_BOOL) {
		switch (desc.traits.numeric.scalar.width) {
		case 2: return ReflectionPrimitiveType::Bool2;
		case 3: return ReflectionPrimitiveType::Bool3;
		case 4: return ReflectionPrimitiveType::Bool4;
		}
	}

	throw Exception("failed to reflect vector type");
}

static ReflectionPrimitiveType reflect_float_type(const SpvReflectTypeDescription& desc)
{
	switch (desc.traits.numeric.scalar.width) {
	case 32: return ReflectionPrimitiveType::Float;
	case 64: return ReflectionPrimitiveType::Double;
	}

	throw Exception("failed to reflect float type");
}

static ReflectionPrimitiveType reflect_int_type(const SpvReflectTypeDescription& desc)
{
	if (desc.traits.numeric.scalar.signedness) {
		switch (desc.traits.numeric.scalar.width) {
		case 8:  return ReflectionPrimitiveType::Char;
		case 16: return ReflectionPrimitiveType::Short;
		case 32: return ReflectionPrimitiveType::Int;
		case 64: return ReflectionPrimitiveType::Long;
		}
	} else {
		switch (desc.traits.numeric.scalar.width) {
		case 8:  return ReflectionPrimitiveType::UChar;
		case 16: return ReflectionPrimitiveType::UShort;
		case 32: return ReflectionPrimitiveType::UInt;
		case 64: return ReflectionPrimitiveType::ULong;
		}
	}

	throw Exception("failed to reflect int type");
}

static ReflectionPrimitiveType reflect_primitive_type(const SpvReflectTypeDescription& desc)
{
	if (desc.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX)
		return reflect_matrix_type(desc);
	else if (desc.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
		return reflect_vector_type(desc);
	else if (desc.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
		return reflect_float_type(desc);
	else if (desc.type_flags & SPV_REFLECT_TYPE_FLAG_INT)
		return reflect_int_type(desc);
	else if (desc.type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
		return ReflectionPrimitiveType::Bool;

	throw Exception("invalid type flags");
}

static ReflectionNameChain* construct_name_chain(
	std::pmr::memory_resource* memory,
	const char*                str
) {
	size_t length = std::strlen(str);
	size_t size   = sizeof(ReflectionNameChain) + length + 1;

	void* result    = memory->allocate(size, alignof(ReflectionNameChain));
	auto* new_chain = std::construct_at(reinterpret_cast<ReflectionNameChain*>(result));
	new_chain->next = nullptr;
	memcpy(new_chain->name, str, length + 1);
	
	return new_chain;
}

static ReflectionNameChain* intern_name_chain(
	std::pmr::memory_resource* memory,
	const ReflectionNameChain* head,
	const char*                str
) {	
	for (const auto* curr = head; curr; curr = curr->next)
		if (std::strcmp(curr->name, str) == 0)
			return const_cast<ReflectionNameChain*>(curr);

	auto* new_chain = construct_name_chain(memory, str);
	new_chain->next = head;

	return new_chain;
}

template <class T>
static T* construct_node(std::pmr::memory_resource* memory)
{
	void* result = memory->allocate(sizeof(T), alignof(T));
	return std::construct_at(reinterpret_cast<T*>(result));
}

template <class T>
static T* construct_array(std::pmr::memory_resource* memory, size_t count)
{
	void* result = memory->allocate(count * sizeof(T), alignof(T));
	std::uninitialized_default_construct_n(reinterpret_cast<T*>(result), count);
	return reinterpret_cast<T*>(result);
}

static char* construct_string(std::pmr::memory_resource* memory, const char* str)
{
	size_t length = std::strlen(str);
	void*  result = memory->allocate(length + 1, alignof(char));
	std::memcpy(result, str, length + 1);
	return reinterpret_cast<char*>(result);
}

static const ReflectionBlockMemberNode* parse_block_variable(
	ReflectionContext&             ctx,
	const SpvReflectBlockVariable& block,
	const uint32_t                 array_dim = 0
) {
	if (block.array.dims_count != array_dim) {
		auto* array_node = construct_node<ReflectionArrayNode>(ctx.memory);
		array_node->type         = ReflectionType::Array;
		array_node->stageFlags   = ctx.stageFlags;
		array_node->nameList     = construct_name_chain(ctx.memory, block.name);
		array_node->offset       = block.offset;
		array_node->paddedSize   = UINT32_MAX;
		array_node->stride       = get_array_stride(block.array, array_dim);
		array_node->elementCount = block.array.dims[array_dim] == 1 ? UINT32_MAX : block.array.dims[array_dim];
		array_node->elementNode  = parse_block_variable(ctx, block, array_dim + 1);

		if (array_node->elementCount != UINT32_MAX)
			array_node->paddedSize = array_node->stride * array_node->elementCount;

		return array_node->as<ReflectionBlockMemberNode>();
	}

	if (block.member_count) {
		auto* struct_node = construct_node<ReflectionStructNode>(ctx.memory);
		struct_node->type       = ReflectionType::Struct;
		struct_node->stageFlags = ctx.stageFlags;
		struct_node->nameList   = construct_name_chain(ctx.memory, block.name);
		struct_node->offset     = block.offset;
		struct_node->paddedSize = block.padded_size;

		ReflectionNameMap name_map(ctx.tempMemory);

		uint32_t     member_count = block.member_count;
		uint32_t     member_idx   = 0;
		const auto** member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, member_count);
		const auto&  last_member  = block.members[member_count - 1];

		for (const auto& member : array_view(block.members, member_count)) {
			auto* new_node = parse_block_variable(ctx, member);

			member_nodes[member_idx++] = new_node;
			name_map.insert({
				new_node->getName(),
				ReflectionNodeMapping{ .node = new_node, .stageFlags = ctx.stageFlags }
			});
		}

		// assign name map with constructed name map
		struct_node->nameMap     = ReflectionNameMap(name_map, ctx.memory);
		struct_node->memberNodes = array_view{ member_nodes, member_count };

		return struct_node->as<ReflectionBlockMemberNode>();
	}
	
	auto* prim_node = construct_node<ReflectionPrimitiveNode>(ctx.memory);
	prim_node->type          = ReflectionType::Primitive;
	prim_node->stageFlags    = ctx.stageFlags;
	prim_node->nameList      = construct_name_chain(ctx.memory, block.name);
	prim_node->offset        = block.offset;
	prim_node->paddedSize    = block.padded_size;
	prim_node->primitiveType = reflect_primitive_type(*block.type_description);

	return prim_node->as<ReflectionBlockMemberNode>();
}

const ReflectionPushConstantNode* parse_push_constant(
	ReflectionContext&             ctx,
	const SpvReflectBlockVariable& block
) {
	auto* pc_node = construct_node<ReflectionPushConstantNode>(ctx.memory);
	pc_node->type              = ReflectionType::PushConstant;
	pc_node->stageFlags        = ctx.stageFlags;
	pc_node->nameList          = construct_name_chain(ctx.memory, block.name);
	pc_node->pushConstantRange = get_pc_range(block, ctx.stageFlags);

	ReflectionNameMap name_map(ctx.tempMemory);

	uint32_t     member_count = block.member_count;
	uint32_t     member_idx   = 0;
	const auto** member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, member_count);
	const auto&  last_member  = block.members[member_count - 1];

	for (const auto& member : array_view(block.members, member_count)) {
		if (&member != &last_member && is_unsized_array(block.array))
			throw Exception("only last member of push constant can be unsized array");

		auto* new_node = parse_block_variable(ctx, member);

		member_nodes[member_idx++] = new_node;
		name_map.insert({
			new_node->getName(),
			ReflectionNodeMapping{ .node = new_node, .stageFlags = ctx.stageFlags }
		});
	}

	// assign name map with constructed name map
	pc_node->nameMap     = ReflectionNameMap(name_map, ctx.memory);
	pc_node->memberNodes = array_view{ member_nodes, member_count };

	return pc_node;
}

static const ReflectionDescriptorNode* parse_descriptor_binding(
	ReflectionContext&                 ctx,
	const SpvReflectDescriptorBinding& binding,
	const uint32_t                     array_dim = 0
) {
	if (array_dim < binding.array.dims_count) {
		if (array_dim > 1 && is_unsized_array(binding.array))
			throw Exception("multiple dimension unsized array on descriptor binding is not supported at "
				SET_BINDING_FMT(binding));

		auto* array_node = construct_node<ReflectionDescriptorArrayNode>(ctx.memory);
		array_node->type           = ReflectionType::DescriptorArray;
		array_node->stageFlags     = ctx.stageFlags;
		array_node->nameList       = construct_name_chain(ctx.memory, binding.name);
		array_node->descriptorType = to_descriptor_type(binding.descriptor_type);
		array_node->set            = binding.set;
		array_node->binding        = binding.binding;
		array_node->stride         = get_array_stride(binding.array, array_dim);
		array_node->elementCount   = binding.array.dims[array_dim] <= 1 ? UINT32_MAX : binding.array.dims[array_dim];
		array_node->elementNode    = parse_descriptor_binding(ctx, binding, array_dim + 1);

		return array_node->as<ReflectionDescriptorNode>();
	}

	if (is_block_variable(binding)) {
		auto* block_node = construct_node<ReflectionDescriptorBlockNode>(ctx.memory);
		block_node->type           = ReflectionType::DescriptorBlock;
		block_node->stageFlags     = ctx.stageFlags;
		block_node->nameList       = construct_name_chain(ctx.memory, binding.name);
		block_node->descriptorType = to_descriptor_type(binding.descriptor_type);
		block_node->set            = binding.set;
		block_node->binding        = binding.binding;

		ReflectionNameMap name_map(ctx.tempMemory);

		uint32_t     member_count = binding.block.member_count;
		uint32_t     member_idx   = 0;
		const auto** member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, member_count);
		const auto&  last_member  = binding.block.members[member_count - 1];

		for (const auto& member : array_view(binding.block.members, member_count)) {
			if (&member != &last_member && is_unsized_array(binding.block.array))
				throw Exception("only last member of block can be unsized array at" SET_BINDING_FMT(binding));

			auto* new_node = parse_block_variable(ctx, member);

			member_nodes[member_idx++] = new_node;
			name_map.insert({
				new_node->getName(),
				ReflectionNodeMapping{ .node = new_node, .stageFlags = ctx.stageFlags }
			});
		}

		// assign name map with constructed name map
		block_node->nameMap     = ReflectionNameMap(name_map, ctx.memory);
		block_node->memberNodes = array_view{ member_nodes, member_count };

		return block_node->as<ReflectionDescriptorNode>();
	}

	auto* desc_node = construct_node<ReflectionDescriptorNode>(ctx.memory);
	desc_node->type           = ReflectionType::Descriptor;
	desc_node->stageFlags     = ctx.stageFlags;
	desc_node->nameList       = construct_name_chain(ctx.memory, binding.name);
	desc_node->descriptorType = to_descriptor_type(binding.descriptor_type);
	desc_node->set            = binding.set;
	desc_node->binding        = binding.binding;

	return desc_node->as<ReflectionDescriptorNode>();
}

static const ReflectionRootNode* parse_impl(
	ReflectionContext&            ctx,
	const SpvReflectShaderModule& module
) {
	if (module.entry_point_count != 1)
		throw Exception("SPIR-V module must have exactly one entry point");
	if (1 < module.push_constant_block_count)
		throw Exception("SPIR-V module must have at most one push constant block");

	auto* entry_points = construct_array<ReflectionEntryPoint>(ctx.memory, 1);
	entry_points[0].stageFlags = to_shader_stage(module.shader_stage);
	entry_points[0].name       = construct_string(ctx.memory, module.entry_point_name);

	ReflectionNameMap    name_map(ctx.tempMemory);
	ReflectionBindingMap binding_map(ctx.tempMemory);

	uint32_t            desc_node_count   = module.descriptor_binding_count;
	uint32_t            pc_node_count     = module.push_constant_block_count;
	uint32_t            root_member_count = desc_node_count + pc_node_count;
	uint32_t            root_member_idx   = 0;
	const auto**        root_members      = construct_array<ReflectionRootMemberNodePtr>(ctx.memory, root_member_count);
	ReflectionSetRange* set_ranges        = nullptr;
	uint32_t            set_count         = 0;

	if (desc_node_count > 0) {
		set_count  = module.descriptor_bindings[desc_node_count - 1].set + 1;
		set_ranges = construct_array<ReflectionSetRange>(ctx.memory, set_count);
	}


	auto* root_node = construct_node<ReflectionRootNode>(ctx.memory);
	root_node->type              = ReflectionType::Root;
	root_node->stageFlags        = to_shader_stage(module.shader_stage);
	root_node->entryPoints       = array_view(entry_points, module.entry_point_count);
	root_node->minSet            = UINT32_MAX;
	root_node->maxSet            = 0;
	root_node->descriptorCount   = desc_node_count;
	root_node->pushConstantCount = pc_node_count;
	root_node->targetFlags       = ReflectionTargetFlagBits::Shader;

	for (const auto& set : array_view(module.descriptor_sets, module.descriptor_set_count)) {
		const auto* last_binding = set.bindings[set.binding_count - 1];
		uint32_t    desc_offset  = root_member_idx;

		for (const auto* binding : array_view(set.bindings, set.binding_count)) {
			if (binding != last_binding && is_unsized_array(binding->array))
					throw Exception("only last binding of descriptor set can be unsized array at "
						SET_BINDING_FMT(*binding));

			auto* new_node = parse_descriptor_binding(ctx, *binding);

			root_members[root_member_idx++] = new_node->as<ReflectionRootMemberNode>();
			name_map.insert({
				new_node->getName(),
				ReflectionNodeMapping{ new_node, ctx.stageFlags }
			});
			binding_map.insert({
				COMBINE_SET_BINDING(new_node->set, new_node->binding),
				new_node
			});
		}

		root_node->minSet = std::min(root_node->minSet, set.set);
		root_node->maxSet = std::max(root_node->maxSet, set.set);
		set_ranges[set.set] = ReflectionSetRange{
			reinterpret_cast<const ReflectionDescriptorNode* const*>(root_members + desc_offset),
			set.binding_count
		};
	}

	if (module.push_constant_blocks) {
		auto* pc_node = parse_push_constant(ctx, module.push_constant_blocks[0]);

		root_members[root_member_idx++] = pc_node->as<ReflectionRootMemberNode>();
		root_node->pushConstantCount = 1;
	}
	
	// assign name map with constructed name map
	root_node->nameMap     = ReflectionNameMap(name_map, ctx.memory);
	root_node->bindingMap  = ReflectionBindingMap(binding_map, ctx.memory);
	root_node->memberNodes = array_view{
		root_members,
		root_member_count
	};
	root_node->setRanges   = array_view{
		set_ranges,
		set_count
	};

	return root_node;
}

static bool check_node_compatible(const ReflectionNode* lhs, const ReflectionNode* rhs);

// member count can be different, but each member must be compatible
static bool check_block_compatible(
	const ReflectionRootMemberNode* lhs,
	const ReflectionRootMemberNode* rhs
) {
	if (lhs == rhs) return true;

	auto lhs_type = lhs->getType();
	auto rhs_type = rhs->getType();

	if (lhs_type != rhs_type) return false;

	VERA_ASSERT_MSG(
		lhs_type == ReflectionType::DescriptorBlock ||
		lhs_type == ReflectionType::PushConstant,
		"cannot check compatibility on non-block node");

	auto lhs_members = lhs->getBlockMemberNodes();
	auto rhs_members = rhs->getBlockMemberNodes();
	auto lhs_first   = lhs_members.begin();
	auto rhs_first   = rhs_members.begin();
	auto lhs_last    = lhs_members.end();
	auto rhs_last    = rhs_members.end();

	while (lhs_first != lhs_last && rhs_first != rhs_last) {
		auto lhs_offset = (*lhs_first)->getOffset();
		auto lhs_size   = (*lhs_first)->getPaddedSize();
		auto rhs_offset = (*rhs_first)->getOffset();
		auto rhs_size   = (*rhs_first)->getPaddedSize();

		if (lhs_offset == rhs_offset) {
			if (!check_node_compatible(*lhs_first, *rhs_first))
				return false;

			++lhs_first;
			++rhs_first;
		} else if (lhs_offset < rhs_offset) {
			if (rhs_offset < lhs_offset + lhs_size)
				return false;

			++lhs_first;
		} else {
			if (lhs_offset < rhs_offset + rhs_size)
				return false;

			++rhs_first;
		}
	}

	return true;
}

static bool check_node_compatible(const ReflectionNode* lhs, const ReflectionNode* rhs)
{
	if (lhs == rhs) return true;

	auto lhs_type = lhs->getType();
	auto rhs_type = rhs->getType();

	if (lhs_type != rhs_type) return false;

	VERA_ASSERT_MSG(lhs_type != ReflectionType::Root,
		"cannot check compatibility on root node");

	switch (lhs_type) {
	case ReflectionType::Descriptor: {
		auto lhs_desc = static_cast<const ReflectionDescriptorNode*>(lhs);
		auto rhs_desc = static_cast<const ReflectionDescriptorNode*>(rhs);

		return
			lhs_desc->descriptorType == rhs_desc->descriptorType &&
			lhs_desc->set == rhs_desc->set &&
			lhs_desc->binding == rhs_desc->binding;
	}
	case ReflectionType::DescriptorArray: {
		auto lhs_array = static_cast<const ReflectionDescriptorArrayNode*>(lhs);
		auto rhs_array = static_cast<const ReflectionDescriptorArrayNode*>(rhs);

		return
			lhs_array->descriptorType == rhs_array->descriptorType &&
			lhs_array->set == rhs_array->set &&
			lhs_array->binding == rhs_array->binding &&
			lhs_array->elementCount == rhs_array->elementCount &&
			lhs_array->stride == rhs_array->stride &&
			check_node_compatible(lhs_array->elementNode, rhs_array->elementNode);
	}
	case ReflectionType::DescriptorBlock: {
		auto lhs_block = static_cast<const ReflectionDescriptorBlockNode*>(lhs);
		auto rhs_block = static_cast<const ReflectionDescriptorBlockNode*>(rhs);

		return
			lhs_block->descriptorType == rhs_block->descriptorType &&
			lhs_block->set == rhs_block->set &&
			lhs_block->binding == rhs_block->binding &&
			check_block_compatible(lhs_block, rhs_block);
	}
	case ReflectionType::PushConstant: {
		auto lhs_pc = static_cast<const ReflectionPushConstantNode*>(lhs);
		auto rhs_pc = static_cast<const ReflectionPushConstantNode*>(rhs);

		return check_block_compatible(lhs_pc, rhs_pc);
	}
	case ReflectionType::Struct: {
		auto lhs_struct  = static_cast<const ReflectionStructNode*>(lhs);
		auto rhs_struct  = static_cast<const ReflectionStructNode*>(rhs);
		auto lhs_members = lhs_struct->memberNodes;
		auto rhs_members = rhs_struct->memberNodes;

		if (lhs_struct->offset != rhs_struct->offset ||
			lhs_struct->paddedSize != rhs_struct->paddedSize ||
			lhs_members.size() != rhs_members.size())
			return false;

		for (size_t i = 0; i < lhs_members.size(); ++i)
			if (!check_node_compatible(lhs_members[i], rhs_members[i]))
				return false;

		return true;
	}
	case ReflectionType::Array: {
		auto lhs_array = static_cast<const ReflectionArrayNode*>(lhs);
		auto rhs_array = static_cast<const ReflectionArrayNode*>(rhs);

		return
			lhs_array->offset == rhs_array->offset &&
			lhs_array->paddedSize == rhs_array->paddedSize &&
			lhs_array->elementCount == rhs_array->elementCount &&
			lhs_array->stride == rhs_array->stride &&
			check_node_compatible(lhs_array->elementNode, rhs_array->elementNode);
	}
	case ReflectionType::Primitive: {
		auto lhs_prim = static_cast<const ReflectionPrimitiveNode*>(lhs);
		auto rhs_prim = static_cast<const ReflectionPrimitiveNode*>(rhs);

		return
			lhs_prim->offset == rhs_prim->offset &&
			lhs_prim->paddedSize == rhs_prim->paddedSize &&
			lhs_prim->primitiveType == rhs_prim->primitiveType;
	}
	}
	
	VERA_ERROR_MSG("invalid reflection type");
}

static bool find_minimum_descriptor_node(
	PerStageDescriptorNodeIteratorArray& desc_begins,
	PerStageDescriptorNodeIteratorArray& desc_ends,
	PerStageDescriptorNodeArray&         out_nodes,
	ShaderStageFlags&                    out_stage_flags
) {
	ReflectionDescriptorNodePtr min_node        = nullptr;
	uint64_t                    min_binding_key = UINT64_MAX;
	size_t                      stage_count     = desc_begins.size();
	bool                        is_block        = false;

	out_nodes.clear();
	out_stage_flags = {};

	for (size_t i = 0; i < stage_count; ++i) {
		const auto it = desc_begins[i];
		
		if (it == desc_ends[i]) continue;

		uint64_t key = COMBINE_SET_BINDING((*it)->set, (*it)->binding);

		if (key < min_binding_key) {
			min_binding_key = key;
			min_node        = *it;
			is_block        = (*it)->type == ReflectionType::DescriptorBlock;
		}
	}

	if (!min_node) return false;

	// advance iterators which point to the minimum node and check compatibility
	for (size_t i = 0; i < stage_count; ++i) {
		const auto it = desc_begins[i];

		if (it == desc_ends[i]) continue;

		uint64_t key = COMBINE_SET_BINDING((*it)->set, (*it)->binding);

		if (key == min_binding_key) {
			if (!is_block && !check_node_compatible(min_node, *it))
				throw Exception("incompatible descriptor binding at" SET_BINDING_FMT(*min_node));
			
			out_nodes.push_back(*it);
			out_stage_flags |= (*it)->getStageFlags();
			desc_begins[i]++;
		}
	}

	// block nodes must be checked each other
	if (is_block) {
		size_t block_count = out_nodes.size();

		for (size_t i = 0; i < block_count; ++i)
			for (size_t j = i + 1; j < block_count; ++j)
				if (!check_block_compatible(out_nodes[i], out_nodes[j]))
					throw Exception("incompatible descriptor block binding at" SET_BINDING_FMT(*min_node));
	}

	return true;
}

static bool find_minimum_block_member_node(
	PerStageBlockMemberNodeIteratorArray& member_begins,
	PerStageBlockMemberNodeIteratorArray& member_ends,
	PerStageBlockMemberNodeArray&         out_member_nodes,
	ShaderStageFlags&                     out_stage_flags
) {
	ReflectionBlockMemberNodePtr min_node    = nullptr;
	uint32_t                     min_offset  = UINT32_MAX;
	size_t                       stage_count = member_begins.size();

	out_member_nodes.clear();
	out_stage_flags = {};

	for (size_t i = 0; i < stage_count; ++i) {
		if (const auto it = member_begins[i]; it != member_ends[i]) {
			if (uint32_t offset = (*it)->getOffset(); offset < min_offset) {
				min_offset = offset;
				min_node   = *it;
			}
		}
	}

	if (!min_node) return false;

	for (size_t i = 0; i < stage_count; ++i) {
		if (const auto it = member_begins[i]; it != member_ends[i]) {
			if (uint32_t offset = (*it)->getOffset(); offset == min_offset) {
				out_member_nodes.push_back(*it);
				out_stage_flags |= (*it)->getStageFlags();
				member_begins[i]++;
			}
		}
	}

	return true;
}

static void insert_name_list(
	ReflectionNameMap&         name_map,
	const ReflectionNameChain* name_list,
	const ReflectionNode*      new_node
) {
	for (const auto* curr = name_list; curr; curr = curr->next) {
		if (auto it = name_map.find(curr->name); it == name_map.end()) {
			name_map.insert({
				curr->name,
				ReflectionNodeMapping{ new_node, curr->stageFlags }
			});
		} else if (it->second.node != new_node) {
			throw Exception("name map collision with name: {}", static_cast<const char*>(curr->name));
		}
	}

}

static const ReflectionBlockMemberNode* clone_block_member_node(
	ReflectionContext&               ctx,
	const ReflectionBlockMemberNode* src_node,
	ShaderStageFlags                 stage_flags
) {
	switch (src_node->type) {
	case ReflectionType::Struct: {
		const auto* src_strcut   = static_cast<const ReflectionStructNode*>(src_node);
		auto*       new_struct   = construct_node<ReflectionStructNode>(ctx.memory);

		uint32_t member_count = static_cast<uint32_t>(src_strcut->memberNodes.size());
		uint32_t member_idx   = 0;
		auto*    member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, member_count);

		new_struct->type        = ReflectionType::Struct;
		new_struct->offset      = src_strcut->offset;
		new_struct->nameList    = construct_name_chain(ctx.memory, src_strcut->getName());
		new_struct->paddedSize  = src_strcut->paddedSize;
		new_struct->memberNodes = array_view{ member_nodes, src_strcut->memberNodes.size() };
		new_struct->nameMap     = ReflectionNameMap(src_strcut->nameMap, ctx.memory);

		for (auto& mapping : new_struct->nameMap)
			mapping.second.stageFlags = stage_flags;
		for (const auto* member : src_strcut->memberNodes)
			member_nodes[member_idx++] = clone_block_member_node(ctx, member, stage_flags);

		return new_struct;
	}
	case ReflectionType::Array: {
		const auto* src_array = static_cast<const ReflectionArrayNode*>(src_node);
		auto*       new_array = construct_node<ReflectionArrayNode>(ctx.memory);

		new_array->type         = ReflectionType::Array;
		new_array->offset       = src_array->offset;
		new_array->nameList     = construct_name_chain(ctx.memory, src_array->getName());
		new_array->paddedSize   = src_array->paddedSize;
		new_array->stride       = src_array->stride;
		new_array->elementCount = src_array->elementCount;
		new_array->elementNode  = clone_block_member_node(ctx, src_array->elementNode, stage_flags);

		return new_array;
	}
	case ReflectionType::Primitive: {
		const auto* src_prim = static_cast<const ReflectionPrimitiveNode*>(src_node);
		auto*       new_prim = construct_node<ReflectionPrimitiveNode>(ctx.memory);

		new_prim->type          = ReflectionType::Primitive;
		new_prim->offset        = src_prim->offset;
		new_prim->nameList      = construct_name_chain(ctx.memory, src_prim->getName());
		new_prim->paddedSize    = src_prim->paddedSize;
		new_prim->primitiveType = src_prim->primitiveType;

		return new_prim;
	}
	}

	VERA_ERROR_MSG("invalid reflection type");
}

//static const ReflectionBlockMemberNode* merge_block_member_node(
//	ReflectionContext&                       ctx,
//	array_view<ReflectionBlockMemberNodePtr> src_nodes
//) {
//	ShaderStageFlags       stage_flags = {}; 
//	ReflectionStringChain* name_list   = nullptr;
//
//	for (const auto* src : src_nodes) {
//		stage_flags |= src->getStageFlags();
//		name_list    = intern_string_chain(
//			ctx.memory,
//			name_list,
//			src->getName());
//	}
//
//	switch (src_nodes.front()->type) {
//	case ReflectionType::Struct: {
//		const auto* src_strcut   = static_cast<const ReflectionStructNode*>(src_nodes.front());
//		auto*       new_struct   = construct_node<ReflectionStructNode>(ctx.memory);
//
//		uint32_t member_count = src_strcut->memberNodes.size();
//		auto*    member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, member_count);
//
//		PerStageBlockMemberNodeArray block_members;
//		ReflectionNameMap            name_map(ctx.tempNameMapAllocator);
//
//		new_struct->type        = ReflectionType::Struct;
//		new_struct->offset      = src_strcut->offset;
//		new_struct->nameList    = name_list;
//		new_struct->blockSize   = src_strcut->blockSize;
//		new_struct->memberNodes = array_view{ member_nodes, src_strcut->memberNodes.size() };
//		new_struct->nameMap     = {};
//
//		for (uint32_t i = 0; i < member_count; ++i) {
//			block_members.clear();
//
//			for (const auto* src : src_nodes)
//				block_members.push_back(src->as<ReflectionStructNode>()->memberNodes[i]);
//
//			auto* new_member = merge_block_member_node(ctx, block_members);
//
//			member_nodes[i] = new_member;
//
//			for (const auto* curr = new_member->nameList; curr; curr = curr->next) {
//
//			}
//
//			for (const auto* block_member : block_members) {
//				const auto  member_stage_flags = block_member->getStageFlags();
//				const char* member_name        = block_member->getName();
//
//				if (auto it = name_map.find(member_name); it == name_map.end()) {
//					name_map.insert({
//						member_name,
//						ReflectionNodeMapping{ new_member, member_stage_flags  }
//					});
//				} else {
//					if (it->second.node != new_member)
//						throw Exception("name map collision with name: {}", member_name);
//
//					it->second.stageFlags |= member_stage_flags ;
//				}
//			}
//		}
//
//		new_struct->nameMap = ReflectionNameMap(name_map, ctx.nameMapAllocator);
//
//		return new_struct;
//	}
//	case ReflectionType::Array: {
//		const auto* src_array = static_cast<const ReflectionArrayNode*>(src_nodes.front());
//		auto*       new_array = construct_node<ReflectionArrayNode>(ctx.memory);
//
//		new_array->type         = ReflectionType::Array;
//		new_array->offset       = src_array->offset;
//		new_array->nameList     = name_list;
//		new_array->stride       = src_array->stride;
//		new_array->elementCount = src_array->elementCount;
//		new_array->elementNode  = clone_block_member_node(ctx, src_array->elementNode, stage_flags);
//
//		return new_array;
//	}
//	case ReflectionType::Primitive: {
//		const auto* src_prim = static_cast<const ReflectionPrimitiveNode*>(src_nodes.front());
//		auto*       new_prim = construct_node<ReflectionPrimitiveNode>(ctx.memory);
//
//		new_prim->type          = ReflectionType::Primitive;
//		new_prim->offset        = src_prim->offset;
//		new_prim->nameList      = name_list;
//		new_prim->primitiveType = src_prim->primitiveType;
//
//		return new_prim;
//	}
//	}
//}

static const ReflectionDescriptorNode* merge_descriptor_node(
	ReflectionContext&                      ctx,
	array_view<ReflectionDescriptorNodePtr> src_nodes,
	ShaderStageFlags                        stage_flags
) {
	switch (src_nodes.front()->type) {
	case ReflectionType::Descriptor: {
		const auto* first_desc = src_nodes.front();
		auto*       new_desc   = construct_node<ReflectionDescriptorNode>(ctx.memory);

		new_desc->type           = ReflectionType::Descriptor;
		new_desc->stageFlags     = stage_flags;
		new_desc->nameList       = nullptr;
		new_desc->descriptorType = first_desc->descriptorType;
		new_desc->set            = first_desc->set;
		new_desc->binding        = first_desc->binding;

		for (const auto* src : src_nodes)
			new_desc->nameList = intern_name_chain(ctx.memory, new_desc->nameList, src->getName());

		return new_desc;
	}
	case ReflectionType::DescriptorArray: {
		const auto* first_array = static_cast<const ReflectionDescriptorArrayNode*>(src_nodes.front());
		auto*       new_array   = construct_node<ReflectionDescriptorArrayNode>(ctx.memory);

		new_array->type           = ReflectionType::DescriptorArray;
		new_array->stageFlags     = stage_flags;
		new_array->nameList       = nullptr;
		new_array->descriptorType = first_array->descriptorType;
		new_array->set            = first_array->set;
		new_array->binding        = first_array->binding;
		new_array->elementCount   = first_array->elementCount;
		new_array->elementNode    = nullptr;
		new_array->stride         = first_array->stride;

		PerStageDescriptorNodeArray element_nodes;

		for (const auto* src : src_nodes) {
			new_array->nameList = intern_name_chain(ctx.memory, new_array->nameList, src->getName());
			element_nodes.push_back(src);
		}

		new_array->elementNode = merge_descriptor_node(ctx, element_nodes, stage_flags);

		return new_array;
	}
	case ReflectionType::DescriptorBlock: {
		const auto* first_block = static_cast<const ReflectionDescriptorBlockNode*>(src_nodes.front());
		auto*       new_block   = construct_node<ReflectionDescriptorBlockNode>(ctx.memory);

		new_block->type           = ReflectionType::DescriptorBlock;
		new_block->stageFlags     = stage_flags;
		new_block->nameList       = nullptr;
		new_block->descriptorType = first_block->descriptorType;
	
		std::pmr::vector<ReflectionBlockMemberNodePtr> block_members(ctx.tempMemory);
		ReflectionNameMap                              name_map(ctx.tempMemory);

		PerStageBlockMemberNodeIteratorArray member_begins;
		PerStageBlockMemberNodeIteratorArray member_ends;
		PerStageBlockMemberNodeArray         member_nodes;
		ShaderStageFlags                     stage_flags;

		for (const auto* src : src_nodes) {
			const auto* src_block = src->as<ReflectionDescriptorBlockNode>();
			member_begins.push_back(src_block->memberNodes.begin());
			member_ends.push_back(src_block->memberNodes.end());
			new_block->nameList = intern_name_chain(ctx.memory, new_block->nameList, src->getName());
		}

		while (find_minimum_block_member_node(member_begins, member_ends, member_nodes, stage_flags)) {
			// TODO: support merging with varying name(when member_nodes.size() > 1)
			auto* new_member = clone_block_member_node(ctx, member_nodes.front(), stage_flags);

			block_members.push_back(new_member);
			name_map.insert({
				new_member->getName(),
				ReflectionNodeMapping{ new_member, stage_flags }
			});
		}

		return new_block;
	}
	}

	VERA_ERROR_MSG("invalid reflection type");
}

static void verify_push_constant_compatibility(
	array_view<ReflectionPushConstantNodePtr> pc_nodes,
	ShaderStageFlags                          src_stage_mask,
	uint32_t                                  target_idx
) {
	ReflectionPushConstantNodePtr target_node = pc_nodes[target_idx];

	for (const auto* pc : pc_nodes) {
		if (!(pc->getStageFlags() & src_stage_mask)) continue;

		if (!check_block_compatible(target_node, pc)) {
			auto src_stage    = pc->getStageFlags().flag_bit();
			auto target_stage = target_node->getStageFlags().flag_bit();

			throw Exception("incompatible push constant block between stages "
				"'{}' and '{}' ",
				get_shader_stage_string(src_stage),
				get_shader_stage_string(target_stage));
		}
	}
}

static const ReflectionPushConstantNode* merge_push_constant_node(
	ReflectionContext&                        ctx,
	array_view<ReflectionPushConstantNodePtr> src_nodes,
	const PushConstantRange&                  target_range
) {
	PerStageBlockMemberNodeIteratorArray pc_begins;
	PerStageBlockMemberNodeIteratorArray pc_ends;
	PerStageBlockMemberNodeArray         min_pc_nodes;
	ShaderStageFlags                     stage_flags;
	ReflectionNameChain*                 name_list  = nullptr;

	for (const auto& pc : src_nodes) {
		pc_begins.push_back(pc->memberNodes.begin());
		pc_ends.push_back(pc->memberNodes.end());
		stage_flags |= pc->getStageFlags();
		name_list    = intern_name_chain(ctx.memory, name_list, pc->getName());
	}

	std::pmr::vector<ReflectionBlockMemberNodePtr> block_members(ctx.tempMemory);
	ReflectionNameMap                              name_map(ctx.tempMemory);

	auto* new_pc = construct_node<ReflectionPushConstantNode>(ctx.memory);
	new_pc->type        = ReflectionType::PushConstant;
	new_pc->stageFlags  = stage_flags;
	new_pc->nameList    = name_list;
	new_pc->memberNodes = {};
	new_pc->nameMap     = {};

	while (find_minimum_block_member_node(pc_begins, pc_ends, min_pc_nodes, stage_flags)) {
		auto* new_member = clone_block_member_node(ctx, min_pc_nodes.front(), stage_flags);

		block_members.push_back(new_member);

		name_map.insert({
			new_member->getName(),
			ReflectionNodeMapping{ new_member, stage_flags }
		});
	}

	auto** member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, block_members.size());
	std::copy(VERA_SPAN(block_members), member_nodes);

	new_pc->memberNodes = array_view{ member_nodes, block_members.size() };
	new_pc->nameMap     = ReflectionNameMap(name_map, ctx.memory);

	return new_pc;
}

static const ReflectionRootNode* merge_impl(
	ReflectionContext&                   ctx,
	PerStageRootNodeArray&               root_nodes,
	PerStageDescriptorNodeIteratorArray& desc_begins,
	PerStageDescriptorNodeIteratorArray& desc_ends,
	PerStagePushConstantNodeArray&       pc_nodes
) {
	uint32_t entry_point_count = 0;
	uint32_t entry_point_idx   = 0;

	for (const auto* root_node : root_nodes)
		entry_point_count += root_node->entryPoints.empty() ? 0 : 1;

	auto* entry_points = construct_array<ReflectionEntryPoint>(ctx.memory, entry_point_count);

	for (const auto* root_node : root_nodes) {
		if (root_node->entryPoints.empty()) continue;

		entry_points[entry_point_idx++] = ReflectionEntryPoint{
			root_node->getShaderStageFlags(),
			construct_string(ctx.memory, root_node->entryPoints.front().name)
		};
	}

	auto* root_node = construct_node<ReflectionRootNode>(ctx.memory);
	root_node->type              = ReflectionType::Root;
	root_node->stageFlags        = ctx.stageFlags;
	root_node->entryPoints       = array_view(entry_points, entry_point_count);
	root_node->minSet            = UINT32_MAX;
	root_node->maxSet            = 0;
	root_node->descriptorCount   = 0;
	root_node->pushConstantCount = 0;
	root_node->targetFlags       = ReflectionTargetFlagBits::PipelineLayout;

	PerStageDescriptorNodeArray                       min_nodes;
	ShaderStageFlags                                  stage_flags;
	ReflectionNameMap                                 name_map(ctx.tempMemory);
	std::pmr::vector<const ReflectionRootMemberNode*> member_nodes(ctx.tempMemory);

	while (find_minimum_descriptor_node(desc_begins, desc_ends, min_nodes, stage_flags)) {
		auto* new_node = merge_descriptor_node(ctx, min_nodes, stage_flags);

		member_nodes.push_back(new_node);
		insert_name_list(name_map, new_node->nameList, new_node);

		root_node->minSet = std::min(root_node->minSet, min_nodes.front()->set);
		root_node->maxSet = std::max(root_node->maxSet, min_nodes.front()->set);
		root_node->descriptorCount++;
	}

	if (!pc_nodes.empty()) {
		static_vector<PushConstantRange, MAX_SHADER_STAGE_COUNT> pc_ranges;

		for (const auto& pc : pc_nodes)
			pc_ranges.push_back(pc->pushConstantRange);

		// if range intersects each other, merge into one range
		for (size_t i = 0; i < pc_ranges.size(); ++i) {
			auto& lhs_range = pc_ranges[i];

			for (size_t j = i + 1; j < pc_ranges.size();) {
				auto& rhs_range = pc_ranges[j];

				if (is_pc_range_intersect(lhs_range, rhs_range)) {
					auto target_idx = static_cast<uint32_t>(j);

					verify_push_constant_compatibility(pc_nodes, lhs_range.stageFlags, target_idx);

					lhs_range.stageFlags |= rhs_range.stageFlags;
					lhs_range.offset      = std::min(lhs_range.offset, rhs_range.offset);
					lhs_range.size        = std::max(
						lhs_range.offset + lhs_range.size,
						rhs_range.offset + rhs_range.size) - lhs_range.offset;

					pc_ranges.erase(pc_ranges.cbegin() + j);
				} else {
					++j;
				}
			}
		}

		for (const auto& pc_range : pc_ranges) {
			auto* new_pc = merge_push_constant_node(ctx, pc_nodes, pc_range);
			member_nodes.push_back(new_pc);
			insert_name_list(name_map, new_pc->nameList, new_pc);
			root_node->pushConstantCount++;
		}
	}

	if (!member_nodes.empty()) {
		auto** root_members = construct_array<ReflectionRootMemberNodePtr>(ctx.memory, member_nodes.size());
		std::copy(VERA_SPAN(member_nodes), root_members);

		root_node->nameMap     = ReflectionNameMap(name_map, ctx.memory);
		root_node->memberNodes = array_view{ root_members, member_nodes.size() };
	}

	return root_node;
}

ShaderStageFlags ReflectionRootNode::getShaderStageFlags() const VERA_NOEXCEPT
{
	return stageFlags;
}

ReflectionTargetFlags ReflectionRootNode::getTargetFlags() const VERA_NOEXCEPT
{
	return targetFlags;
}

const char* ReflectionRootNode::getEntryPointName(ShaderStageFlagBits stage) const VERA_NOEXCEPT
{
	// default stage will return first entry point name
	for (const auto& entry_point : entryPoints)
		if (entry_point.stageFlags.has(stage))
			return entry_point.name;

	return nullptr;
}

uint32_t ReflectionRootNode::getSetCount() const VERA_NOEXCEPT
{
	return static_cast<uint32_t>(setRanges.size());
}

uint32_t ReflectionRootNode::getMinSet() const VERA_NOEXCEPT
{
	return minSet;
}

uint32_t ReflectionRootNode::getMaxSet() const VERA_NOEXCEPT
{
	return maxSet;
}

array_view<const ReflectionDescriptorNode*> ReflectionRootNode::enumerateDescriptor() const VERA_NOEXCEPT
{
	const auto*    data       = memberNodes.data();
	const uint32_t desc_count = descriptorCount;

	return array_view{
		reinterpret_cast<const ReflectionDescriptorNode* const*>(data),
		desc_count
	};
}

array_view<const ReflectionDescriptorNode*> ReflectionRootNode::enumerateDescriptorSet(uint32_t set) const VERA_NOEXCEPT
{
	if (set >= setRanges.size()) return {};
	return setRanges[set];
}

array_view<const ReflectionPushConstantNode*> ReflectionRootNode::enumeratePushConstant() const VERA_NOEXCEPT
{
	const auto*    data       = memberNodes.data();
	const uint32_t desc_count = descriptorCount;
	const uint32_t pc_count   = pushConstantCount;

	return array_view{
		reinterpret_cast<const ReflectionPushConstantNode* const*>(data + desc_count),
		pc_count
	};
}

const ReflectionDescriptorNode* ReflectionRootNode::findDescriptor(uint32_t set, uint32_t binding) const VERA_NOEXCEPT
{
	if (auto it = bindingMap.find(COMBINE_SET_BINDING(set, binding)); it != bindingMap.end())
		return it->second;
	
	return nullptr;
}

const ReflectionPushConstantNode* ReflectionRootNode::findPushConstant(ShaderStageFlags stage_flags) const VERA_NOEXCEPT
{
	const auto* const* pc_first = memberNodes.data() + descriptorCount;
	const auto* const* pc_last  = pc_first + pushConstantCount;

	for (auto pc_it = pc_first; pc_it != pc_last; ++pc_it)
		if ((*pc_it)->getStageFlags().has(stage_flags))
			return reinterpret_cast<const ReflectionPushConstantNode*>(*pc_it);

	return nullptr;
}

array_view<uint32_t> ReflectionDesc::stripReflectionInstructions(const uint32_t* spirv_code, size_t size_in_byte)
{
	return { spirv_code, size_in_byte / sizeof(uint32_t) };

	// below is not implemented correctly

	//enum : uint32_t {
	//	SPIRV_MAGIC_NUMBER = 0x07230203,
	//	SPV_OP_FUNCTION    = 54
	//};

	//VERA_ASSERT_MSG(size_in_byte % sizeof(uint32_t) == 0, "invalid SPIR-V code size");

	//const uint32_t* first = spirv_code;
	//const uint32_t* last  = spirv_code + size_in_byte / sizeof(uint32_t);

	//if (*first != SPIRV_MAGIC_NUMBER)
	//	throw Exception("invalid SPIR-V magic number");

	//first += 5; // skip spirv header

	//while (first != last) {
	//	uint32_t word = *first;
	//	uint32_t op   = word & 0xFFFF;
	//	uint32_t len  = (word >> 16) & 0xFFFF;

	//	VERA_ASSERT_MSG(len != 0, "invalid SPIR-V instruction length");

	//	if (op == SPV_OP_FUNCTION) {
	//		return {
	//			spirv_code,
	//			static_cast<size_t>(first - spirv_code)
	//		};
	//	}

	//	first += len;
	//}

	//throw Exception("failed to strip reflection instructions from SPIR-V code");
}

ReflectionDesc::ReflectionDesc() :
	m_memory(INITIAL_MONOTONIC_CHUNK_SIZE),
	m_root_node(nullptr) {}

ReflectionDesc::~ReflectionDesc()
{
	m_memory.release();
}

void ReflectionDesc::parse(const uint32_t* spirv_code, size_t size_in_byte)
{
	spv_reflect::ShaderModule shader_module(size_in_byte, spirv_code);

	if (shader_module.GetResult() != SpvReflectResult::SPV_REFLECT_RESULT_SUCCESS)
		throw Exception("SPIR-V reflection failed: {}",
			get_spv_reflect_result_string(shader_module.GetResult()));

	std::pmr::monotonic_buffer_resource temp_memory(VERA_KIB(1));

	ReflectionContext ctx = {
		.memory     = &m_memory,
		.tempMemory = &temp_memory,
		.stageFlags = to_shader_stage(shader_module.GetShaderStage())
	};

	m_root_node = parse_impl(ctx, shader_module.GetShaderModule());
}

void ReflectionDesc::merge(array_view<const ReflectionDesc*> reflections)
{
	VERA_ASSERT_MSG(reflections.size() <= MAX_SHADER_STAGE_COUNT,
		"exceeded maximum shader stage count for merging reflections");

	PerStageRootNodeArray               root_nodes;
	PerStageDescriptorNodeIteratorArray desc_begins;
	PerStageDescriptorNodeIteratorArray desc_ends;
	PerStagePushConstantNodeArray       pcs;
	ShaderStageFlags                    stage_flags;

	for (const auto* reflection : reflections) {
		if (!reflection || reflection->empty()) continue;

		const auto* root_node = reflection->getRootNode();
		const auto  target    = root_node->getTargetFlags();

		if (target & (ReflectionTargetFlagBits::DescriptorSetLayout &
			  ReflectionTargetFlagBits::Shader))
			throw Exception("only descriptor set layout and shader reflection can be merged");

		if (target == ReflectionTargetFlagBits::Shader) {
			if (stage_flags.has(root_node->getShaderStageFlags()))
				throw Exception("duplicate shader stage in merging reflections");
			stage_flags |= root_node->getShaderStageFlags();
		}

		if (auto desc_view = root_node->enumerateDescriptor(); !desc_view.empty()) {
			desc_begins.push_back(desc_view.data());
			desc_ends.push_back(desc_view.data() + desc_view.size());
		}

		if (const auto* pc_node = root_node->findPushConstant())
			pcs.push_back(pc_node);

		root_nodes.push_back(root_node);
	}

	std::pmr::monotonic_buffer_resource temp_memory(VERA_KIB(1));

	ReflectionContext ctx = {
		.memory     = &m_memory,
		.tempMemory = &temp_memory,
		.stageFlags = stage_flags
	};

	m_root_node = merge_impl(ctx, root_nodes, desc_begins, desc_ends, pcs);
}

const ReflectionRootNode* ReflectionDesc::getRootNode() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_root_node != nullptr, "reflection descriptor not initialized");
	return m_root_node;
}

void ReflectionDesc::clear() VERA_NOEXCEPT
{
	m_memory.release();
	m_root_node = nullptr;
}

bool ReflectionDesc::empty() const VERA_NOEXCEPT
{
	return m_root_node == nullptr;
}

VERA_NAMESPACE_END
