#include "../../include/vera/core/shader_reflection.h"
#include "../impl/device_impl.h"
#include "../impl/shader_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/logger.h"
#include "../../include/vera/core/shader.h"
#include "../../include/vera/util/static_vector.h"
#include "../../include/vera/util/stopwatch.h"
#include "../spirv/spirv_reflect.h"

#define MAX_SHADER_STAGE_COUNT 16

VERA_NAMESPACE_BEGIN

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

static ReflectionDecorationFlags to_decoration_flags(SpvReflectDecorationFlags flags)
{
	ReflectionDecorationFlags result;

	if (flags & SPV_REFLECT_DECORATION_BLOCK)
		result |= ReflectionDecorationFlagBits::Block;
	if (flags & SPV_REFLECT_DECORATION_BUFFER_BLOCK)
		result |= ReflectionDecorationFlagBits::BufferBlock;
	if (flags & SPV_REFLECT_DECORATION_ROW_MAJOR)
		result |= ReflectionDecorationFlagBits::RowMajor;
	if (flags & SPV_REFLECT_DECORATION_COLUMN_MAJOR)
		result |= ReflectionDecorationFlagBits::ColMajor;
	if (flags & SPV_REFLECT_DECORATION_BUILT_IN)
		result |= ReflectionDecorationFlagBits::BuiltIn;
	if (flags & SPV_REFLECT_DECORATION_NOPERSPECTIVE)
		result |= ReflectionDecorationFlagBits::NoPerspective;
	if (flags & SPV_REFLECT_DECORATION_FLAT)
		result |= ReflectionDecorationFlagBits::Flat;
	if (flags & SPV_REFLECT_DECORATION_NON_WRITABLE)
		result |= ReflectionDecorationFlagBits::NonWritable;
	if (flags & SPV_REFLECT_DECORATION_RELAXED_PRECISION)
		result |= ReflectionDecorationFlagBits::RelaxedPrecision;
	if (flags & SPV_REFLECT_DECORATION_NON_READABLE)
		result |= ReflectionDecorationFlagBits::NonReadable;
	if (flags & SPV_REFLECT_DECORATION_PATCH)
		result |= ReflectionDecorationFlagBits::Patch;
	if (flags & SPV_REFLECT_DECORATION_PER_VERTEX)
		result |= ReflectionDecorationFlagBits::PerVertex;
	if (flags & SPV_REFLECT_DECORATION_PER_TASK)
		result |= ReflectionDecorationFlagBits::PerTask;
	if (flags & SPV_REFLECT_DECORATION_WEIGHT_TEXTURE)
		throw Exception("SPV_REFLECT_DECORATION_WEIGHT_TEXTURE is not supported");
	if (flags & SPV_REFLECT_DECORATION_BLOCK_MATCH_TEXTURE)
		throw Exception("SPV_REFLECT_DECORATION_BLOCK_MATCH_TEXTURE is not supported");

	return result;
}

static ReflectionBuiltIn to_builtin(int builtin)
{
	if (builtin >= 0 && builtin < static_cast<int>(ReflectionBuiltIn::InstanceIndex))
		return static_cast<ReflectionBuiltIn>(builtin + 1);
	return ReflectionBuiltIn::Unknown;
}

