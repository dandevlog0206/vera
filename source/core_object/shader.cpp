#include "../../include/vera/core/shader.h"
#include "../impl/device_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/resource_layout.h"
#include "../../include/vera/util/hash.h"
#include <spirv_reflect.h>
#include <fstream>

VERA_NAMESPACE_BEGIN

static ResourceType to_resource_type(SpvReflectDescriptorType type)
{
	switch (type) {
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
		return ResourceType::Sampler;
	case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		return ResourceType::CombinedImageSampler;
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		return ResourceType::SampledImage;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		return ResourceType::StorageImage;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		return ResourceType::UniformTexelBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		return ResourceType::StorageTexelBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		return ResourceType::UniformBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		return ResourceType::StorageBuffer;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		return ResourceType::UniformBufferDynamic;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return ResourceType::StorageBufferDynamic;
	case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		return ResourceType::InputAttachment;
	}

	VERA_ASSERT_MSG(false, "invalid resource type");
	return {};
}

static PipelineBindPoint get_pipeline_bind_point(SpvReflectShaderStageFlagBits stage)
{
	switch (stage) {
	case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
	case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
	case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
		return PipelineBindPoint::Graphics;
	case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
		return PipelineBindPoint::Compute;
	case SPV_REFLECT_SHADER_STAGE_TASK_BIT_EXT:
	case SPV_REFLECT_SHADER_STAGE_MESH_BIT_EXT:
	case SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR:
		return PipelineBindPoint::Unknown; // not supported yet
	}

	VERA_ASSERT_MSG(false, "unsupported shader stage for pipeline bind point");
	return {};
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
	}

	VERA_ASSERT_MSG(false, "invalid shader stage");
	return {};
}

static bool sort_reflection(const ReflectionRootMemberDesc* a, const ReflectionRootMemberDesc* b)
{
	bool pc_a = a->type == ReflectionType::PushConstant;
	bool pc_b = b->type == ReflectionType::PushConstant;

	VERA_ASSERT(!(pc_a && pc_b));

	if (pc_a || pc_b)
		return pc_b;

	const auto& res_a = static_cast<const ReflectionResourceDesc&>(*a);
	const auto& res_b = static_cast<const ReflectionResourceDesc&>(*b);

	if (res_a.set == res_b.set)
		return res_a.binding < res_b.binding;
	
	return res_a.set < res_b.set;
}

static bool sort_name_map(const ReflectionNameMap& a, const ReflectionNameMap& b)
{
	if (a.stageFlags == b.stageFlags)
		return static_cast<uint32_t>(a.stageFlags) < static_cast<uint32_t>(b.stageFlags);
	return std::strcmp(a.name, b.name) < 0;
}

static bool is_unsized_array(const SpvReflectBindingArrayTraits& traits)
{
	return 0 < traits.dims_count && traits.dims[0] <= 1;
}

static bool is_unsized_array(const SpvReflectArrayTraits& traits)
{
	return 0 < traits.dims_count && traits.dims[0] <= 1;
}

static uint32_t get_array_stride(const SpvReflectArrayTraits& traits, uint32_t dim)
{
	uint32_t result = traits.stride;

	for (uint32_t i = dim + 1; i < traits.dims_count; ++i)
		result *= traits.dims[i];

	return result;
}

static ReflectionPrimitiveType reflect_int_type(const SpvReflectTypeDescription& desc)
{
	if (desc.traits.numeric.scalar.signedness) {
		switch (desc.traits.numeric.scalar.width) {
		case 8:  return ReflectionPrimitiveType::Int8;
		case 16: return ReflectionPrimitiveType::Int16;
		case 32: return ReflectionPrimitiveType::Int32;
		case 64: return ReflectionPrimitiveType::Int64;
		}
	} else {
		switch (desc.traits.numeric.scalar.width) {
		case 8:  return ReflectionPrimitiveType::UInt8;
		case 16: return ReflectionPrimitiveType::UInt16;
		case 32: return ReflectionPrimitiveType::UInt32;
		case 64: return ReflectionPrimitiveType::UInt64;
		}
	}

	throw Exception("failed to reflect int type");
}

