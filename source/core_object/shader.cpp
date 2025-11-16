#include "../../include/vera/core/shader.h"
#include "../impl/device_impl.h"
#include "../impl/shader_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/util/hash.h"
#include <spirv_reflect.h>
#include <fstream>

VERA_NAMESPACE_BEGIN

class ShaderParserInfo
{
public:
	std::vector<uint32_t>  spirvCode;
	SpvReflectShaderModule reflectModule;
};

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

static uint32_t get_descriptor_count(const SpvReflectDescriptorBinding& binding)
{
	uint32_t count = 1;
	
	for (uint32_t i = 0; i < binding.array.dims_count; ++i) {
		if (binding.array.dims[i] == 1) // unsized array
			return UINT32_MAX;

		count *= binding.array.dims[i];
	}

	return count;
}

static void create_shader_impl(
	obj<Device>           device,
	DeviceImpl&           device_impl,
	ShaderImpl&           impl,
	std::vector<uint32_t> spirv_code,
	hash_t                hash_value
) {
	vk::ShaderModuleCreateInfo shader_info;
	shader_info.codeSize = static_cast<uint32_t>(spirv_code.size() * sizeof(uint32_t));
	shader_info.pCode    = spirv_code.data();

	impl.device           = std::move(device);
	impl.shaderLayout     = {};
	impl.shaderReflection = {};
	impl.shaderModule     = device_impl.device.createShaderModule(shader_info);
	impl.parserInfo       = std::make_unique<ShaderParserInfo>();
	impl.hashValue        = hash_value;

	impl.parserInfo->spirvCode.swap(spirv_code);
	
	SpvReflectResult result = spvReflectCreateShaderModule2(
		SPV_REFLECT_MODULE_FLAG_NO_COPY,
		impl.parserInfo->spirvCode.size() * sizeof(uint32_t),
		impl.parserInfo->spirvCode.data(),
		&impl.parserInfo->reflectModule);

	if (result != SPV_REFLECT_RESULT_SUCCESS)
		throw Exception("failed to create SPIR-V reflection module");
}

static size_t hash_shader_code(array_view<uint32_t> spirv_code)
{
	size_t seed = 0;

	for (uint32_t word : spirv_code)
		hash_combine(seed, word);

	return seed;
}

const vk::ShaderModule& get_vk_shader_module(const_ref<Shader> shader) VERA_NOEXCEPT
{
	return CoreObject::getImpl(shader).shaderModule;
}

vk::ShaderModule& get_vk_shader_module(ref<Shader> shader) VERA_NOEXCEPT
{
	return CoreObject::getImpl(shader).shaderModule;
}

obj<Shader> Shader::create(obj<Device> device, std::string_view path)
{
	std::vector<uint32_t> binary;
	std::ifstream         file(path.data(), std::ios_base::binary | std::ios_base::ate);

	if (!file.is_open())
		throw Exception("failed to open shader file: " + std::string(path));

	size_t file_size = static_cast<size_t>(file.tellg());

	file.seekg(std::ios_base::beg);

	if (file_size % 4 != 0)
		throw Exception("invalid SPIR-V shader file size: {}", path);

	binary.resize(file_size / 4);
	file.read(reinterpret_cast<char*>(binary.data()), file_size);

	return create(std::move(device), std::move(binary));
}

obj<Shader> Shader::create(obj<Device> device, std::vector<uint32_t>&& spirv_code)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_shader_code(spirv_code);

	if (auto it = device_impl.shaderCacheMap.find(hash_value);
		it != device_impl.shaderCacheMap.end()) {
		return unsafe_obj_cast<Shader>(it->second);
	}

	auto  obj  = createNewCoreObject<Shader>();
	auto& impl = getImpl(obj);

	create_shader_impl(
		std::move(device),
		device_impl,
		impl,
		std::move(spirv_code),
		hash_value);

	device_impl.shaderCacheMap.insert({ hash_value, obj });
	
	return obj;
}

obj<Shader> Shader::create(obj<Device> device, array_view<uint32_t> spirv_code)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_shader_code(spirv_code);

	if (auto it = device_impl.shaderCacheMap.find(hash_value);
		it != device_impl.shaderCacheMap.end()) {
		return unsafe_obj_cast<Shader>(it->second);
	}

	auto  obj  = createNewCoreObject<Shader>();
	auto& impl = getImpl(obj);

	create_shader_impl(
		std::move(device),
		device_impl,
		impl,
		std::vector<uint32_t>(spirv_code.begin(), spirv_code.end()),
		hash_value);

	device_impl.shaderCacheMap.insert({ hash_value, obj });
	
	return obj;
}

Shader::~Shader()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.shaderCacheMap.erase(impl.hashValue);
	device_impl.device.destroy(impl.shaderModule);

	destroyObjectImpl(this);
}

obj<Device> Shader::getDevice() const VERA_NOEXCEPT
{
	return getImpl(this).device;
}

obj<ShaderLayout> Shader::getShaderLayout() const VERA_NOEXCEPT
{
	auto& impl = getImpl(this);

	if (!impl.shaderLayout) {
		auto& module = impl.parserInfo->reflectModule;

		ShaderLayoutCreateInfo layout_info;
	
		layout_info.stage = to_shader_stage(module.shader_stage);

		for (const auto& binding : array_view(module.descriptor_bindings, module.descriptor_binding_count)) {
			auto& layout_binding = layout_info.bindings.emplace_back();
			layout_binding.set             = binding.set;
			layout_binding.binding         = binding.binding;
			layout_binding.descriptorType  = to_descriptor_type(binding.descriptor_type);
			layout_binding.descriptorCount = get_descriptor_count(binding);
		}

		if (module.push_constant_block_count > 0) {
			const auto& pc = module.push_constant_blocks[0];
			layout_info.pushConstantOffset = pc.offset;
			layout_info.pushConstantSize   = pc.padded_size;
		} else {
			layout_info.pushConstantOffset = 0;
			layout_info.pushConstantSize   = 0;
		}

		impl.shaderLayout = ShaderLayout::create(impl.device, layout_info);

		if (impl.shaderReflection)
			impl.parserInfo.reset();
	}

	return impl.shaderLayout;
}

size_t Shader::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END