static Format to_format(SpvReflectFormat format)
{
	switch (format) {
	case SPV_REFLECT_FORMAT_R16_UINT:            return Format::R16Uint;
	case SPV_REFLECT_FORMAT_R16_SINT:            return Format::R16Sint;
	case SPV_REFLECT_FORMAT_R16_SFLOAT:          return Format::R16Float;
	case SPV_REFLECT_FORMAT_R16G16_UINT:         return Format::RG16Uint;
	case SPV_REFLECT_FORMAT_R16G16_SINT:         return Format::RG16Sint;
	case SPV_REFLECT_FORMAT_R16G16_SFLOAT:       return Format::RG16Float;
	case SPV_REFLECT_FORMAT_R16G16B16_UINT:      return Format::RGB16Uint;
	case SPV_REFLECT_FORMAT_R16G16B16_SINT:      return Format::RGB16Sint;
	case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:    return Format::RGB16Float;
	case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:   return Format::RGBA16Uint;
	case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:   return Format::RGBA16Sint;
	case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT: return Format::RGBA16Float;
	case SPV_REFLECT_FORMAT_R32_UINT:            return Format::R32Uint;
	case SPV_REFLECT_FORMAT_R32_SINT:            return Format::R32Sint;
	case SPV_REFLECT_FORMAT_R32_SFLOAT:          return Format::R32Float;
	case SPV_REFLECT_FORMAT_R32G32_UINT:         return Format::RG32Uint;
	case SPV_REFLECT_FORMAT_R32G32_SINT:         return Format::RG32Sint;
	case SPV_REFLECT_FORMAT_R32G32_SFLOAT:       return Format::RG32Float;
	case SPV_REFLECT_FORMAT_R32G32B32_UINT:      return Format::RGB32Uint;
	case SPV_REFLECT_FORMAT_R32G32B32_SINT:      return Format::RGB32Sint;
	case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:    return Format::RGB32Float;
	case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:   return Format::RGBA32Uint;
	case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:   return Format::RGBA32Sint;
	case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return Format::RGBA32Float;
	case SPV_REFLECT_FORMAT_R64_UINT:            return Format::R64Uint;
	case SPV_REFLECT_FORMAT_R64_SINT:            return Format::R64Sint;
	case SPV_REFLECT_FORMAT_R64_SFLOAT:          return Format::R64Float;
	case SPV_REFLECT_FORMAT_R64G64_UINT:         return Format::RG64Uint;
	case SPV_REFLECT_FORMAT_R64G64_SINT:         return Format::RG64Sint;
	case SPV_REFLECT_FORMAT_R64G64_SFLOAT:       return Format::RG64Float;
	case SPV_REFLECT_FORMAT_R64G64B64_UINT:      return Format::RGB64Uint;
	case SPV_REFLECT_FORMAT_R64G64B64_SINT:      return Format::RGB64Sint;
	case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:    return Format::RGB64Float;
	case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:   return Format::RGBA64Uint;
	case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:   return Format::RGBA64Sint;
	case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: return Format::RGBA64Float;
	}

	return Format::Unknown;
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
	case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
		return DescriptorType::AccelerationStructure;
	}
	return DescriptorType::Unknown;
}

static const ReflectionSpecConstant* find_spec_constant(
	array_view<ReflectionSpecConstant> spec_constants,
	uint32_t                           constant_id
) {
	for (const auto& spec_const : spec_constants)
		if (spec_const.constantId == constant_id)
			return &spec_const;
	return nullptr;
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

	return ReflectionPrimitiveType::Unknown;
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

	return ReflectionPrimitiveType::Unknown;
}

static ReflectionPrimitiveType reflect_float_type(const SpvReflectBlockVariable& block)
{
	switch (block.numeric.scalar.width) {
	case 32: return ReflectionPrimitiveType::Float;
	case 64: return ReflectionPrimitiveType::Double;
	}

	return ReflectionPrimitiveType::Unknown;
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

	return ReflectionPrimitiveType::Unknown;
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

	return ReflectionPrimitiveType::Unknown;
}

static ReflectionInterfaceIO to_interface_io(SpvStorageClass storage_class)
{
	switch (storage_class) {
	case 1: // SpvStorageClassInput
		return ReflectionInterfaceIO::Input;
	case 3: // SpvStorageClassOutput
		return ReflectionInterfaceIO::Output;
	}
	return ReflectionInterfaceIO::Unknown;
}

template <class T>
static T* allocate(
	ShaderReflectionImpl& impl,
	size_t                count = 1
) {
	auto* ptr = reinterpret_cast<T*>(impl.memory.allocate(sizeof(T) * count, alignof(T)));
	std::uninitialized_default_construct_n(ptr, count);
	return ptr;
}

