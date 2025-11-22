#include "../../include/vera/core/shader.h"
#include "../impl/device_impl.h"
#include "../impl/shader_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/util/hash.h"
#include "../spirv/spirv_reflect.h"
#include <spirv_cross/spirv_cross.hpp>
#include <fstream>
#include <chrono>

VERA_NAMESPACE_BEGIN

struct EntryPoint
{
	ShaderStageFlags stageFlag;
	std::string_view name;
};

static ShaderStageFlags get_shader_stage_flag(spv::ExecutionModel exec_model)
{
	switch (exec_model) {
	case spv::ExecutionModelVertex:
		return ShaderStageFlagBits::Vertex;
	case spv::ExecutionModelTessellationControl:
		return ShaderStageFlagBits::TessellationControl;
	case spv::ExecutionModelTessellationEvaluation:
		return ShaderStageFlagBits::TessellationEvaluation;
	case spv::ExecutionModelGeometry:
		return ShaderStageFlagBits::Geometry;
	case spv::ExecutionModelFragment:
		return ShaderStageFlagBits::Fragment;
	case spv::ExecutionModelGLCompute:
		return ShaderStageFlagBits::Compute;
	case spv::ExecutionModelTaskNV:
	case spv::ExecutionModelTaskEXT:
		return ShaderStageFlagBits::Task;
	case spv::ExecutionModelMeshNV:
	case spv::ExecutionModelMeshEXT:
		return ShaderStageFlagBits::Mesh;
	case spv::ExecutionModelRayGenerationKHR:
		return ShaderStageFlagBits::RayGen;
	case spv::ExecutionModelIntersectionKHR:
		return ShaderStageFlagBits::Intersection;
	case spv::ExecutionModelAnyHitKHR:
		return ShaderStageFlagBits::AnyHit;
	case spv::ExecutionModelClosestHitKHR:
		return ShaderStageFlagBits::ClosestHit;
	case spv::ExecutionModelMissKHR:
		return ShaderStageFlagBits::Miss;
	case spv::ExecutionModelCallableKHR:
		return ShaderStageFlagBits::Callable;
	default:
		throw Exception("unsupported shader execution model");
	}
}

static EntryPoint parse_entry_point(array_view<uint32_t> spirv_code)
{
	EntryPoint result;

	if (spirv_code[0] != spv::MagicNumber)
		throw Exception("invalid SPIR-V magic number");

	uint32_t word_count = static_cast<uint32_t>(spirv_code.size());
	uint32_t word_idx   = 5; // Skip header

	while (word_idx < word_count) {
		const uint32_t* inst = &spirv_code[word_idx];
		uint32_t        len  = (*inst) >> 16;
		spv::Op         op   = static_cast<spv::Op>((*inst) & 0xFFFF);

		if (op == spv::OpEntryPoint) {
			auto        exec_model       = static_cast<spv::ExecutionModel>(inst[1]);
			const auto* entry_point_name = reinterpret_cast<const char*>(&inst[3]);

			if (entry_point_name == nullptr)
				throw Exception("failed to find entry point name in SPIR-V code");

			result.stageFlag = get_shader_stage_flag(exec_model);
			result.name      = std::string_view(entry_point_name);

			return result;
		}

		if (len == 0)
			throw Exception("invalid SPIR-V instruction length");

		word_idx += len;
	}

	throw Exception("failed to find entry point in SPIR-V code");
}

static void create_shader_impl(
	obj<Device>           device,
	DeviceImpl&           device_impl,
	ShaderImpl&           impl,
	std::vector<uint32_t> spirv_code,
	hash_t                hash_value
) {
	EntryPoint entry_point = parse_entry_point(spirv_code);

	vk::ShaderModuleCreateInfo shader_info;
	shader_info.codeSize = spirv_code.size() * sizeof(uint32_t);
	shader_info.pCode    = spirv_code.data();

	spv_reflect::ShaderModule spv_module(shader_info.codeSize, shader_info.pCode);

	impl.device           = std::move(device);
	impl.vkShaderModule   = device_impl.vkDevice.createShaderModule(shader_info);
	impl.shaderReflection = ShaderReflection::create(impl.device, spirv_code);
	impl.spirvCode        = std::move(spirv_code);
	impl.entryPointName   = entry_point.name;
	impl.stageFlags       = entry_point.stageFlag;
	impl.hashValue        = hash_value;
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
	return CoreObject::getImpl(shader).vkShaderModule;
}

vk::ShaderModule& get_vk_shader_module(ref<Shader> shader) VERA_NOEXCEPT
{
	return CoreObject::getImpl(shader).vkShaderModule;
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

	if (auto cached_obj = device_impl.findCachedObject<Shader>(hash_value))
		return cached_obj;

	auto  obj  = createNewCoreObject<Shader>();
	auto& impl = getImpl(obj);

	create_shader_impl(
		std::move(device),
		device_impl,
		impl,
		std::move(spirv_code),
		hash_value);

	device_impl.registerCachedObject<Shader>(hash_value, obj);
	
	return obj;
}

obj<Shader> Shader::create(obj<Device> device, array_view<uint32_t> spirv_code)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_shader_code(spirv_code);

	if (auto cached_obj = device_impl.findCachedObject<Shader>(hash_value))
		return cached_obj;

	auto  obj  = createNewCoreObject<Shader>();
	auto& impl = getImpl(obj);

	create_shader_impl(
		std::move(device),
		device_impl,
		impl,
		std::vector<uint32_t>(spirv_code.begin(), spirv_code.end()),
		hash_value);

	device_impl.registerCachedObject<Shader>(hash_value, obj);
	
	return obj;
}

Shader::~Shader() VERA_NOEXCEPT
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterCachedObject<Shader>(impl.hashValue);
	device_impl.vkDevice.destroy(impl.vkShaderModule);

	destroyObjectImpl(this);
}

obj<Device> Shader::getDevice() const VERA_NOEXCEPT
{
	return getImpl(this).device;
}

ShaderStageFlags Shader::getStageFlags() const VERA_NOEXCEPT
{
	return getImpl(this).stageFlags;
}

std::string_view Shader::getEntryPointName() const VERA_NOEXCEPT
{
	return getImpl(this).entryPointName;
}

size_t Shader::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END