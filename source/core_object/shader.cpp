#include "../../include/vera/core/shader.h"
#include "../impl/device_impl.h"
#include "../impl/shader_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/logger.h"
#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/util/hash.h"
#include <spirv_cross/spirv_cross.hpp>
#include <fstream>
#include <chrono>

VERA_NAMESPACE_BEGIN

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
	impl.vkShaderModule   = device_impl.vkDevice.createShaderModule(shader_info);
	impl.shaderReflection = {};
	impl.spirvCode        = std::move(spirv_code);
	impl.hashValue        = hash_value;

	auto begin0 = std::chrono::high_resolution_clock::now();

	spirv_cross::Compiler compiler(impl.spirvCode.data(), impl.spirvCode.size());

	auto end0 = std::chrono::high_resolution_clock::now();

	auto begin1 = std::chrono::high_resolution_clock::now();

	auto end1 = std::chrono::high_resolution_clock::now();

	auto resources = compiler.get_shader_resources();

	Logger::info(
		"Shader parsing time: {:.3f} ms (SPIRV-Cross: {:.3f} ms, SPIRV-Reflect: {:.3f} ms)",
		std::chrono::duration<double, std::milli>(end1 - begin0).count(),
		std::chrono::duration<double, std::milli>(end0 - begin0).count(),
		std::chrono::duration<double, std::milli>(end1 - begin1).count());
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

	if (auto it = device_impl.shaderCache.find(hash_value);
		it != device_impl.shaderCache.end()) {
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

	device_impl.registerShader(hash_value, obj);
	
	return obj;
}

obj<Shader> Shader::create(obj<Device> device, array_view<uint32_t> spirv_code)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_shader_code(spirv_code);

	if (auto it = device_impl.shaderCache.find(hash_value);
		it != device_impl.shaderCache.end()) {
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

	device_impl.registerShader(hash_value, obj);
	
	return obj;
}

Shader::~Shader()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterShader(impl.hashValue);
	device_impl.vkDevice.destroy(impl.vkShaderModule);

	destroyObjectImpl(this);
}

obj<Device> Shader::getDevice() const VERA_NOEXCEPT
{
	return getImpl(this).device;
}

size_t Shader::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END