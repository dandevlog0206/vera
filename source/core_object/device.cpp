#include "../../include/vera/core/device.h"
#include "../impl/context_impl.h"
#include "../impl/device_impl.h"
#include "../impl/command_buffer_impl.h"

#include "../../include/vera/core/context.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/pipeline.h"
#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/core/program_reflection.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/util/static_vector.h"
#include <fstream>

#define MAX_EXTENSION_COUNT 128

#define FEATURE_INDEX(type) static_cast<size_t>(type)
#define ENABLE_FEATURE(enum_value) impl.enabledFeatures[static_cast<uint32_t>(enum_value)] = 1;
#define CHAIN_DEVICE_FEATURE(chain, type, name) \
	type name;                                  \
	name.pNext = chain;                         \
	chain      = &name;

#define CHAIN_STRUCT(chain, name) \
	name.pNext = chain;           \
	chain      = &name;

VERA_NAMESPACE_BEGIN

static MemoryHeapFlags to_memory_heap_flags(vk::MemoryHeapFlags flags)
{
	MemoryHeapFlags result;

	if (flags & vk::MemoryHeapFlagBits::eDeviceLocal)
		result |= MemoryHeapFlagBits::DeviceLocal;
	if (flags & vk::MemoryHeapFlagBits::eMultiInstance)
		result |= MemoryHeapFlagBits::MultiInstance;

	return result;
}

static MemoryPropertyFlags to_memory_property_flags(vk::MemoryPropertyFlags flags)
{
	MemoryPropertyFlags result;

	if (flags & vk::MemoryPropertyFlagBits::eDeviceLocal)
		result |= MemoryPropertyFlagBits::DeviceLocal;
	if (flags & vk::MemoryPropertyFlagBits::eHostVisible)
		result |= MemoryPropertyFlagBits::HostVisible;
	if (flags & vk::MemoryPropertyFlagBits::eHostCoherent)
		result |= MemoryPropertyFlagBits::HostCoherent;
	if (flags & vk::MemoryPropertyFlagBits::eHostCached)
		result |= MemoryPropertyFlagBits::HostCached;
	if (flags & vk::MemoryPropertyFlagBits::eLazilyAllocated)
		result |= MemoryPropertyFlagBits::LazilyAllocated;
	if (flags & vk::MemoryPropertyFlagBits::eProtected)
		result |= MemoryPropertyFlagBits::Protected;

	return result;
}

static void get_device_features(vk::PhysicalDevice physical_device, void* chain)
{
	vk::PhysicalDeviceFeatures2 device_features2;
	device_features2.pNext = chain;
	physical_device.getFeatures2(&device_features2);
}

const vk::Device& get_vk_device(const_ref<Device> device) VERA_NOEXCEPT
{
	return CoreObject::getImpl(device).vkDevice;
}

vk::Device& get_vk_device(ref<Device> device) VERA_NOEXCEPT
{
	return CoreObject::getImpl(device).vkDevice;
}

void DeviceFaultInfo::saveVendorBinaryToFile(std::string_view path) const
{
	if (vendorBinaryData.empty())
		throw Exception("no vendor binary data to save");

	std::ofstream file(path.data(), std::ios::binary);

	if (!file.is_open())
		throw Exception("failed to open file to save vendor binary data");

	file.write(reinterpret_cast<const char*>(vendorBinaryData.data()), vendorBinaryData.size());
	file.close();
}