static ReflectionPrimitiveType reflect_float_type(const SpvReflectTypeDescription& desc)
{
	switch (desc.traits.numeric.scalar.width) {
	case 32: return ReflectionPrimitiveType::Float;
	case 64: return ReflectionPrimitiveType::Double;
	}

	throw Exception("failed to reflect float type");
}

static ReflectionPrimitiveType reflect_vector_type(const SpvReflectTypeDescription& desc)
{
	if (desc.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_FLOAT) {
		switch (desc.traits.numeric.scalar.width) {
		case 32:
			switch (desc.traits.numeric.vector.component_count) {
			case 2: return ReflectionPrimitiveType::Float_2;
			case 3: return ReflectionPrimitiveType::Float_3;
			case 4: return ReflectionPrimitiveType::Float_4;
			}
			break;
		case 64:
			switch (desc.traits.numeric.vector.component_count) {
			case 2: return ReflectionPrimitiveType::Double_2;
			case 3: return ReflectionPrimitiveType::Double_3;
			case 4: return ReflectionPrimitiveType::Double_4;
			}
			break;
		}
	} else if (desc.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_INT) {
		if (desc.traits.numeric.scalar.signedness) {
			switch (desc.traits.numeric.scalar.width) {
			case 8:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Int8_2;
				case 3: return ReflectionPrimitiveType::Int8_3;
				case 4: return ReflectionPrimitiveType::Int8_4;
				}
				break;
			case 16:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Int16_2;
				case 3: return ReflectionPrimitiveType::Int16_3;
				case 4: return ReflectionPrimitiveType::Int16_4;
				}
				break;
			case 32:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Int32_2;
				case 3: return ReflectionPrimitiveType::Int32_3;
				case 4: return ReflectionPrimitiveType::Int32_4;
				}
				break;
			case 64:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::Int64_2;
				case 3: return ReflectionPrimitiveType::Int64_3;
				case 4: return ReflectionPrimitiveType::Int64_4;
				}
				break;
			}
		} else {
			switch (desc.traits.numeric.scalar.width) {
			case 8:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UInt8_2;
				case 3: return ReflectionPrimitiveType::UInt8_3;
				case 4: return ReflectionPrimitiveType::UInt8_4;
				}
				break;
			case 16:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UInt16_2;
				case 3: return ReflectionPrimitiveType::UInt16_3;
				case 4: return ReflectionPrimitiveType::UInt16_4;
				}
				break;
			case 32:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UInt32_2;
				case 3: return ReflectionPrimitiveType::UInt32_3;
				case 4: return ReflectionPrimitiveType::UInt32_4;
				}
				break;
			case 64:
				switch (desc.traits.numeric.vector.component_count) {
				case 2: return ReflectionPrimitiveType::UInt64_2;
				case 3: return ReflectionPrimitiveType::UInt64_3;
				case 4: return ReflectionPrimitiveType::UInt64_4;
				}
				break;
			}
		}
	} else if (desc.type_flags & SpvReflectTypeFlagBits::SPV_REFLECT_TYPE_FLAG_BOOL) {
		switch (desc.traits.numeric.scalar.width) {
		case 2: return ReflectionPrimitiveType::Bool_2;
		case 3: return ReflectionPrimitiveType::Bool_3;
		case 4: return ReflectionPrimitiveType::Bool_4;
		}
	}

	throw Exception("failed to reflect vector type");
}

