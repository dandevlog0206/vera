#include "reflection_node.h"

#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/logger.h"
#include "../../include/vera/util/static_vector.h"
#include "../spirv/spirv_reflect.h"

#define MAX_SHADER_STAGE_COUNT 16
#define INITIAL_MONOTONIC_CHUNK_SIZE VERA_KIB(2)

#define VALIDATE_NODE_LAYOUT(node_type, member, prop) offsetof(node_type, member) == ReflectionNode::prop_off_v<prop>
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
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, name, Name));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, descriptorType, DescriptorType));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, set, Set));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorNode, binding, Binding));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, name, Name));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, descriptorType, DescriptorType));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, set, Set));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, binding, Binding));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, elementNode, ElementNode));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, elementCount, ElementCount));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorArrayNode, stride, Stride));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, name, Name));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, descriptorType, DescriptorType));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, set, Set));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, binding, Binding));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionDescriptorBlockNode, block, Block));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, stageFlags, StageFlags));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, name, Name));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPushConstantNode, block, Block));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, offset, Offset));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, name, Name));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, paddedSize, PaddedSize));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, memberNodes, MemberNodes));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, nameMap, NameMap));
	
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, offset, Offset));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, name, Name));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, paddedSize, PaddedSize));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, elementNode, ElementNode));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, elementCount, ElementCount));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionArrayNode, stride, Stride));

	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, type, Type));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, offset, Offset));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, name, Name));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionStructNode, paddedSize, PaddedSize));
	static_assert(VALIDATE_NODE_LAYOUT(ReflectionPrimitiveNode, primitiveType, PrimitiveType));
};

typedef const ReflectionNode* ReflectionNodePtr;
typedef const ReflectionResourceNode* ReflectionResourceNodePtr;
typedef const ReflectionBlockNode* ReflectionBlockNodePtr;
typedef const ReflectionRootNode* ReflectionRootNodePtr;
typedef const ReflectionDescriptorNode* ReflectionDescriptorNodePtr;
typedef const ReflectionDescriptorArrayNode* ReflectionDescriptorArrayNodePtr;
typedef const ReflectionDescriptorBlockNode* ReflectionDescriptorBlockNodePtr;
typedef const ReflectionPushConstantNode* ReflectionPushConstantNodePtr;
typedef const ReflectionStructNode* ReflectionStructNodePtr;
typedef const ReflectionArrayNode* ReflectionArrayNodePtr;
typedef const ReflectionPrimitiveNode* ReflectionPrimitiveNodePtr;

typedef static_vector<const ReflectionResourceNodePtr*, MAX_SHADER_STAGE_COUNT> PerStageResourceNodeIteratorArray;
typedef static_vector<const ReflectionBlockNodePtr*, MAX_SHADER_STAGE_COUNT> PerStageBlockNodeIteratorArray;
typedef static_vector<const ReflectionDescriptorNodePtr*, MAX_SHADER_STAGE_COUNT> PerStageDescriptorNodeIteratorArray;
typedef static_vector<const ReflectionRootNode*, MAX_SHADER_STAGE_COUNT>  PerStageRootNodeArray;
typedef static_vector<const ReflectionResourceNode*, MAX_SHADER_STAGE_COUNT>  PerStageRootMemberNodeArray;
typedef static_vector<const ReflectionBlockNode*, MAX_SHADER_STAGE_COUNT> PerStageBlockNodeArray;
typedef static_vector<const ReflectionDescriptorNode*, MAX_SHADER_STAGE_COUNT> PerStageDescriptorNodeArray;
typedef static_vector<const ReflectionPushConstantNode*, MAX_SHADER_STAGE_COUNT>  PerStagePushConstantNodeArray;
typedef static_vector<const ReflectionStructNode*, MAX_SHADER_STAGE_COUNT>  PerStageStructNodeArray;

class TestMemoryResource : public std::pmr::memory_resource
{
protected:
	void* do_allocate(size_t _Bytes, size_t _Align) override
	{
		return _aligned_malloc(_Bytes, 8);
	}

	void do_deallocate(void* _Ptr, size_t _Bytes, size_t _Align) override
	{
		_aligned_free(_Ptr);
	}

	bool do_is_equal(const memory_resource& _That) const noexcept override
	{
		return this == &_That;
	}
};

struct ReflectionContext
{
	std::pmr::memory_resource* memory;
	std::pmr::memory_resource* tempMemory;
	ShaderStageFlags           stageFlags;
};

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

