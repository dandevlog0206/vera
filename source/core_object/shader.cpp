#include "../../include/vera/core/shader.h"
#include "../spirv/reflection_desc.h"
#include "../impl/device_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/util/hash.h"
#include <fstream>

VERA_NAMESPACE_BEGIN

static size_t hash_shader_code(const uint32_t* spirv_code, size_t size_in_byte)
{
	size_t word_size = size_in_byte / sizeof(uint32_t);
	size_t seed      = 0;

	for (size_t i = 0; i < word_size; ++i)
		hash_combine(seed, spirv_code[i]);

	return seed;
}

const vk::ShaderModule& get_vk_shader_module(const_ref<Shader> shader) VERA_NOEXCEPT
{
	return CoreObject::getImpl(shader).shader;
}

vk::ShaderModule& get_vk_shader_module(ref<Shader> shader) VERA_NOEXCEPT
{
	return CoreObject::getImpl(shader).shader;
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

	auto obj   = create(std::move(device), reinterpret_cast<uint32_t*>(binary.data()), binary.size());
	auto& impl = getImpl(obj);

	impl.filePath = path;

	return obj;
}

obj<Shader> Shader::create(obj<Device> device, const uint32_t* spirv_code, size_t size_in_byte)
{
	VERA_ASSERT_MSG(size_in_byte % 4 == 0, "SPIR-V code size must be multiple of 4");

	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_shader_code(spirv_code, size_in_byte);

	if (auto it = device_impl.shaderCacheMap.find(hash_value);
		it != device_impl.shaderCacheMap.end()) {
		return unsafe_obj_cast<Shader>(it->second);
	}

	auto  obj  = createNewCoreObject<Shader>();
	auto& impl = getImpl(obj);

	auto striped = ReflectionDesc::stripReflectionInstructions(spirv_code, size_in_byte);

	vk::ShaderModuleCreateInfo shader_info;
	shader_info.codeSize = size_in_byte;
	shader_info.pCode    = reinterpret_cast<const uint32_t*>(spirv_code);

	impl.device    = std::move(device);
	impl.shader    = device_impl.device.createShaderModule(shader_info);
	impl.spirvCode.assign(VERA_SPAN(striped));
	impl.hashValue = hash_value;

	device_impl.shaderCacheMap.insert({ hash_value, obj });
	
	return obj;
}

Shader::~Shader()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.shaderCacheMap.erase(impl.hashValue);
	device_impl.device.destroy(impl.shader);

	destroyObjectImpl(this);
}

obj<Device> Shader::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

obj<ShaderReflection> Shader::getShaderReflection() VERA_NOEXCEPT
{
	return getImpl(this).getOrCreateShaderReflection();
}

size_t Shader::hash() const
{
	return getImpl(this).hashValue;
}

obj<ShaderReflection> ShaderImpl::getOrCreateShaderReflection() VERA_NOEXCEPT
{
	if (!shaderReflection) {
		shaderReflection = ShaderReflection::create(
			spirvCode.data(),
			spirvCode.size() * sizeof(uint32_t));

		// after creating reflection, clear spirv code to save memory
		spirvCode.clear();
	}

	return shaderReflection;
}

VERA_NAMESPACE_END