obj<Device> Device::create(obj<Context> context, const DeviceCreateInfo& info)
{
	static_vector<const char*, MAX_EXTENSION_COUNT> device_layers;
	static_vector<const char*, MAX_EXTENSION_COUNT> device_extensions;

	auto  obj      = createNewCoreObject<Device>();
	auto& impl     = getImpl(obj);
	auto& ctx_impl = getImpl(context);

	impl.context                  = std::move(context);
	impl.graphicsQueueFamilyIndex = -1;
	impl.transferQueueFamilyIndex = -1;
	impl.computeQueueFamilyIndex  = -1;
	impl.pipelineCacheFilePath    = info.pipelineCacheFilePath;

	auto    physical_device  = ctx_impl.vkInstance.enumeratePhysicalDevices()[info.deviceID];
	auto    queue_families   = physical_device.getQueueFamilyProperties();
	int32_t graphics_family  = -1;
	int32_t compute_family   = -1;
	int32_t transfer_family  = -1;
	float   queue_priorities = 1.f;

	// Get Queue Family Indices
	for (size_t i = 0; i < queue_families.size(); ++i) {
		const auto& props = queue_families[i];
		if ((props.queueFlags & vk::QueueFlagBits::eGraphics) && graphics_family == -1)
			graphics_family = static_cast<int32_t>(i);
		if ((props.queueFlags & vk::QueueFlagBits::eCompute) && compute_family == -1)
			compute_family = static_cast<int32_t>(i);
		if ((props.queueFlags & vk::QueueFlagBits::eTransfer) && transfer_family == -1)
			transfer_family = static_cast<int32_t>(i);
	}

	if (graphics_family == -1)
		graphics_family = 0;
	if (compute_family == -1)
		compute_family = graphics_family;
	if (transfer_family == -1)
		transfer_family = graphics_family;

	static_vector<vk::DeviceQueueCreateInfo, 3> queue_create_infos;

	vk::DeviceQueueCreateInfo graphics_queue_info;
	graphics_queue_info.queueFamilyIndex = graphics_family;
	graphics_queue_info.queueCount       = 1;
	graphics_queue_info.pQueuePriorities = &queue_priorities;
	queue_create_infos.push_back(graphics_queue_info);

	if (compute_family != graphics_family) {
		vk::DeviceQueueCreateInfo compute_queue_info;
		compute_queue_info.queueFamilyIndex = compute_family;
		compute_queue_info.queueCount       = 1;
		compute_queue_info.pQueuePriorities = &queue_priorities;
		queue_create_infos.push_back(compute_queue_info);
	}

	if (transfer_family != graphics_family && transfer_family != compute_family) {
		vk::DeviceQueueCreateInfo transfer_queue_info;
		transfer_queue_info.queueFamilyIndex = transfer_family;
		transfer_queue_info.queueCount       = 1;
		transfer_queue_info.pQueuePriorities = &queue_priorities;
		queue_create_infos.push_back(transfer_queue_info);
	}

	// Enable device layers and extensions
	for (const auto& layer : info.deviceLayers)
		device_layers.push_back(layer.data());
	for (const auto& ext : info.deviceExtensions)
		device_extensions.push_back(ext.data());

	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
	device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
	device_extensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

	// Check physical device features
	void* curr_chain = nullptr;
	CHAIN_DEVICE_FEATURE(curr_chain, vk::PhysicalDeviceTimelineSemaphoreFeatures, timeline_semaphore)
	CHAIN_DEVICE_FEATURE(curr_chain, vk::PhysicalDeviceDynamicRenderingFeatures, dynamic_rendering)
	CHAIN_DEVICE_FEATURE(curr_chain, vk::PhysicalDeviceDescriptorIndexingFeatures, descriptor_indexing)
	CHAIN_DEVICE_FEATURE(curr_chain, vk::PhysicalDeviceMaintenance4Features, maintenance4_features)
	CHAIN_DEVICE_FEATURE(curr_chain, vk::PhysicalDeviceMeshShaderFeaturesEXT, mesh_shader_features)
	CHAIN_DEVICE_FEATURE(curr_chain, vk::PhysicalDeviceFaultFeaturesEXT, device_fault)

	get_device_features(physical_device, curr_chain);

	impl.enabledFeatures.resize(VERA_ENUM_COUNT(DeviceFeatureType), 0);

	if (timeline_semaphore.timelineSemaphore) {
		device_extensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
		ENABLE_FEATURE(DeviceFeatureType::TimelineSemaphore);
	}
	if (mesh_shader_features.meshShader) {
		device_extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
		mesh_shader_features.multiviewMeshShader                    = VK_FALSE;
		mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;
		mesh_shader_features.meshShaderQueries                      = VK_FALSE;
		if (mesh_shader_features.taskShader)
			ENABLE_FEATURE(DeviceFeatureType::TaskShader);
		ENABLE_FEATURE(DeviceFeatureType::MeshShader);
	}
	if (ctx_impl.enableDeviceFault && device_fault.deviceFault) {
		device_extensions.push_back(VK_EXT_DEVICE_FAULT_EXTENSION_NAME);
		ENABLE_FEATURE(DeviceFeatureType::DeviceFault);
	}

	if (!dynamic_rendering.dynamicRendering)
		throw Exception("dynamic rendering feature is not supported");
	if (!descriptor_indexing.runtimeDescriptorArray ||
		!descriptor_indexing.shaderSampledImageArrayNonUniformIndexing)
		throw Exception("descriptor indexing feature is not supported");

	vk::PhysicalDeviceFeatures device_features;
	device_features.geometryShader           = true;
	device_features.fragmentStoresAndAtomics = true;

	// Get physical device properties
	vk::PhysicalDeviceProperties2 device_props;
	device_props.pNext = &impl.vkDescriptorIndexingProperties;

	physical_device.getProperties2(&device_props);

	// Create logical device
	vk::DeviceCreateInfo device_info;
	device_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size());
	device_info.pQueueCreateInfos       = queue_create_infos.data();
	device_info.enabledLayerCount       = static_cast<uint32_t>(device_layers.size());
	device_info.ppEnabledLayerNames     = device_layers.data();
	device_info.enabledExtensionCount   = static_cast<uint32_t>(device_extensions.size());
	device_info.ppEnabledExtensionNames = device_extensions.data();
	device_info.pEnabledFeatures        = &device_features;
	device_info.pNext                   = curr_chain;

	impl.vkPhysicalDevice         = physical_device;
	impl.vkDeviceProperties       = physical_device.getProperties();
	impl.vkDeviceMemoryProperties = physical_device.getMemoryProperties();
	impl.vkDevice                 = physical_device.createDevice(device_info);
	impl.vkGraphicsQueue          = impl.vkDevice.getQueue(graphics_family, 0);
	impl.vkComputeQueue           = impl.vkDevice.getQueue(compute_family, 0);
	impl.vkTransferQueue          = impl.vkDevice.getQueue(transfer_family, 0);
	impl.vkSampleCount            = vk::SampleCountFlagBits::e1;

	impl.colorFormat              = info.colorFormat == Format::Unknown ? Format::RGBA8Unorm : info.colorFormat;
	impl.depthFormat              = info.depthFormat == Format::Unknown ? Format::D32Float : info.depthFormat;
	impl.graphicsQueueFamilyIndex = graphics_family;
	impl.computeQueueFamilyIndex  = compute_family;
	impl.transferQueueFamilyIndex = transfer_family;
	impl.pipelineCacheFilePath    = info.pipelineCacheFilePath;
	impl.defaultSampler           = Sampler::create(obj);

	if (info.enablePipelineCache) {
		std::vector<uint8_t>        binary;
		vk::PipelineCacheCreateInfo cache_info;

		if (!info.pipelineCacheFilePath.empty()) {
			std::ifstream file(info.pipelineCacheFilePath.data(), std::ios::binary | std::ios::ate);
			
			if (file.is_open()) {
				binary.resize(file.tellg());
				file.seekg(0, std::ios::beg);

				file.read(reinterpret_cast<char*>(binary.data()), binary.size());

				cache_info.initialDataSize = binary.size();
				cache_info.pInitialData    = binary.data();
			}
		}

		impl.vkPipelineCache = impl.vkDevice.createPipelineCache(cache_info);
	}

	for (uint32_t i = 0; i < impl.vkDeviceMemoryProperties.memoryTypeCount; ++i) {
		uint32_t heap_idx   = impl.vkDeviceMemoryProperties.memoryTypes[i].heapIndex;
		auto     flags      = impl.vkDeviceMemoryProperties.memoryTypes[i].propertyFlags;
		size_t   heap_size  = impl.vkDeviceMemoryProperties.memoryHeaps[heap_idx].size;
		auto     heap_flags = impl.vkDeviceMemoryProperties.memoryHeaps[heap_idx].flags;

		auto& prop = impl.memoryTypes.emplace_back();
		prop.heapID        = heap_idx;
		prop.size          = heap_size;
		prop.heapFlags     = to_memory_heap_flags(heap_flags);
		prop.propertyFlags = to_memory_property_flags(flags);
	}

	return obj;
}