static std::string_view copy_string(
	ShaderReflectionImpl& impl,
	const char*           str
) {
	if (!str)
		return std::string_view();

	if (size_t len = strlen(str); len > 0) {
		auto* ptr = reinterpret_cast<char*>(impl.memory.allocate(len + 1, alignof(char)));
		memcpy(ptr, str, len);
		ptr[len] = '\0';

		return std::string_view(ptr, len);
	}

	return std::string_view();
}

static Version get_spirv_version(array_view<uint32_t> spirv_code)
{
	if (spirv_code.size() < 5)
		return Version(0, 0, 0);
	
	return Version(
		(spirv_code[1] >> 16) & 0xFF,
		(spirv_code[1] >> 8) & 0xFF,
		0);
}

static ReflectionArrayTraits create_array_traits(
	ShaderReflectionImpl&        impl,
	const SpvReflectArrayTraits& traits
) {
	auto* new_dims        = allocate<uint32_t>(impl, traits.dims_count);
	auto* new_spec_consts = reinterpret_cast<const ReflectionSpecConstant**>(nullptr);
	auto* new_is_runtimes = reinterpret_cast<bool*>(nullptr);
	bool  has_spec_consts = false;
	bool  has_runtime     = false;

	for (uint32_t i = 0; i < traits.dims_count; ++i) {
		new_dims[i] = traits.dims[i];
		has_spec_consts |= traits.spec_constant_op_ids[i] != UINT32_MAX;
		has_runtime     |= traits.dims[i] == 0;
	}

	if (has_spec_consts) {
		new_spec_consts = allocate<const ReflectionSpecConstant*>(impl, traits.dims_count);

		for (uint32_t i = 0; i < traits.dims_count; ++i) {
			new_spec_consts[i] = nullptr;
			if (traits.spec_constant_op_ids[i] != UINT32_MAX)
				new_spec_consts[i] = find_spec_constant(
					impl.specConstants,
					traits.spec_constant_op_ids[i]);
		}
	}

	if (has_runtime) {
		new_is_runtimes = allocate<bool>(impl, traits.dims_count);
		for (uint32_t i = 0; i < traits.dims_count; ++i)
			new_is_runtimes[i] = traits.dims[i] == 0;
	}

	return ReflectionArrayTraits(
		traits.dims_count,
		traits.stride,
		new_dims
	);
}

static ReflectionArrayTraits create_array_traits(
	ShaderReflectionImpl&               impl,
	const SpvReflectBindingArrayTraits& traits
) {
	auto* new_dims        = allocate<uint32_t>(impl, traits.dims_count);
	auto* new_is_runtimes = reinterpret_cast<bool*>(nullptr);
	bool  has_runtime     = false;

	for (uint32_t i = 0; i < traits.dims_count; ++i) {
		new_dims[i] = traits.dims[i];
		has_runtime     |= traits.dims[i] == 0;
	}

	if (has_runtime) {
		new_is_runtimes = allocate<bool>(impl, traits.dims_count);
		for (uint32_t i = 0; i < traits.dims_count; ++i)
			new_is_runtimes[i] = traits.dims[i] == 0;
	}

	return ReflectionArrayTraits(
		traits.dims_count,
		1,
		new_dims,
		nullptr,
		new_is_runtimes
	);
}

static ReflectionInterfaceVariable* create_interface_variable(
	ShaderReflectionImpl&              impl,
	const SpvReflectInterfaceVariable& var
) {
	auto* new_var = allocate<ReflectionInterfaceVariable>(impl);
	new_var->stageFlags      = impl.stageFlags;
	new_var->name            = copy_string(impl, var.name);
	new_var->location        = var.location;
	new_var->component       = var.component;
	new_var->io              = to_interface_io(var.storage_class);
	new_var->semantic        = copy_string(impl, var.semantic);
	new_var->decorationFlags = to_decoration_flags(var.decoration_flags);
	new_var->builtIn         = to_builtin(var.built_in);
	new_var->format          = to_format(var.format);
	new_var->arrayTraits     = create_array_traits(impl, var.array);

	if (var.member_count > 0) {
		auto* member_vars = allocate<const ReflectionInterfaceVariable*>(impl, var.member_count);
		for (uint32_t i = 0; i < var.member_count; ++i)
			member_vars[i] = create_interface_variable(impl, var.members[i]);
		new_var->members = array_view<const ReflectionInterfaceVariable*>(member_vars, var.member_count);
	}

	return new_var;
}