static ReflectionPrimitiveType reflect_matrix_type(const SpvReflectTypeDescription& desc)
{
	switch (desc.traits.numeric.scalar.width) {
	case 32:
		switch (desc.traits.numeric.matrix.row_count) {
		case 2:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Float_2x2;
			case 3: return ReflectionPrimitiveType::Float_2x3;
			case 4: return ReflectionPrimitiveType::Float_2x4;
			}
			break;
		case 3:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Float_3x2;
			case 3: return ReflectionPrimitiveType::Float_3x3;
			case 4: return ReflectionPrimitiveType::Float_3x4;
			}
			break;
		case 4:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Float_4x2;
			case 3: return ReflectionPrimitiveType::Float_4x3;
			case 4: return ReflectionPrimitiveType::Float_4x4;
			}
			break;
		};
		break;
	case 64:
		switch (desc.traits.numeric.matrix.row_count) {
		case 2:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Double_2x2;
			case 3: return ReflectionPrimitiveType::Double_2x3;
			case 4: return ReflectionPrimitiveType::Double_2x4;
			}
			break;
		case 3:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Double_3x2;
			case 3: return ReflectionPrimitiveType::Double_3x3;
			case 4: return ReflectionPrimitiveType::Double_3x4;
			}
			break;
		case 4:
			switch (desc.traits.numeric.matrix.column_count) {
			case 2: return ReflectionPrimitiveType::Double_4x2;
			case 3: return ReflectionPrimitiveType::Double_4x3;
			case 4: return ReflectionPrimitiveType::Double_4x4;
			}
			break;
		};
		break;
	}

	throw Exception("failed to reflect matrix type");
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

static ReflectionBlockDesc* reflect_block_variable(
	ShaderImpl&                    impl,
	const SpvReflectBlockVariable& block,
	uint32_t                       array_dim = 0
) {
	if (block.array.dims_count != array_dim) {
		auto* result = new ReflectionArrayDesc;
		result->type          = ReflectionType::Array;
		result->offset        = block.offset;
		result->stride        = get_array_stride(block.array, array_dim);
		result->elementCount = block.array.dims[array_dim] == 1 ? UINT32_MAX : block.array.dims[array_dim];
		result->element       = reflect_block_variable(impl, block, array_dim + 1);

		return result;
	}

	if (block.member_count) {
		auto* result = new ReflectionStructDesc;
		result->type         = ReflectionType::Struct;
		result->offset       = block.offset;
		result->memberCount  = block.member_count;
		result->members      = new ReflectionBlockDesc*[block.member_count];
		result->nameMapCount = block.member_count;
		result->nameMaps     = new ReflectionNameMap[result->nameMapCount];

		for (uint32_t i = 0; i < block.member_count; ++i) {
			result->members[i]  = reflect_block_variable(impl, block.members[i]);
			result->nameMaps[i] = ReflectionNameMap{
				.name       = impl.namePool << block.members[i].name,
				.stageFlags = impl.stageFlags,
				.index      = i
			};
		}

		std::sort(VERA_SPAN_ARRAY(result->nameMaps, result->nameMapCount), sort_name_map);

		return result;
	}

	auto* result = new ReflectionPrimitiveDesc;
	result->type          = ReflectionType::Primitive;
	result->offset        = block.offset;
	result->primitiveType = reflect_primitive_type(*block.type_description);

	return result;
}

static ReflectionRootMemberDesc* reflect_resource(
	ShaderImpl&                        impl,
	const SpvReflectDescriptorBinding& binding,
	uint32_t                           reflection_idx,
	uint32_t                           array_dim
) {
	if (1 < binding.array.dims_count)
		throw Exception("dimension of resource array must be less than 2");

	if (binding.array.dims_count != array_dim) {
		auto* result  = new ReflectionResourceArrayDesc;
		auto* element = reflect_resource(impl, binding, reflection_idx, array_dim + 1);

		result->type            = ReflectionType::ResourceArray;
		result->stageFlags      = impl.stageFlags;
		result->reflectionIndex = reflection_idx;
		result->resourceLayout  = nullptr;
		result->resourceType    = to_resource_type(binding.descriptor_type);
		result->set             = binding.set;
		result->binding         = binding.binding;
		result->elementCount    = is_unsized_array(binding.array) ? UINT32_MAX : binding.array.dims[0];
		result->element         = static_cast<ReflectionResourceDesc*>(element);

		return result;
	}

	if (binding.block.name) {
		auto* result = new ReflectionResourceBlockDesc;

		result->type            = ReflectionType::ResourceBlock;
		result->stageFlags      = impl.stageFlags;
		result->reflectionIndex = reflection_idx;
		result->resourceLayout  = nullptr;
		result->resourceType    = to_resource_type(binding.descriptor_type);
		result->set             = binding.set;
		result->binding         = binding.binding;
		result->sizeInByte      = binding.block.padded_size;
		result->memberCount     = binding.block.member_count;
		result->members         = new ReflectionBlockDesc*[binding.block.member_count];
		result->nameMapCount    = binding.block.member_count;
		result->nameMaps        = new ReflectionNameMap[binding.block.member_count];

		for (uint32_t i = 0; i < binding.block.member_count; ++i) {
			if (i != binding.block.member_count - 1 && is_unsized_array(binding.block.array))
				throw Exception("only last member of block can be unsized array");

			result->members[i]  = reflect_block_variable(impl, binding.block.members[i]);
			result->nameMaps[i] = ReflectionNameMap{
				.name       = impl.namePool << binding.block.members[i].name,
				.stageFlags = impl.stageFlags,
				.index      = i
			};
		}

		std::sort(VERA_SPAN_ARRAY(result->nameMaps, result->nameMapCount), sort_name_map);

		return result;
	}

	auto* result = new ReflectionResourceDesc;

	result->type            = ReflectionType::Resource;
	result->stageFlags      = impl.stageFlags;
	result->reflectionIndex = reflection_idx;
	result->resourceLayout  = nullptr;
	result->resourceType    = to_resource_type(binding.descriptor_type);
	result->set             = binding.set;
	result->binding         = binding.binding;

	return result;
}