static ReflectionPrimitiveType reflect_matrix_type(const SpvReflectBlockVariable& block)
{
	if (block.decoration_flags & SPV_REFLECT_DECORATION_ROW_MAJOR) {
		switch (block.numeric.scalar.width) {
		case 32:
			switch (block.numeric.matrix.row_count) {
			case 2:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::RFloat2x2;
				case 3: return ReflectionPrimitiveType::RFloat2x3;
				case 4: return ReflectionPrimitiveType::RFloat2x4;
				}
				break;
			case 3:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::RFloat3x2;
				case 3: return ReflectionPrimitiveType::RFloat3x3;
				case 4: return ReflectionPrimitiveType::RFloat3x4;
				}
				break;
			case 4:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::RFloat4x2;
				case 3: return ReflectionPrimitiveType::RFloat4x3;
				case 4: return ReflectionPrimitiveType::RFloat4x4;
				}
				break;
			};
			break;
		case 64:
			switch (block.numeric.matrix.row_count) {
			case 2:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::RDouble2x2;
				case 3: return ReflectionPrimitiveType::RDouble2x3;
				case 4: return ReflectionPrimitiveType::RDouble2x4;
				}
				break;
			case 3:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::RDouble3x2;
				case 3: return ReflectionPrimitiveType::RDouble3x3;
				case 4: return ReflectionPrimitiveType::RDouble3x4;
				}
				break;
			case 4:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::RDouble4x2;
				case 3: return ReflectionPrimitiveType::RDouble4x3;
				case 4: return ReflectionPrimitiveType::RDouble4x4;
				}
				break;
			};
			break;
		}
	} else if (block.decoration_flags & SPV_REFLECT_DECORATION_COLUMN_MAJOR) {
		switch (block.numeric.scalar.width) {
		case 32:
			switch (block.numeric.matrix.row_count) {
			case 2:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::CFloat2x2;
				case 3: return ReflectionPrimitiveType::CFloat2x3;
				case 4: return ReflectionPrimitiveType::CFloat2x4;
				}
				break;
			case 3:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::CFloat3x2;
				case 3: return ReflectionPrimitiveType::CFloat3x3;
				case 4: return ReflectionPrimitiveType::CFloat3x4;
				}
				break;
			case 4:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::CFloat4x2;
				case 3: return ReflectionPrimitiveType::CFloat4x3;
				case 4: return ReflectionPrimitiveType::CFloat4x4;
				}
				break;
			};
			break;
		case 64:
			switch (block.numeric.matrix.row_count) {
			case 2:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::CDouble2x2;
				case 3: return ReflectionPrimitiveType::CDouble2x3;
				case 4: return ReflectionPrimitiveType::CDouble2x4;
				}
				break;
			case 3:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::CDouble3x2;
				case 3: return ReflectionPrimitiveType::CDouble3x3;
				case 4: return ReflectionPrimitiveType::CDouble3x4;
				}
				break;
			case 4:
				switch (block.numeric.matrix.column_count) {
				case 2: return ReflectionPrimitiveType::CDouble4x2;
				case 3: return ReflectionPrimitiveType::CDouble4x3;
				case 4: return ReflectionPrimitiveType::CDouble4x4;
				}
				break;
			};
			break;
		}
	}

	throw Exception("failed to reflect matrix type");
}

static ReflectionPrimitiveType reflect_vector_type(const SpvReflectBlockVariable& block)
{
	auto type_flags = block.type_description->type_flags;

	if (type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT) {
		switch (block.numeric.scalar.width) {
		case 32:
			switch (block.numeric.vector.component_count) {
			case 2: return ReflectionPrimitiveType::Float2;
			case 3: return ReflectionPrimitiveType::Float3;
			case 4: return ReflectionPrimitiveType::Float4;
			}
			break;
		case 64:
			switch (block.numeric.vector.component_count) {
			case 2: return ReflectionPrimitiveType::Double2;
			case 3: return ReflectionPrimitiveType::Double3;
			case 4: return ReflectionPrimitiveType::Double4;
			}
			break;
		}
	} else if (type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT) {
		if (block.numeric.scalar.signedness) {
			switch (block.numeric.scalar.width) {
			case 8:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Char2;
				case 3: return ReflectionPrimitiveType::Char3;
				case 4: return ReflectionPrimitiveType::Char4;
				}
				break;
			case 16:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Short2;
				case 3: return ReflectionPrimitiveType::Short3;
				case 4: return ReflectionPrimitiveType::Short4;
				}
				break;
			case 32:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Int2;
				case 3: return ReflectionPrimitiveType::Int3;
				case 4: return ReflectionPrimitiveType::Int4;
				}
				break;
			case 64:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Long2;
				case 3: return ReflectionPrimitiveType::Long3;
				case 4: return ReflectionPrimitiveType::Long4;
				}
				break;
			}
		} else {
			switch (block.numeric.scalar.width) {
			case 8:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UChar2;
				case 3: return ReflectionPrimitiveType::UChar3;
				case 4: return ReflectionPrimitiveType::UChar4;
				}
				break;
			case 16:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UShort2;
				case 3: return ReflectionPrimitiveType::UShort3;
				case 4: return ReflectionPrimitiveType::UShort4;
				}
				break;
			case 32:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UInt2;
				case 3: return ReflectionPrimitiveType::UInt3;
				case 4: return ReflectionPrimitiveType::UInt4;
				}
				break;
			case 64:
				switch (block.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::ULong2;
				case 3: return ReflectionPrimitiveType::ULong3;
				case 4: return ReflectionPrimitiveType::ULong4;
				}
				break;
			}
		}
	} else if (type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_BOOL) {
		switch (block.numeric.scalar.width) {
		case 2: return ReflectionPrimitiveType::Bool2;
		case 3: return ReflectionPrimitiveType::Bool3;
		case 4: return ReflectionPrimitiveType::Bool4;
		}
	}

	throw Exception("failed to reflect vector type");
}