static ReflectionBlockVariable* create_block_variable(
	ShaderReflectionImpl&          impl,
	const SpvReflectBlockVariable& var
) {
	if (var.type_description == nullptr)
		return nullptr;

	auto* new_var = allocate<ReflectionBlockVariable>(impl);
	new_var->stageFlags      = impl.stageFlags;
	new_var->name            = copy_string(impl, var.name);
	new_var->offset          = var.offset;
	new_var->absoluteOffset  = var.absolute_offset;
	new_var->size            = var.size;
	new_var->paddedSize      = var.padded_size;
	new_var->decorationFlags = to_decoration_flags(var.decoration_flags);
	new_var->primitiveType   = parse_primitive_type(var);
	new_var->arrayTraits     = create_array_traits(impl, var.array);

	if (var.member_count > 0) {
		auto* member_vars = allocate<const ReflectionBlockVariable*>(impl, var.member_count);
		for (uint32_t i = 0; i < var.member_count; ++i)
			member_vars[i] = create_block_variable(impl, var.members[i]);
		new_var->members = array_view<const ReflectionBlockVariable*>(member_vars, var.member_count);
	}

	return new_var;
}

static ReflectionDescriptorBinding* create_descriptor_binding(
	ShaderReflectionImpl&               impl,
	const SpvReflectDescriptorBinding&  binding
) {
	auto* new_binding = allocate<ReflectionDescriptorBinding>(impl);
	new_binding->stageFlags           = impl.stageFlags;
	new_binding->name                 = copy_string(impl, binding.name);
	new_binding->set                  = binding.set;
	new_binding->binding              = binding.binding;
	new_binding->inputAttachmentIndex = binding.input_attachment_index;
	new_binding->descriptorType       = to_descriptor_type(binding.descriptor_type);
	new_binding->decorationFlags      = to_decoration_flags(binding.decoration_flags);
	new_binding->arrayTraits          = create_array_traits(impl, binding.array);
	new_binding->block                = create_block_variable(impl, binding.block);
	new_binding->elementCount         = binding.count;
	new_binding->accessed             = binding.accessed;

	return new_binding;
}