static ReflectionRootMemberDesc* reflect_push_constant(
	ShaderImpl&                    impl,
	const SpvReflectBlockVariable& block,
	uint32_t                       reflection_idx
) {
	if (block.array.dims_count)
		throw Exception("push constant must not be an array");

	auto* result = new ReflectionPushConstantDesc{};
	result->type            = ReflectionType::PushConstant;
	result->stageFlags      = impl.stageFlags;
	result->reflectionIndex = reflection_idx;
	result->sizeInByte      = block.padded_size;
	result->memberCount     = block.member_count;
	result->members         = new ReflectionBlockDesc*[block.member_count];
	result->nameMapCount    = block.member_count;
	result->nameMaps        = new ReflectionNameMap[block.member_count];

	for (uint32_t i = 0; i < block.member_count; ++i) {
		result->members[i]  = reflect_block_variable(impl, block.members[i]);
		result->nameMaps[i] = ReflectionNameMap{
			.name       = impl.namePool << block.members[i].name,
			.stageFlags = impl.stageFlags,
			.index      = i
		};
	}

	std::sort(VERA_SPAN_ARRAY(result->nameMaps, result->nameMapCount), sort_name_map);

	return result;
}

static void parse_shader_reflection_info(ShaderImpl& impl, const uint32_t* spirv_code, size_t size_in_byte)
{
	SpvReflectShaderModule module;

	if (spvReflectCreateShaderModule(size_in_byte, spirv_code, &module) != SPV_REFLECT_RESULT_SUCCESS)
		throw Exception("failed to parse SPIR-V code");

	const auto desc_sets = array_view(module.descriptor_sets, module.descriptor_set_count);
	const auto pc_blocks = array_view(module.push_constant_blocks, module.push_constant_block_count);

	impl.reflection.reflectionCount   = module.descriptor_binding_count + module.push_constant_block_count;
	impl.reflection.reflections       = new ReflectionRootMemberDesc*[impl.reflection.reflectionCount];
	impl.reflection.nameMapCount      = impl.reflection.reflectionCount;
	impl.reflection.nameMaps          = new ReflectionNameMap[impl.reflection.nameMapCount];
	impl.reflection.resourceCount     = module.descriptor_binding_count;
	impl.reflection.pushConstantCount = module.push_constant_block_count;
	impl.reflection.maxSetCount       = 0;

	impl.entryPointName    = impl.namePool << module.entry_point_name;
	impl.stageFlags        = to_shader_stage(module.shader_stage);
	impl.pipelineBindPoint = get_pipeline_bind_point(module.shader_stage);

	uint32_t resource_idx = 0;

	for (const auto& set : desc_sets) {
		for (uint32_t i = 0; i < set.binding_count; ++i, ++resource_idx) {
			auto& binding = *set.bindings[i];

			if (binding.binding != i)
				throw Exception("binding numbers must be sequentially defined starting from 0");
			if (i != set.binding_count - 1 && is_unsized_array(binding.array))
				throw Exception("only last binding of descriptor set can be unsized array");

			impl.reflection.reflections[resource_idx] = reflect_resource(impl, binding, resource_idx, 0);
			impl.reflection.nameMaps[resource_idx]    = ReflectionNameMap{
				.name       = impl.namePool << binding.name,
				.stageFlags = impl.stageFlags,
				.index      = resource_idx
			};
		}

		impl.reflection.maxSetCount = std::max(impl.reflection.maxSetCount, set.set + 1);
	}

	if (1 < pc_blocks.size())
		throw Exception("only one push constant block is allowed");

	impl.pushConstantRanges.reserve(1);
	for (const auto& pc : pc_blocks) {
		auto& range = impl.pushConstantRanges.emplace_back();
		range.offset     = pc.offset;
		range.size       = pc.size;
		range.stageFlags = impl.stageFlags;

		impl.reflection.reflections[resource_idx] = reflect_push_constant(impl, pc, resource_idx);
		impl.reflection.nameMaps[resource_idx]    = ReflectionNameMap{
			.name       = impl.namePool << pc.name,
			.stageFlags = impl.stageFlags,
			.index      = resource_idx
		};

		++resource_idx;
	}

	// TODO: check if sort is needed
	std::sort(VERA_SPAN_ARRAY(impl.reflection.reflections, impl.reflection.reflectionCount), sort_reflection);
	std::sort(VERA_SPAN_ARRAY(impl.reflection.nameMaps, impl.reflection.nameMapCount), sort_name_map);

	spvReflectDestroyShaderModule(&module);
}