Device::~Device() VERA_NOEXCEPT
{
	auto& impl     = getImpl(this);
	auto& ctx_impl = getImpl(impl.context);

	VERA_ASSERT_MSG(impl.shaderCache.empty(), "shader cache is not empty");
	VERA_ASSERT_MSG(impl.shaderReflectionCache.empty(), "shader reflection cache is not empty");
	VERA_ASSERT_MSG(impl.programReflectionCache.empty(), "program reflection cache is not empty");
	VERA_ASSERT_MSG(impl.descriptorSetLayoutCache.empty(), "descriptor set layout cache is not empty");
	VERA_ASSERT_MSG(impl.pipelineLayoutCache.empty(), "pipeline layout cache is not empty");
	VERA_ASSERT_MSG(impl.pipelineCache.empty(), "pipeline cache is not empty");
	VERA_ASSERT_MSG(impl.samplerCache.empty(), "sampler cache is not empty");

	if (impl.vkPipelineCache && !impl.pipelineCacheFilePath.empty()) {
		std::ofstream file(impl.pipelineCacheFilePath.data(), std::ios::binary);

		auto cache_data = impl.vkDevice.getPipelineCacheData(impl.vkPipelineCache);

		file.write(reinterpret_cast<const char*>(cache_data.data()), cache_data.size());
	}

	impl.vkDevice.waitIdle();

	impl.vkDevice.destroy(impl.vkPipelineCache);
	impl.vkDevice.destroy();

	destroyObjectImpl<Device>(this);
}