static void create_shader_reflection_impl(ShaderReflectionImpl& impl, const SpvReflectShaderModule& module)
{
	if (module.entry_point_count > 1)
		throw Exception("multiple entry points are not supported");

	impl.stageFlags     = to_shader_stage(module.shader_stage);
	impl.entryPointName = copy_string(impl, module.entry_point_name);
	impl.localSize      = uint3(
		module.entry_points[0].local_size.x,
		module.entry_points[0].local_size.y,
		module.entry_points[0].local_size.z);

	if (module.spec_constant_count > 0) {
		auto* spec_constants = allocate<ReflectionSpecConstant>(impl, module.spec_constant_count);
		
		for (uint32_t i = 0; i < module.spec_constant_count; ++i) {
			auto& src = module.spec_constants[i];
			auto& dst = spec_constants[i];

			dst.name       = copy_string(impl, src.name);
			dst.constantId = src.constant_id;
		}

		impl.specConstants = array_view(spec_constants, module.spec_constant_count);
	}

	if (uint32_t var_count = module.interface_variable_count; var_count > 0) {
		auto* interface_vars = allocate<const ReflectionInterfaceVariable*>(impl, var_count);
		for (uint32_t i = 0; i < var_count; ++i)
			interface_vars[i] = create_interface_variable(impl, module.interface_variables[i]);
		impl.interfaceVariables = array_view(interface_vars, var_count);

		auto*    input_vars  = allocate<const ReflectionInterfaceVariable*>(impl, var_count);
		auto*    output_vars = allocate<const ReflectionInterfaceVariable*>(impl, var_count);
		uint32_t input_idx   = 0;
		uint32_t output_idx  = 0;

		for (const auto& var : impl.interfaceVariables) {
			if (var->io == ReflectionInterfaceIO::Input) {
				input_vars[input_idx++] = var;
			} else if (var->io == ReflectionInterfaceIO::Output) {
				output_vars[output_idx++] = var;
			}
		}

		impl.inputVariables  = array_view(input_vars, input_idx);
		impl.outputVariables = array_view(output_vars, output_idx);
	}

	if (uint32_t var_count = module.descriptor_binding_count; var_count > 0) {
		auto* descriptor_bindings = allocate<const ReflectionDescriptorBinding*>(impl, var_count);
		for (uint32_t i = 0; i < var_count; ++i)
			descriptor_bindings[i] = create_descriptor_binding(impl, module.descriptor_bindings[i]);
		impl.descriptorBindings = array_view(descriptor_bindings, var_count);

		auto*    desc_sets = allocate<const ReflectionDescriptorSet*>(impl, module.descriptor_set_count);
		uint32_t set_idx = 0;

		for (const auto& src : array_view(module.descriptor_sets, module.descriptor_set_count)) {
			auto*    bindings    = allocate<const ReflectionDescriptorBinding*>(impl, src.binding_count);
			uint32_t binding_idx = 0;

			for (auto* binding : impl.descriptorBindings) {
				if (binding->set == src.set) {
					bindings[binding_idx++] = binding;
				} else if (binding->set > src.set) {
					break; // early out
				}
			}

			auto* desc_set = allocate<ReflectionDescriptorSet>(impl);
			desc_set->stageFlags = impl.stageFlags;
			desc_set->set        = src.set;
			desc_set->bindings   = array_view(bindings, src.binding_count);

			desc_sets[set_idx++] = desc_set;
		}

		impl.descriptorSets = array_view(desc_sets, module.descriptor_set_count);
	}

	if (module.push_constant_block_count == 0) {
		impl.pushConstantBlock = nullptr;
	} else if (module.push_constant_block_count == 1) {
		impl.pushConstantBlock = create_block_variable(impl, module.push_constant_blocks[0]);
	} else if (module.push_constant_block_count > 1) {
		throw Exception("multiple push constant blocks are not supported");
	}
}

static size_t hash_shader_code(array_view<uint32_t> spirv_code)
{
	size_t seed = 0;

	for (uint32_t word : spirv_code)
		hash_combine(seed, word);

	return seed;
}

obj<ShaderReflection> ShaderReflection::create(obj<Device> device, const_ref<Shader> shader)
{
	auto& device_impl = getImpl(device);
	auto& shader_impl = getImpl(shader);
	auto  hash_value  = shader_impl.hashValue;

	// TODO: more specific exception
	if (device != shader_impl.device)
		throw Exception("device mismatch");

	if (auto cached_obj = device_impl.findCachedObject<ShaderReflection>(hash_value))
		return cached_obj;

	auto  obj  = createNewCoreObject<ShaderReflection>();
	auto& impl = getImpl(obj);

	StopWatch sw0, sw1;

	sw0.start();
	spv_reflect::ShaderModule module(
		shader_impl.spirvCode.size() * sizeof(uint32_t),
		shader_impl.spirvCode.data(),
		SPV_REFLECT_MODULE_FLAG_NO_COPY);
	sw0.stop();

	impl.device       = std::move(device);
	impl.spirvVersion = get_spirv_version(shader_impl.spirvCode);
	impl.rootNode     = ReflectionRootNode::create(module, &impl.memory);
	impl.hashValue    = hash_value;

	sw1.start();
	create_shader_reflection_impl(impl, module.GetShaderModule());
	sw1.stop();

	Logger::info(
		"ShaderReflection created: SPIR-V parsing time = {} ms, reflection creation time = {} ms",
		sw0.get_ms(),
		sw1.get_ms());

	device_impl.registerCachedObject<ShaderReflection>(hash_value, obj);

	return obj;
}