static size_t hash_shader_code(const uint32_t* spirv_code, size_t size_in_byte)
{
	size_t word_size = size_in_byte / sizeof(uint32_t);
	size_t seed      = 0;

	for (size_t i = 0; i < word_size; ++i)
		hash_combine(seed, spirv_code[i]);

	return seed;
}

obj<Shader> Shader::create(obj<Device> device, std::string_view path)
{
	std::vector<uint8_t> binary;
	std::ifstream        file(path.data(), std::ios_base::binary | std::ios_base::ate);

	if (!file.is_open())
		throw Exception("failed to open shader file: " + std::string(path));

	binary.resize(file.tellg());
	file.seekg(std::ios_base::beg);

	file.read(reinterpret_cast<char*>(binary.data()), binary.size());

	return create(std::move(device), reinterpret_cast<uint32_t*>(binary.data()), binary.size());
}

obj<Shader> Shader::create(obj<Device> device, const uint32_t* spirv_code, size_t size_in_byte)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_shader_code(spirv_code, size_in_byte);

	if (auto it = device_impl.shaderCacheMap.find(hash_value);
		it != device_impl.shaderCacheMap.end()) {
		return unsafe_obj_cast<Shader>(it->second);
	}

	auto  obj  = createNewCoreObject<Shader>();
	auto& impl = getImpl(obj);

	vk::ShaderModuleCreateInfo shader_info;
	shader_info.codeSize = size_in_byte;
	shader_info.pCode    = reinterpret_cast<const uint32_t*>(spirv_code);

	impl.device    = std::move(device);
	impl.shader    = device_impl.device.createShaderModule(shader_info);
	impl.hashValue = hash_value;

	parse_shader_reflection_info(impl, spirv_code, size_in_byte);

	device_impl.shaderCacheMap.insert({ hash_value, obj });
	
	return obj;
}

Shader::~Shader()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	destroy_shader_reflection(impl.reflection);

	device_impl.shaderCacheMap.erase(impl.hashValue);
	device_impl.device.destroy(impl.shader);

	destroyObjectImpl(this);
}

obj<Device> Shader::getDevice()
{
	auto& impl = getImpl(this);

	return impl.device;
}

ShaderStageFlags Shader::getShaderStageFlags() const
{
	return getImpl(this).stageFlags;
}

size_t Shader::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END