static ReflectionPrimitiveType reflect_float_type(const SpvReflectBlockVariable& block)
{
	switch (block.numeric.scalar.width) {
	case 32: return ReflectionPrimitiveType::Float;
	case 64: return ReflectionPrimitiveType::Double;
	}

	throw Exception("failed to reflect float type");
}

static ReflectionPrimitiveType reflect_int_type(const SpvReflectBlockVariable& block)
{
	if (block.numeric.scalar.signedness) {
		switch (block.numeric.scalar.width) {
		case 8:  return ReflectionPrimitiveType::Char;
		case 16: return ReflectionPrimitiveType::Short;
		case 32: return ReflectionPrimitiveType::Int;
		case 64: return ReflectionPrimitiveType::Long;
		}
	} else {
		switch (block.numeric.scalar.width) {
		case 8:  return ReflectionPrimitiveType::UChar;
		case 16: return ReflectionPrimitiveType::UShort;
		case 32: return ReflectionPrimitiveType::UInt;
		case 64: return ReflectionPrimitiveType::ULong;
		}
	}

	throw Exception("failed to reflect int type");
}

static ReflectionPrimitiveType parse_primitive_type(const SpvReflectBlockVariable& block)
{
	auto type_flags = block.type_description->type_flags;

	if (type_flags& SPV_REFLECT_TYPE_FLAG_MATRIX)
		return reflect_matrix_type(block);
	else if (type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
		return reflect_vector_type(block);
	else if (type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
		return reflect_float_type(block);
	else if (type_flags & SPV_REFLECT_TYPE_FLAG_INT)
		return reflect_int_type(block);
	else if (type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
		return ReflectionPrimitiveType::Bool;

	throw Exception("invalid type flags");
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

static const ReflectionBlockNode* parse_block_variable(
	ReflectionContext&             ctx,
	const SpvReflectBlockVariable& block,
	const uint32_t                 array_dim = 0
) {
	if (block.array.dims_count != array_dim) {
		auto* array_node = construct_node<ReflectionArrayNode>(ctx.memory);
		array_node->type         = ReflectionNodeType::Array;
		array_node->stageFlags   = ctx.stageFlags;
		array_node->name         = construct_string(ctx.memory, block.name);
		array_node->offset       = block.offset;
		array_node->paddedSize   = block.padded_size;
		array_node->elementNode  = parse_block_variable(ctx, block, array_dim + 1);
		array_node->elementCount = block.array.dims[array_dim] <= 1 ? UINT32_MAX : block.array.dims[array_dim];
		array_node->stride       = get_array_stride(block.array, array_dim);

		return array_node->as<ReflectionBlockNode>();
	}

	if (block.member_count) {
		auto* struct_node = construct_node<ReflectionStructNode>(ctx.memory);
		struct_node->type       = ReflectionNodeType::Struct;
		struct_node->stageFlags = ctx.stageFlags;
		struct_node->name       = construct_string(ctx.memory, block.name);
		struct_node->offset     = block.offset;

		ReflectionNameMap name_map(ctx.tempMemory);

		uint32_t     member_count = block.member_count;
		uint32_t     member_idx   = 0;
		const auto** member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, member_count);
		const auto&  last_member  = block.members[member_count - 1];

		for (const auto& member : array_view(block.members, member_count)) {
			auto* new_node = parse_block_variable(ctx, member);

			member_nodes[member_idx++] = new_node;
			name_map.insert({ new_node->name, new_node });
		}

		// assign name map with constructed name map
		struct_node->nameMap     = ReflectionNameMap(name_map, ctx.memory);
		struct_node->memberNodes = array_view{ member_nodes, member_count };

		return struct_node->as<ReflectionBlockNode>();
	}
	
	auto* prim_node = construct_node<ReflectionPrimitiveNode>(ctx.memory);
	prim_node->type          = ReflectionNodeType::Primitive;
	prim_node->stageFlags    = ctx.stageFlags;
	prim_node->name          = construct_string(ctx.memory, block.name);
	prim_node->offset        = block.offset;
	prim_node->paddedSize    = block.padded_size;
	prim_node->primitiveType = parse_primitive_type(block);

	return prim_node->as<ReflectionBlockNode>();
}

const ReflectionPushConstantNode* parse_push_constant(
	ReflectionContext&             ctx,
	const SpvReflectBlockVariable& block
) {
	auto* pc_node = construct_node<ReflectionPushConstantNode>(ctx.memory);
	pc_node->type       = ReflectionNodeType::PushConstant;
	pc_node->stageFlags = ctx.stageFlags;
	pc_node->name       = construct_string(ctx.memory, block.name);
	pc_node->offset     = block.offset;
	pc_node->paddedSize = block.padded_size;
	pc_node->block      = parse_block_variable(ctx, block)->as<ReflectionStructNode>();

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
		array_node->type           = ReflectionNodeType::DescriptorArray;
		array_node->stageFlags     = ctx.stageFlags;
		array_node->name           = construct_string(ctx.memory, binding.name);
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
		block_node->type           = ReflectionNodeType::DescriptorBlock;
		block_node->stageFlags     = ctx.stageFlags;
		block_node->name           = construct_string(ctx.memory, binding.name);
		block_node->descriptorType = to_descriptor_type(binding.descriptor_type);
		block_node->set            = binding.set;
		block_node->binding        = binding.binding;
		block_node->block          = parse_block_variable(ctx, binding.block)->as<ReflectionStructNode>();

		return block_node->as<ReflectionDescriptorNode>();
	}

	auto* desc_node = construct_node<ReflectionDescriptorNode>(ctx.memory);
	desc_node->type           = ReflectionNodeType::Descriptor;
	desc_node->stageFlags     = ctx.stageFlags;
	desc_node->name           = construct_string(ctx.memory, binding.name);
	desc_node->descriptorType = to_descriptor_type(binding.descriptor_type);
	desc_node->set            = binding.set;
	desc_node->binding        = binding.binding;

	return desc_node->as<ReflectionDescriptorNode>();
}

static ReflectionRootNode* parse_impl(
	ReflectionContext&            ctx,
	const SpvReflectShaderModule& module
) {
	if (module.entry_point_count != 1)
		throw Exception("SPIR-V module must have exactly one entry point");
	if (1 < module.push_constant_block_count)
		throw Exception("SPIR-V module must have at most one push constant block");

	ReflectionNameMap    name_map(ctx.tempMemory);
	ReflectionBindingMap binding_map(ctx.tempMemory);

	uint32_t            desc_node_count   = module.descriptor_binding_count;
	uint32_t            pc_node_count     = module.push_constant_block_count;
	uint32_t            root_member_count = desc_node_count + pc_node_count;
	uint32_t            root_member_idx   = 0;
	const auto**        root_members      = construct_array<ReflectionResourceNodePtr>(ctx.memory, root_member_count);
	ReflectionSetRange* set_ranges        = nullptr;
	uint32_t            set_count         = 0;

	if (desc_node_count > 0) {
		set_count  = module.descriptor_bindings[desc_node_count - 1].set + 1;
		set_ranges = construct_array<ReflectionSetRange>(ctx.memory, set_count);
	}

	auto* root_node = construct_node<ReflectionRootNode>(ctx.memory);
	root_node->type              = ReflectionNodeType::Root;
	root_node->stageFlags        = to_shader_stage(module.shader_stage);
	root_node->targetFlags       = ReflectionTargetFlagBits::Shader;
	root_node->descriptorCount   = desc_node_count;
	root_node->pushConstantCount = pc_node_count;
	root_node->minSet            = UINT32_MAX;
	root_node->maxSet            = 0;

	for (const auto& set : array_view(module.descriptor_sets, module.descriptor_set_count)) {
		const auto* last_binding = set.bindings[set.binding_count - 1];
		uint32_t    desc_offset  = root_member_idx;

		for (const auto* binding : array_view(set.bindings, set.binding_count)) {
			if (binding != last_binding && is_unsized_array(binding->array))
					throw Exception("only last binding of descriptor set can be unsized array at "
						SET_BINDING_FMT(*binding));

			auto* new_node = parse_descriptor_binding(ctx, *binding);

			root_members[root_member_idx++] = new_node->as<ReflectionResourceNode>();
			name_map.insert({ new_node->getName(), new_node });
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

		root_members[root_member_idx++] = pc_node->as<ReflectionResourceNode>();
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

static const ReflectionBlockNode* clone_block_node(
	ReflectionContext&         ctx,
	const ReflectionBlockNode* src_node
) {
	switch (src_node->type) {
	case ReflectionNodeType::Struct: {
		const auto* src_strcut   = static_cast<const ReflectionStructNode*>(src_node);
		auto*       new_struct   = construct_node<ReflectionStructNode>(ctx.memory);

		size_t   member_count = src_strcut->memberNodes.size();
		auto*    member_nodes = construct_array<ReflectionBlockNodePtr>(ctx.memory, member_count);
		uint32_t member_idx   = 0;

		new_struct->type        = ReflectionNodeType::Struct;
		new_struct->stageFlags  = src_strcut->stageFlags;
		new_struct->name        = construct_string(ctx.memory, src_strcut->name);
		new_struct->offset      = src_strcut->offset;
		new_struct->paddedSize  = src_strcut->paddedSize;
		new_struct->memberNodes = array_view{ member_nodes, src_strcut->memberNodes.size() };
		new_struct->nameMap     = ReflectionNameMap(src_strcut->nameMap, ctx.memory);

		for (const auto* member : src_strcut->memberNodes)
			member_nodes[member_idx++] = clone_block_node(ctx, member);

		return new_struct;
	}
	case ReflectionNodeType::Array: {
		const auto* src_array = static_cast<const ReflectionArrayNode*>(src_node);
		auto*       new_array = construct_node<ReflectionArrayNode>(ctx.memory);

		new_array->type         = ReflectionNodeType::Array;
		new_array->stageFlags   = src_array->stageFlags;
		new_array->name         = construct_string(ctx.memory, src_array->name);
		new_array->offset       = src_array->offset;
		new_array->paddedSize   = src_array->paddedSize;
		new_array->stride       = src_array->stride;
		new_array->elementCount = src_array->elementCount;
		new_array->elementNode  = clone_block_node(ctx, src_array->elementNode);

		return new_array;
	}
	case ReflectionNodeType::Primitive: {
		const auto* src_prim = static_cast<const ReflectionPrimitiveNode*>(src_node);
		auto*       new_prim = construct_node<ReflectionPrimitiveNode>(ctx.memory);

		new_prim->type          = ReflectionNodeType::Primitive;
		new_prim->stageFlags    = src_prim->stageFlags;
		new_prim->name          = construct_string(ctx.memory, src_prim->name);
		new_prim->offset        = src_prim->offset;
		new_prim->paddedSize    = src_prim->paddedSize;
		new_prim->primitiveType = src_prim->primitiveType;

		return new_prim;
	}
	}

	VERA_ERROR_MSG("invalid reflection type");
}

static const ReflectionBlockNode* find_minimum_block_node(
	PerStageBlockNodeIteratorArray& node_begins,
	PerStageBlockNodeIteratorArray& node_ends,
	ShaderStageFlags&               out_stage_flags
) {
	const ReflectionBlockNode* min_node    = nullptr;
	uint32_t                   min_offset  = UINT32_MAX;
	size_t                     stage_count = node_begins.size();

	out_stage_flags = {};

	for (size_t i = 0; i < stage_count; ++i) {
		if (node_begins[i] == node_ends[i]) continue;

		if (uint32_t offset = (*node_begins[i])->offset; offset < min_offset) {
			min_offset = offset;
			min_node   = *node_begins[i];
		}
	}

	if (!min_node) return nullptr;

	for (size_t i = 0; i < stage_count; ++i) {
		if (node_begins[i] == node_ends[i]) continue;

		if (uint32_t offset = (*node_begins[i])->offset; offset == min_offset) {
			out_stage_flags |= (*node_begins[i])->stageFlags;
			node_begins[i]++;
		}
	}

	return min_node;
}

static const ReflectionStructNode* merge_block_node(
	ReflectionContext&                  ctx,
	array_view<ReflectionStructNodePtr> src_nodes
) {
	const auto* first_struct = src_nodes.front();
	auto*       new_struct   = construct_node<ReflectionStructNode>(ctx.memory);
	
	std::pmr::vector<ReflectionBlockNodePtr> member_nodes(ctx.tempMemory);

	PerStageBlockNodeIteratorArray member_begins;
	PerStageBlockNodeIteratorArray member_ends;
	ShaderStageFlags               stage_flags;

	for (const auto* src : src_nodes) {
		member_begins.push_back(src->memberNodes.begin());
		member_ends.push_back(src->memberNodes.end());
	}

	while (const auto* min_node = find_minimum_block_node(member_begins, member_ends, stage_flags))
		member_nodes.push_back(clone_block_node(ctx, min_node));

	new_struct->type        = ReflectionNodeType::Struct;
	new_struct->stageFlags  = stage_flags;
	new_struct->name        = construct_string(ctx.memory, first_struct->name);
	new_struct->offset      = first_struct->offset;
	new_struct->paddedSize  = first_struct->paddedSize;
	new_struct->memberNodes = array_view{ member_nodes.data(), member_nodes.size() };
	new_struct->nameMap     = ReflectionNameMap(first_struct->nameMap, ctx.memory);

	return new_struct;
}

static const ReflectionDescriptorNode* merge_descriptor_node(
	ReflectionContext&                      ctx,
	array_view<ReflectionDescriptorNodePtr> src_nodes,
	ShaderStageFlags                        stage_flags
) {
	switch (src_nodes.front()->type) {
	case ReflectionNodeType::Descriptor: {
		const auto* first_desc = src_nodes.front();
		auto*       new_desc   = construct_node<ReflectionDescriptorNode>(ctx.memory);

		new_desc->type           = ReflectionNodeType::Descriptor;
		new_desc->stageFlags     = stage_flags;
		new_desc->name           = construct_string(ctx.memory, first_desc->name);
		new_desc->descriptorType = first_desc->descriptorType;
		new_desc->set            = first_desc->set;
		new_desc->binding        = first_desc->binding;

		return new_desc;
	}
	case ReflectionNodeType::DescriptorArray: {
		const auto* first_array = static_cast<const ReflectionDescriptorArrayNode*>(src_nodes.front());
		auto*       new_array   = construct_node<ReflectionDescriptorArrayNode>(ctx.memory);

		PerStageDescriptorNodeArray element_nodes;

		for (const auto* src : src_nodes)
			element_nodes.push_back(src);

		new_array->type           = ReflectionNodeType::DescriptorArray;
		new_array->stageFlags     = stage_flags;
		new_array->name           = construct_string(ctx.memory, first_array->name);
		new_array->descriptorType = first_array->descriptorType;
		new_array->set            = first_array->set;
		new_array->binding        = first_array->binding;
		new_array->elementCount   = first_array->elementCount;
		new_array->elementNode    = merge_descriptor_node(ctx, element_nodes, stage_flags);
		new_array->stride         = first_array->stride;

		return new_array;
	}
	case ReflectionNodeType::DescriptorBlock: {
		const auto* first_block = static_cast<const ReflectionDescriptorBlockNode*>(src_nodes.front());
		auto*       new_block   = construct_node<ReflectionDescriptorBlockNode>(ctx.memory);

		PerStageStructNodeArray block_nodes;

		for (const auto* src : src_nodes)
			block_nodes.push_back(src->as<ReflectionDescriptorBlockNode>()->block);

		new_block->type           = ReflectionNodeType::DescriptorBlock;
		new_block->stageFlags     = stage_flags;
		new_block->name           = construct_string(ctx.memory, first_block->name);
		new_block->descriptorType = first_block->descriptorType;
		new_block->set            = first_block->set;
		new_block->binding        = first_block->binding;
		new_block->block          = merge_block_node(ctx, block_nodes);

		return new_block;
	}
	}

	VERA_ERROR_MSG("invalid reflection type");
}

static bool check_block_node_compatible(
	const ReflectionBlockNode* lhs,
	const ReflectionBlockNode* rhs
) {
	if (lhs == rhs) return true;

	auto lhs_type = lhs->getType();
	auto rhs_type = rhs->getType();

	if (lhs_type != rhs_type || strcmp(lhs->name, rhs->name) != 0)
		return false;

	switch (lhs_type) {
	case ReflectionNodeType::Struct: {
		auto lhs_struct  = static_cast<const ReflectionStructNode*>(lhs);
		auto rhs_struct  = static_cast<const ReflectionStructNode*>(rhs);
		auto lhs_members = lhs_struct->memberNodes;
		auto rhs_members = rhs_struct->memberNodes;

		if (lhs_struct->offset != rhs_struct->offset ||
			lhs_struct->paddedSize != rhs_struct->paddedSize ||
			lhs_members.size() != rhs_members.size())
			return false;

		for (size_t i = 0; i < lhs_members.size(); ++i)
			if (!check_block_node_compatible(lhs_members[i], rhs_members[i]))
				return false;

		return true;
	}
	case ReflectionNodeType::Array: {
		auto lhs_array = static_cast<const ReflectionArrayNode*>(lhs);
		auto rhs_array = static_cast<const ReflectionArrayNode*>(rhs);

		return
			lhs_array->offset == rhs_array->offset &&
			lhs_array->paddedSize == rhs_array->paddedSize &&
			check_block_node_compatible(lhs_array->elementNode, rhs_array->elementNode) &&
			lhs_array->elementCount == rhs_array->elementCount &&
			lhs_array->stride == rhs_array->stride;
	}
	case ReflectionNodeType::Primitive: {
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

static bool check_block_compatible(
	const ReflectionStructNode* lhs,
	const ReflectionStructNode* rhs
) {
	auto lhs_first = lhs->memberNodes.begin();
	auto rhs_first = rhs->memberNodes.begin();
	auto lhs_last = lhs->memberNodes.end();
	auto rhs_last = rhs->memberNodes.end();

	while (lhs_first != lhs_last && rhs_first != rhs_last) {
		auto lhs_offset = (*lhs_first)->getOffset();
		auto rhs_offset = (*rhs_first)->getOffset();

		if (lhs_offset == rhs_offset) {
			if (!check_block_node_compatible(*lhs_first, *rhs_first))
				return false;
			++lhs_first;
			++rhs_first;
		} else if (lhs_offset < rhs_offset) {
			if (rhs_offset < lhs_offset + (*lhs_first)->getPaddedSize())
				return false;
			++lhs_first;
		} else {
			if (lhs_offset < rhs_offset + (*rhs_first)->getPaddedSize())
				return false;
			++rhs_first;
		}
	}
	
	return true;
}

static bool check_descriptor_node_compatible(
	const ReflectionDescriptorNode* lhs,
	const ReflectionDescriptorNode* rhs,
	bool                            check_block = false
) {
	if (lhs == rhs) return true;

	auto lhs_type = lhs->getType();
	auto rhs_type = rhs->getType();

	if (lhs_type != rhs_type) return false;

	VERA_ASSERT_MSG(lhs_type != ReflectionNodeType::Root,
		"cannot check compatibility on root node");

	switch (lhs_type) {
	case ReflectionNodeType::Descriptor: {
		const auto* lhs_desc = static_cast<const ReflectionDescriptorNode*>(lhs);
		const auto* rhs_desc = static_cast<const ReflectionDescriptorNode*>(rhs);

		return
			lhs_desc->descriptorType == rhs_desc->descriptorType &&
			lhs_desc->set == rhs_desc->set &&
			lhs_desc->binding == rhs_desc->binding;
	}
	case ReflectionNodeType::DescriptorArray: {
		const auto* lhs_array = static_cast<const ReflectionDescriptorArrayNode*>(lhs);
		const auto* rhs_array = static_cast<const ReflectionDescriptorArrayNode*>(rhs);

		return
			lhs_array->descriptorType == rhs_array->descriptorType &&
			lhs_array->set == rhs_array->set &&
			lhs_array->binding == rhs_array->binding &&
			check_descriptor_node_compatible(lhs_array->elementNode, rhs_array->elementNode, check_block) &&
			lhs_array->stride == rhs_array->stride;
	}
	case ReflectionNodeType::DescriptorBlock: {
		const auto* lhs_block = static_cast<const ReflectionDescriptorBlockNode*>(lhs);
		const auto* rhs_block = static_cast<const ReflectionDescriptorBlockNode*>(rhs);

		return
			lhs_block->descriptorType == rhs_block->descriptorType &&
			lhs_block->set == rhs_block->set &&
			lhs_block->binding == rhs_block->binding &&
			(!check_block || check_block_compatible(lhs_block->block, rhs_block->block));
	}
	}
	
	VERA_ERROR_MSG("invalid reflection type");
}

static bool find_minimum_descriptor_nodes(
	PerStageDescriptorNodeIteratorArray& desc_begins,
	PerStageDescriptorNodeIteratorArray& desc_ends,
	PerStageDescriptorNodeArray&         out_nodes,
	ShaderStageFlags&                    out_stage_flags
) {
	ReflectionDescriptorNodePtr min_node        = nullptr;
	uint64_t                    min_binding_key = UINT64_MAX;
	size_t                      stage_count     = desc_begins.size();

	out_nodes.clear();
	out_stage_flags = {};

	for (size_t i = 0; i < stage_count; ++i) {
		const auto it = desc_begins[i];
		
		if (it == desc_ends[i]) continue;

		uint64_t key = COMBINE_SET_BINDING((*it)->set, (*it)->binding);

		if (key < min_binding_key) {
			min_binding_key = key;
			min_node        = *it;
		}
	}

	if (!min_node) return false;

	for (size_t i = 0; i < stage_count; ++i) {
		const auto it = desc_begins[i];

		if (it == desc_ends[i]) continue;

		uint64_t key = COMBINE_SET_BINDING((*it)->set, (*it)->binding);

		if (key == min_binding_key) {
			out_nodes.push_back(*it);
			out_stage_flags |= (*it)->getStageFlags();
			desc_begins[i]++;
		}
	}

	return true;
}

static ReflectionRootNode* merge_impl(
	ReflectionContext&                    ctx,
	array_view<const ReflectionRootNode*> root_nodes,
	PerStageDescriptorNodeIteratorArray&  desc_begins,
	PerStageDescriptorNodeIteratorArray&  desc_ends,
	PerStagePushConstantNodeArray&        pc_nodes
) {
	static const auto sort_by_name =
		[](const ReflectionResourceNodePtr& lhs, const ReflectionResourceNodePtr& rhs) {
			return std::strcmp(lhs->name, rhs->name) < 0;
		};

	uint32_t entry_point_count = 0;
	uint32_t entry_point_idx   = 0;

	auto* root_node = construct_node<ReflectionRootNode>(ctx.memory);
	root_node->type              = ReflectionNodeType::Root;
	root_node->stageFlags        = ctx.stageFlags;
	root_node->targetFlags       = ReflectionTargetFlagBits::PipelineLayout;
	root_node->setCount          = 0;
	root_node->minSet            = UINT32_MAX;
	root_node->maxSet            = 0;
	root_node->descriptorCount   = 0;
	root_node->pushConstantCount = 0;

	PerStageDescriptorNodeArray                 min_nodes;
	ShaderStageFlags                            stage_flags;
	ReflectionNameMap                           name_map(ctx.tempMemory);
	std::pmr::vector<ReflectionResourceNodePtr> member_nodes(ctx.tempMemory);

	while (find_minimum_descriptor_nodes(desc_begins, desc_ends, min_nodes, stage_flags)) {
		std::sort(VERA_SPAN(min_nodes), sort_by_name);

		auto** first = min_nodes.begin();
		auto** last  = first + 1;
		auto** end   = min_nodes.end();
		auto*  prev  = *first;

		while (first != end) {
			while (last < end && strcmp((*first)->name, (*last)->name) == 0) last++;

			if (!check_descriptor_node_compatible(prev, *first))
				throw Exception("incompatible descriptor binding between stages "
					"'{}' and '{}' at set {}, binding {}",
					get_shader_stage_string(prev->stageFlags.flag_bit()),
					get_shader_stage_string((*first)->stageFlags.flag_bit()),
					prev->set,
					prev->binding);

			for (auto** it0 = first; it0 != last; ++it0) {
				for (auto** it1 = it0 + 1; it1 != last; ++it1) {
					if (!check_descriptor_node_compatible(*it0, *it1, true))
						throw Exception("incompatible descriptor binding between stages "
							"'{}' and '{}' at set {}, binding {}",
							get_shader_stage_string((*it0)->stageFlags.flag_bit()),
							get_shader_stage_string((*it1)->stageFlags.flag_bit()),
							(*it0)->set,
							(*it0)->binding);
				}
			}

			auto* new_node = merge_descriptor_node(
				ctx,
				array_view(first, static_cast<uint32_t>(last - first)),
				stage_flags);

			member_nodes.push_back(new_node);

			if (auto it = name_map.find(new_node->name); it == name_map.end()) {
				name_map.insert({ new_node->name, new_node });
			} else {
				throw Exception("name map collision with name: {}", new_node->name);
			}

			first = last++;
		}

		root_node->minSet = std::min(root_node->minSet, min_nodes.front()->set);
		root_node->maxSet = std::max(root_node->maxSet, min_nodes.front()->set);
		root_node->descriptorCount++;
	}

	if (root_node->minSet == UINT32_MAX && root_node->maxSet == 0) {
		root_node->maxSet = UINT32_MAX;
	} else {
		root_node->setCount = root_node->maxSet - root_node->minSet + 1;
	}

	if (!pc_nodes.empty()) {
		std::sort(VERA_SPAN(pc_nodes), sort_by_name);

		auto** first = pc_nodes.begin();
		auto** last  = first + 1;
		auto** end   = pc_nodes.end();

		while (first != end) {
			while (last < end && strcmp((*first)->name, (*last)->name) == 0) last++;

			for (auto** it0 = first; it0 != last; ++it0) {
				for (auto** it1 = it0 + 1; it1 != last; ++it1) {
					if (!check_block_node_compatible((*it0)->block, (*it1)->block))
						throw Exception("incompatible push constant block between stages '{}' and '{}' ",
							get_shader_stage_string((*it0)->stageFlags.flag_bit()),
							get_shader_stage_string((*it1)->stageFlags.flag_bit()));
				}
			}

			PerStageStructNodeArray block_nodes;
			ShaderStageFlags        stage_flags;

			for (auto** it = first; it != last; ++it) {
				block_nodes.push_back((*it)->block);
				stage_flags |= (*it)->stageFlags;
			}

			auto* new_node = construct_node<ReflectionPushConstantNode>(ctx.memory);
			new_node->type       = ReflectionNodeType::PushConstant;
			new_node->stageFlags = stage_flags;
			new_node->name       = construct_string(ctx.memory, (*first)->name);
			new_node->block      = merge_block_node(ctx, block_nodes);
			new_node->offset     = new_node->block->offset;
			new_node->paddedSize = new_node->block->paddedSize;

			member_nodes.push_back(new_node);

			if (auto it = name_map.find(new_node->name); it == name_map.end()) {
				name_map.insert({ new_node->name, new_node });
			} else {
				throw Exception("name map collision with name: {}", new_node->name);
			}

			root_node->pushConstantCount++;
		}
	}

	if (!member_nodes.empty()) {
		auto** root_members = construct_array<ReflectionResourceNodePtr>(ctx.memory, member_nodes.size());
		std::copy(VERA_SPAN(member_nodes), root_members);

		root_node->nameMap     = ReflectionNameMap(name_map, ctx.memory);
		root_node->memberNodes = array_view{ root_members, member_nodes.size() };
	}

	return root_node;
}

const ReflectionRootNode* ReflectionRootNode::create(
	const spv_reflect::ShaderModule& shader_module,
	std::pmr::memory_resource*       memory
) {
	std::pmr::monotonic_buffer_resource temp_memory(VERA_KIB(1));

	const SpvReflectShaderModule& module = shader_module.GetShaderModule();

	ReflectionContext ctx = {
		.memory     = memory,
		.tempMemory = &temp_memory,
		.stageFlags = to_shader_stage(module.shader_stage)
	};

	return parse_impl(ctx, module);
}

const ReflectionRootNode* ReflectionRootNode::merge(array_view<const ReflectionRootNode*> roots, std::pmr::memory_resource* memory)
{
	VERA_ASSERT_MSG(roots.size() <= MAX_SHADER_STAGE_COUNT,
		"exceeded maximum shader stage count for merging reflections");

	PerStageDescriptorNodeIteratorArray desc_begins;
	PerStageDescriptorNodeIteratorArray desc_ends;
	PerStagePushConstantNodeArray       pcs;
	ShaderStageFlags                    stage_flags;

	for (const auto* root_node : roots) {
		VERA_ASSERT(root_node);

		const auto target = root_node->targetFlags;

		if (target != ReflectionTargetFlagBits::DescriptorSetLayout &&
			target != ReflectionTargetFlagBits::Shader)
			throw Exception("only descriptor set layout and shader reflection can be merged");

		if (target == ReflectionTargetFlagBits::Shader) {
			if (stage_flags.has(root_node->stageFlags))
				throw Exception("duplicate shader stage in merging reflections");
			stage_flags |= root_node->stageFlags;
		}

		if (auto desc_view = root_node->enumerateDescriptor(); !desc_view.empty()) {
			desc_begins.push_back(desc_view.data());
			desc_ends.push_back(desc_view.data() + desc_view.size());
		}

		if (const auto* pc_node = root_node->findPushConstant())
			pcs.push_back(pc_node);
	}

	std::pmr::monotonic_buffer_resource temp_memory(VERA_KIB(1));

	ReflectionContext ctx = {
		.memory     = memory,
		.tempMemory = &temp_memory,
		.stageFlags = stage_flags
	};

	return merge_impl(ctx, roots, desc_begins, desc_ends, pcs);
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

VERA_NAMESPACE_END