obj<ShaderReflection> ShaderReflection::create(obj<Device> device, array_view<uint32_t> spirv_code)
{
	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_shader_code(spirv_code);

	if (auto cached_obj = device_impl.findCachedObject<ShaderReflection>(hash_value))
		return cached_obj;

	auto  obj  = createNewCoreObject<ShaderReflection>();
	auto& impl = getImpl(obj);

	StopWatch sw0, sw1;

	sw0.start();
	spv_reflect::ShaderModule module(
		spirv_code.size() * sizeof(uint32_t),
		spirv_code.data(),
		SPV_REFLECT_MODULE_FLAG_NO_COPY);
	sw0.stop();

	impl.device       = std::move(device);
	impl.spirvVersion = get_spirv_version(spirv_code);
	impl.rootNode     = ReflectionRootNode::create(module, &impl.memory);
	impl.hashValue    = hash_value;

	sw1.start();
	create_shader_reflection_impl(impl, module.GetShaderModule());
	sw1.stop();

	Logger::info(
		"ShaderReflection created: SPIR-V parsing time = {} ms, reflection creation time = {} ms",
		sw0.get_ms(),
		sw1.get_ms());

	device_impl.registerCachedObject<ShaderReflection>(hash_value, obj);

	return obj;
}

ShaderReflection::~ShaderReflection() VERA_NOEXCEPT
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterCachedObject<ShaderReflection>(impl.hashValue);

	destroyObjectImpl(this);
}

Version ShaderReflection::getSpirvVersion() const VERA_NOEXCEPT
{
	return getImpl(this).spirvVersion;
}

ShaderStageFlags ShaderReflection::getStageFlags() const VERA_NOEXCEPT
{
	return getImpl(this).stageFlags;
}

std::string_view ShaderReflection::getEntryPointName() const VERA_NOEXCEPT
{
	return getImpl(this).entryPointName;
}

array_view<const ReflectionInterfaceVariable*> ShaderReflection::enumerateInterfaceVariables() const VERA_NOEXCEPT
{
	return getImpl(this).interfaceVariables;
}

array_view<const ReflectionInterfaceVariable*> ShaderReflection::enumerateInputVariables() const VERA_NOEXCEPT
{
	return getImpl(this).inputVariables;
}

array_view<const ReflectionInterfaceVariable*> ShaderReflection::enumerateOutputVariables() const VERA_NOEXCEPT
{
	return getImpl(this).outputVariables;
}

array_view<const ReflectionDescriptorBinding*> ShaderReflection::enumerateDescriptorBindings() const VERA_NOEXCEPT
{
	return getImpl(this).descriptorBindings;
}

array_view<const ReflectionDescriptorBinding*> ShaderReflection::enumerateDescriptorBindings(uint32_t set) const VERA_NOEXCEPT
{
	if (const auto* desc_set = getDescriptorSet(set))
		return desc_set->bindings;
	return {};
}

array_view<const ReflectionDescriptorSet*> ShaderReflection::enumerateDescriptorSets() const VERA_NOEXCEPT
{
	return getImpl(this).descriptorSets;
}

const ReflectionDescriptorBinding* ShaderReflection::getDescriptorBinding(uint32_t set, uint32_t binding) const VERA_NOEXCEPT
{
	if (const auto* desc_set = getDescriptorSet(set))
		for (const auto* desc_binding : desc_set->bindings)
			if (desc_binding->binding == binding)
				return desc_binding;
	return nullptr;
}

const ReflectionDescriptorSet* ShaderReflection::getDescriptorSet(uint32_t set) const VERA_NOEXCEPT
{
	for (const auto* desc_set : getImpl(this).descriptorSets)
		if (desc_set->set == set)
			return desc_set;
	return nullptr;
}

const ReflectionBlockVariable* ShaderReflection::getPushConstantBlock() const VERA_NOEXCEPT
{
	return getImpl(this).pushConstantBlock;
}

uint3 ShaderReflection::getLocalSize() const VERA_NOEXCEPT
{
	return getImpl(this).localSize;
}

array_view<ReflectionSpecConstant> ShaderReflection::enumerateSpecConstants() const VERA_NOEXCEPT
{
	return getImpl(this).specConstants;
}

hash_t ShaderReflection::hash() const VERA_NOEXCEPT
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END