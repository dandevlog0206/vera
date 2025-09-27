#include "../../include/vera/core/shader.h"
#include "../impl/device_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
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
}

static bool is_unsized_array(const SpvReflectArrayTraits& traits)
{
	return 0 < traits.dims_count && traits.dims[0] == 1;
}

static uint32_t get_array_stride(const SpvReflectArrayTraits& traits, uint32_t dim)
{
	uint32_t result = traits.stride;

	for (uint32_t i = dim + 1; i < traits.dims_count; ++i)
		result *= traits.dims[i];

	return result;
}

static void sort_member_by_name(ReflectionDesc** desc_arr, uint32_t count)
{
	auto* first = desc_arr;
	auto* last  = first + count;
	
	std::sort(first, last, [](const ReflectionDesc* lhs, const ReflectionDesc* rhs) {
		return strcmp(lhs->name, rhs->name) < 0;
	});
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

static ReflectionDesc* reflect_block_variable(ShaderImpl& impl, const SpvReflectBlockVariable& block, uint32_t array_dim = 0)
{
	if (block.array.dims_count != array_dim) {
		auto* result = new ReflectionArrayDesc{};
		result->type          = ReflectionType::Array;
		result->name          = impl.namePool.insert(block.name).first->c_str();
		result->stride        = get_array_stride(block.array, array_dim);
		result->elementCount = block.array.dims[array_dim] == 1 ? UINT32_MAX : block.array.dims[array_dim];
		result->element       = reflect_block_variable(impl, block, array_dim + 1);
		result->offset        = block.offset;

		return result;
	}

	if (block.member_count) {
		auto* result = new ReflectionStructDesc{};
		result->type        = ReflectionType::Struct;
		result->name        = impl.namePool.insert(block.name).first->c_str();
		result->memberCount = block.member_count;
		result->members     = new ReflectionDesc*[block.member_count];
		result->offset      = block.offset;

		for (uint32_t i = 0; i < block.member_count; ++i)
			result->members[i] = reflect_block_variable(impl, block.members[i]);

		sort_member_by_name(result->members, result->memberCount);

		return result;
	}

	auto* result = new ReflectionPrimitiveDesc{};
	result->type          = ReflectionType::Primitive;
	result->name          = impl.namePool.insert(block.name).first->c_str();
	result->primitiveType = reflect_primitive_type(*block.type_description);
	result->offset        = block.offset;

	return result;
}

static ReflectionDesc* reflect_resource(ShaderImpl& impl, const SpvReflectDescriptorBinding& binding, uint32_t array_dim = 0)
{
	if (1 < binding.array.dims_count)
		throw Exception("dimension of resource array must be less than 2");

	if (binding.array.dims_count != array_dim) {
		auto* result = new ReflectionResourceArrayDesc{};
		result->type             = ReflectionType::ResourceArray;
		result->name             = impl.namePool.insert(binding.name).first->c_str();
		result->shaderStageFlags = impl.shaderStageFlags;
		result->resourceType     = to_resource_type(binding.descriptor_type);
		result->set              = binding.set;
		result->binding          = binding.binding;
		result->elementCount     = binding.array.dims[0];
		result->element          = reflect_resource(impl, binding, array_dim + 1);
		
		return result;
	}

	if (binding.block.name) {
		auto* result = new ReflectionResourceBlockDesc{};
		result->type             = ReflectionType::ResourceBlock;
		result->name             = impl.namePool.insert(binding.name).first->c_str();
		result->shaderStageFlags = impl.shaderStageFlags;
		result->sizeInByte       = binding.block.padded_size;
		result->resourceType     = to_resource_type(binding.descriptor_type);
		result->set              = binding.set;
		result->binding          = binding.binding;
		result->memberCount      = binding.block.member_count;
		result->members          = new ReflectionDesc * [binding.block.member_count];

		for (uint32_t i = 0; i < binding.block.member_count; ++i) {
			if (i != binding.block.member_count - 1 && is_unsized_array(binding.block.array))
				throw Exception("only last member of block can be unsized array");

			result->members[i] = reflect_block_variable(impl, binding.block.members[i]);
		}
		return result;
	}

	auto* result = new ReflectionResourceDesc{};
	result->type             = ReflectionType::Resource;
	result->name             = impl.namePool.insert(binding.name).first->c_str();
	result->shaderStageFlags = impl.shaderStageFlags;
	result->resourceType     = to_resource_type(binding.descriptor_type);
	result->set              = binding.set;
	result->binding          = binding.binding;

	return result;
}

static ReflectionDesc* reflect_push_constant(ShaderImpl& impl, const SpvReflectBlockVariable& block)
{
	if (block.array.dims_count)
		throw Exception("push constant must not be an array");

	auto* result = new ReflectionPushConstantDesc{};
	result->type             = ReflectionType::PushConstant;
	result->name             = impl.namePool.insert(block.name).first->c_str();
	result->shaderStageFlags = impl.shaderStageFlags;
	result->sizeInByte       = block.padded_size;
	result->memberCount      = block.member_count;
	result->members          = new ReflectionDesc*[block.member_count];

	for (uint32_t i = 0; i < block.member_count; ++i)
		result->members[i] = reflect_block_variable(impl, block.members[i]);

	sort_member_by_name(result->members, result->memberCount);

	return result;
}

static void destroy_reflection_descriptor(ReflectionDesc* desc_ptr)
{
	switch (desc_ptr->type) {
	case ReflectionType::Array: {
		auto& desc = *static_cast<ReflectionArrayDesc*>(desc_ptr);

		destroy_reflection_descriptor(desc.element);
	} break;
	case ReflectionType::Struct: {
		auto& desc = *static_cast<ReflectionStructDesc*>(desc_ptr);

		for (uint32_t i = 0; i < desc.memberCount; ++i)
			destroy_reflection_descriptor(desc.members[i]);
		delete[] desc.members;
	} break;
	case ReflectionType::ResourceBlock: {
		auto& desc = *static_cast<ReflectionResourceBlockDesc*>(desc_ptr);

		for (uint32_t i = 0; i < desc.memberCount; ++i)
			destroy_reflection_descriptor(desc.members[i]);
		delete[] desc.members;
	} break;
	case ReflectionType::PushConstant: {
		auto& desc = *static_cast<ReflectionPushConstantDesc*>(desc_ptr);
	
		for (uint32_t i = 0; i < desc.memberCount; ++i)
			destroy_reflection_descriptor(desc.members[i]);
		delete[] desc.members;
	} break;
	case ReflectionType::ResourceArray: {
		auto& desc = *static_cast<ReflectionResourceArrayDesc*>(desc_ptr);
	
		destroy_reflection_descriptor(desc.element);
	} break;
	}

	delete desc_ptr;
}

static void parse_shader(ShaderImpl& impl, const uint32_t* spirv_code, size_t size_in_byte)
{
	spv_reflect::ShaderModule parser(size_in_byte, spirv_code);

	uint32_t desc_set_count;
	uint32_t pc_count;

	impl.entryPointName   = parser.GetEntryPointName();
	impl.shaderStageFlags = to_shader_stage(parser.GetShaderStage());

	std::vector<ResourceLayoutBinding> bindings;
	parser.EnumerateDescriptorSets(&desc_set_count, nullptr);
	for (uint32_t i = 0; i < desc_set_count; ++i) {
		auto& set         = *parser.GetDescriptorSet(i);
		auto  refl_offset = impl.reflections.size();

		bindings.clear();
		bindings.reserve(set.binding_count);

		for (uint32_t j = 0; j < set.binding_count; ++j) {
			auto& binding = *set.bindings[j];

			if (strlen(binding.name) == 0) continue;

			auto& layout_binding = bindings.emplace_back();
			layout_binding.binding       = binding.binding;
			layout_binding.resourceType  = to_resource_type(binding.descriptor_type);
			layout_binding.resourceCount = binding.count;
			layout_binding.stageFlags    = impl.shaderStageFlags;

			impl.reflections.push_back(reflect_resource(impl, binding));
		}

		auto resource_layout = ResourceLayout::create(impl.device, bindings);

		for (size_t i = refl_offset; i < impl.reflections.size(); ++i)
			impl.reflections[i]->resourceLayout = resource_layout;

		impl.resourceLayouts.push_back(resource_layout);
	}

	parser.EnumeratePushConstantBlocks(&pc_count, nullptr);
	impl.pushConstantRanges.reserve(pc_count);
	for (uint32_t i = 0; i < pc_count; ++i) {
		auto& pc = *parser.GetPushConstantBlock(i);
		
		auto& range = impl.pushConstantRanges.emplace_back();
		range.offset     = pc.offset;
		range.size       = pc.size;
		range.stageFlags = impl.shaderStageFlags;

		impl.reflections.push_back(reflect_push_constant(impl, pc));
	}
}

static size_t hash_shader_code(const uint32_t* spirv_code, size_t size_in_byte)
{
	size_t size = size_in_byte / 4;
	size_t seed = 0;

	for (size_t i = 0; i < size; ++i)
		hash_combine(seed, spirv_code[i]);

	return seed;
}

ref<Shader> Shader::create(ref<Device> device, std::string_view path)
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

ref<Shader> Shader::create(ref<Device> device, const uint32_t* spirv_code, size_t size_in_byte)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_shader_code(spirv_code, size_in_byte);

	if (auto it = device_impl.shaderMap.find(hash_value);
		it != device_impl.shaderMap.end()) {
		return it->second;
	}

	auto  obj  = createNewObject<Shader>();
	auto& impl = getImpl(obj);

	vk::ShaderModuleCreateInfo shader_info;
	shader_info.codeSize = size_in_byte;
	shader_info.pCode    = reinterpret_cast<const uint32_t*>(spirv_code);

	impl.device    = std::move(device);
	impl.shader    = device_impl.device.createShaderModule(shader_info);
	impl.hashValue = hash_value;

	parse_shader(impl, spirv_code, size_in_byte);

	return device_impl.shaderMap[hash_value] = obj;
}

Shader::~Shader()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	for (auto* desc : impl.reflections)
		destroy_reflection_descriptor(desc);

	device_impl.device.destroy(impl.shader);

	destroyObjectImpl(this);
}

ref<Device> Shader::getDevice()
{
	auto& impl = getImpl(this);

	return impl.device;
}

ShaderStageFlags Shader::getShaderStageFlags() const
{
	return getImpl(this).shaderStageFlags;
}

size_t Shader::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END