obj<Context> Device::getContext() VERA_NOEXCEPT
{
	return getImpl(this).context;
}

obj<Sampler> Device::getDefaultSampler() VERA_NOEXCEPT
{
	return getImpl(this).defaultSampler;
}

obj<Texture> Device::getDefaultTexture() VERA_NOEXCEPT
{
	// TODO: add default texture
	return nullptr;
}

obj<TextureView> Device::getDefaultTextureView() VERA_NOEXCEPT
{
	return nullptr;
}

obj<Buffer> Device::getDefaultBuffer() VERA_NOEXCEPT
{
	return nullptr;
}
obj<BufferView> Device::getDefaultBufferView() VERA_NOEXCEPT
{
	return nullptr;
}

array_view<DeviceMemoryType> Device::enumerateMemoryTypes() const
{
	return getImpl(this).memoryTypes;
}

DeviceFaultInfo Device::getDeviceFaultInfo() const
{
	auto& impl     = getImpl(this);
	auto& ctx_impl = getImpl(impl.context);

	if (!ctx_impl.enableDeviceFault)
		throw Exception("device fault info is not enabled");

	DeviceFaultInfo          result;
	vk::DeviceFaultCountsEXT fault_counts;
	vk::DeviceFaultInfoEXT   fault_info;

	if (impl.vkDevice.getFaultInfoEXT(&fault_counts, nullptr) != vk::Result::eSuccess)
		throw Exception("failed to get device fault counts");

	result.addressInfos.resize(fault_counts.addressInfoCount);
	result.vendorInfos.resize(fault_counts.vendorInfoCount);
	result.vendorBinaryData.resize(fault_counts.vendorBinarySize);

	fault_info.pAddressInfos     =
		reinterpret_cast<vk::DeviceFaultAddressInfoEXT*>(result.addressInfos.data());
	fault_info.pVendorInfos      =
		reinterpret_cast<vk::DeviceFaultVendorInfoEXT*>(result.vendorInfos.data());
	fault_info.pVendorBinaryData =
		reinterpret_cast<void*>(result.vendorBinaryData.data());

	if (impl.vkDevice.getFaultInfoEXT(&fault_counts, &fault_info) != vk::Result::eSuccess)
		throw Exception("failed to get device fault info");

	fault_info.pAddressInfos     = nullptr;
	fault_info.pVendorInfos      = nullptr;
	fault_info.pVendorBinaryData = nullptr;

	result.description.assign(
		fault_info.description.data(),
		fault_info.description.size());

	return result;
}

void Device::waitIdle() const
{
	getImpl(this).vkDevice.waitIdle();
}

bool DeviceImpl::isFeatureEnabled(DeviceFeatureType feature) const VERA_NOEXCEPT
{
	return enabledFeatures[FEATURE_INDEX(feature)] != 0;
}

uint32_t DeviceImpl::findMemoryTypeIndex(MemoryPropertyFlags flags, std::bitset<32> type_mask) VERA_NOEXCEPT
{
	for (uint32_t i = 0; i < memoryTypes.size(); ++i)
		if (type_mask[i] && memoryTypes[i].propertyFlags.has(flags))
			return i;
	
	VERA_ERROR_MSG("failed to find memory type index");
}

template<>
obj<Shader> DeviceImpl::findCachedObject<Shader>(hash_t hash_value)
{
	if (auto it = shaderCache.find(hash_value); it != shaderCache.end())
		return unsafe_obj_cast<Shader>(it->second);
	return {};
}

template<>
obj<ShaderReflection> DeviceImpl::findCachedObject<ShaderReflection>(hash_t hash_value)
{
	if (auto it = shaderReflectionCache.find(hash_value); it != shaderReflectionCache.end())
		return unsafe_obj_cast<ShaderReflection>(it->second);
	return {};
}

template<>
obj<ProgramReflection> DeviceImpl::findCachedObject<ProgramReflection>(hash_t hash_value)
{
	if (auto it = programReflectionCache.find(hash_value); it != programReflectionCache.end())
		return unsafe_obj_cast<ProgramReflection>(it->second);
	return {};
}

template<>
obj<DescriptorSetLayout> DeviceImpl::findCachedObject<DescriptorSetLayout>(hash_t hash_value)
{
	if (auto it = descriptorSetLayoutCache.find(hash_value); it != descriptorSetLayoutCache.end())
		return unsafe_obj_cast<DescriptorSetLayout>(it->second);
	return {};
}

template<>
obj<PipelineLayout> DeviceImpl::findCachedObject<PipelineLayout>(hash_t hash_value)
{
	if (auto it = pipelineLayoutCache.find(hash_value); it != pipelineLayoutCache.end())
		return unsafe_obj_cast<PipelineLayout>(it->second);
	return {};
}

template<>
obj<Pipeline> DeviceImpl::findCachedObject<Pipeline>(hash_t hash_value)
{
	if (auto it = pipelineCache.find(hash_value); it != pipelineCache.end())
		return unsafe_obj_cast<Pipeline>(it->second);
	return {};
}

template<>
obj<Sampler> DeviceImpl::findCachedObject<Sampler>(hash_t hash_value)
{
	if (auto it = samplerCache.find(hash_value); it != samplerCache.end())
		return unsafe_obj_cast<Sampler>(it->second);
	return {};
}

VERA_NAMESPACE_END
