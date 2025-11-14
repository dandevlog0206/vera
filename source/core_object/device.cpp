#include "../../include/vera/core/device.h"
#include "../impl/context_impl.h"
#include "../impl/device_impl.h"
#include "../impl/command_buffer_impl.h"

#include "../../include/vera/core/context.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/pipeline.h"
#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include <fstream>

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

const vk::Device& get_vk_device(const_ref<Device> device) VERA_NOEXCEPT
{
	return CoreObject::getImpl(device).device;
}

vk::Device& get_vk_device(ref<Device> device) VERA_NOEXCEPT
{
	return CoreObject::getImpl(device).device;
}

obj<Device> Device::create(obj<Context> context, const DeviceCreateInfo& info)
{
	std::vector<const char*> device_layers;
	std::vector<const char*> device_extensions;

	auto  obj      = createNewCoreObject<Device>();
	auto& impl     = getImpl(obj);
	auto& ctx_impl = getImpl(context);

	impl.context                  = std::move(context);
	impl.graphicsQueueFamilyIndex = -1;
	impl.transferQueueFamilyIndex = -1;
	impl.computeQueueFamilyIndex  = -1;
	impl.pipelineCacheFilePath    = info.pipelineCacheFilePath;

	auto    physical_device  = ctx_impl.instance.enumeratePhysicalDevices()[info.deviceID];
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

	std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;

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
	device_extensions.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);


	device_extensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
	device_extensions.push_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);
	vk::DeviceDiagnosticsConfigCreateInfoNV aftermath_info;
	aftermath_info.flags = 
		vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableAutomaticCheckpoints |
		vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableResourceTracking |
		vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableShaderDebugInfo |
		vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableShaderErrorReporting;

	// Check physical device features
	vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering;
	dynamic_rendering.pNext = &aftermath_info;
	vk::PhysicalDeviceDescriptorIndexingFeatures descriptor_indexing;
	descriptor_indexing.pNext = &dynamic_rendering;
	vk::PhysicalDeviceMaintenance4Features maintenance4_features;
	maintenance4_features.pNext = &descriptor_indexing;
	vk::PhysicalDeviceMeshShaderFeaturesEXT mesh_shader_features;
	mesh_shader_features.pNext = &maintenance4_features;
	vk::PhysicalDeviceFeatures2 device_features2;
	device_features2.pNext = &mesh_shader_features;

	physical_device.getFeatures2(&device_features2);

	mesh_shader_features.multiviewMeshShader                    = VK_FALSE;
	mesh_shader_features.primitiveFragmentShadingRateMeshShader = VK_FALSE;
	
	vk::PhysicalDeviceFeatures device_features;
	device_features.geometryShader           = true;
	device_features.fragmentStoresAndAtomics = true;

	if (!dynamic_rendering.dynamicRendering)
		throw Exception("dynamic rendering feature is not supported");
	if (!descriptor_indexing.runtimeDescriptorArray ||
		!descriptor_indexing.shaderSampledImageArrayNonUniformIndexing)
		throw Exception("descriptor indexing feature is not supported");

	// Get physical device properties
	vk::PhysicalDeviceProperties2 device_props;
	device_props.pNext = &impl.descriptorIndexingProperties;

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
	device_info.pNext                   = &mesh_shader_features;

	impl.physicalDevice         = physical_device;
	impl.deviceProperties       = physical_device.getProperties();
	impl.deviceMemoryProperties = physical_device.getMemoryProperties();
	impl.device                 = physical_device.createDevice(device_info);
	impl.graphicsQueue          = impl.device.getQueue(graphics_family, 0);
	impl.computeQueue           = impl.device.getQueue(compute_family, 0);
	impl.transferQueue          = impl.device.getQueue(transfer_family, 0);

	impl.sampleCount              = vk::SampleCountFlagBits::e1;
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

		impl.pipelineCache = impl.device.createPipelineCache(cache_info);
	}

	for (uint32_t i = 0; i < impl.deviceMemoryProperties.memoryTypeCount; ++i) {
		uint32_t heap_idx   = impl.deviceMemoryProperties.memoryTypes[i].heapIndex;
		auto     flags      = impl.deviceMemoryProperties.memoryTypes[i].propertyFlags;
		size_t   heap_size  = impl.deviceMemoryProperties.memoryHeaps[heap_idx].size;
		auto     heap_flags = impl.deviceMemoryProperties.memoryHeaps[heap_idx].flags;

		auto& prop = impl.memoryTypes.emplace_back();
		prop.heapID        = heap_idx;
		prop.size          = heap_size;
		prop.heapFlags     = to_memory_heap_flags(heap_flags);
		prop.propertyFlags = to_memory_property_flags(flags);
	}

	return obj;
}

Device::~Device()
{
	auto& impl     = getImpl(this);
	auto& ctx_impl = getImpl(impl.context);

	VERA_ASSERT_MSG(impl.shaderCacheMap.empty(), "shader cache is not empty");
	VERA_ASSERT_MSG(impl.descriptorSetLayoutCacheMap.empty(), "descriptor set layout cache is not empty");
	VERA_ASSERT_MSG(impl.pipelineLayoutCacheMap.empty(), "pipeline layout cache is not empty");
	VERA_ASSERT_MSG(impl.pipelineLayoutCacheMapWithShader.empty(), "pipeline layout with shader cache is not empty");
	VERA_ASSERT_MSG(impl.pipelineCacheMap.empty(), "pipeline cache is not empty");
	VERA_ASSERT_MSG(impl.samplerCacheMap.empty(), "sampler cache is not empty");

	if (impl.pipelineCache && !impl.pipelineCacheFilePath.empty()) {
		std::ofstream file(impl.pipelineCacheFilePath.data(), std::ios::binary);

		auto cache_data = impl.device.getPipelineCacheData(impl.pipelineCache);

		file.write(reinterpret_cast<const char*>(cache_data.data()), cache_data.size());
	}

	impl.device.waitIdle();

	impl.device.destroy(impl.pipelineCache);
	impl.device.destroy();

	destroyObjectImpl<Device>(this);
}

obj<Context> Device::getContext()
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

const std::vector<DeviceMemoryType>& Device::getMemoryTypes() const
{
	return getImpl(this).memoryTypes;
}

void Device::waitIdle() const
{
	getImpl(this).device.waitIdle();
}

uint32_t DeviceImpl::findMemoryTypeIndex(MemoryPropertyFlags flags, std::bitset<32> type_mask) VERA_NOEXCEPT
{
	for (uint32_t i = 0; i < memoryTypes.size(); ++i)
		if (type_mask[i] && memoryTypes[i].propertyFlags.has(flags))
			return i;
	
	VERA_ERROR_MSG("failed to find memory type index");
}

void DeviceImpl::registerShader(hash_t hash_value, ref<Shader> shader)
{
	VERA_ASSERT(hash_value != 0);

	if (!shaderCacheMap.emplace(hash_value, std::move(shader)).second)
		throw Exception("Failed to register shader: hash collision(hash= {:016x})", hash_value);
}

void DeviceImpl::registerDescriptorSetLayout(hash_t hash_value, ref<DescriptorSetLayout> descriptor_set_layout)
{
	VERA_ASSERT(hash_value != 0);

	if (!descriptorSetLayoutCacheMap.emplace(hash_value, std::move(descriptor_set_layout)).second)
		throw Exception("Failed to register descriptor set layout: hash collision(hash= {:016x})", hash_value);
}

void DeviceImpl::registerPipelineLayout(hash_t hash_value, ref<PipelineLayout> pipeline_layout)
{
	VERA_ASSERT(hash_value != 0);

	if (!pipelineLayoutCacheMap.emplace(hash_value, std::move(pipeline_layout)).second)
		throw Exception("Failed to register pipeline layout: hash collision(hash= {:016x})", hash_value);
}

void DeviceImpl::registerPipelineLayoutWithShaders(hash_t hash_value, ref<PipelineLayout> pipeline_layout)
{
	VERA_ASSERT(hash_value != 0);

	if (!pipelineLayoutCacheMapWithShader.emplace(hash_value, std::move(pipeline_layout)).second)
		throw Exception("Failed to register pipeline layout: hash collision(hash= {:016x})", hash_value);
}

void DeviceImpl::registerPipeline(hash_t hash_value, ref<Pipeline> pipeline)
{
	VERA_ASSERT(hash_value != 0);

	if (!pipelineCacheMap.emplace(hash_value, std::move(pipeline)).second)
		throw Exception("Failed to register pipeline: hash collision(hash= {:016x})", hash_value);
}

void DeviceImpl::registerSampler(hash_t hash_value, ref<Sampler> sampler)
{
	VERA_ASSERT(hash_value != 0);

	if (!samplerCacheMap.emplace(hash_value, std::move(sampler)).second)
		throw Exception("Failed to register sampler: hash collision(hash= {:016x})", hash_value);
}

void DeviceImpl::unregisterShader(hash_t hash_value) VERA_NOEXCEPT
{
	size_t erased = shaderCacheMap.erase(hash_value);
	VERA_ASSERT_MSG(erased == 1, "shader not found in cache");
	(void)erased;
}

void DeviceImpl::unregisterDescriptorSetLayout(hash_t hash_value) VERA_NOEXCEPT
{
	size_t erased = descriptorSetLayoutCacheMap.erase(hash_value);
	VERA_ASSERT_MSG(erased == 1, "descriptor set layout not found in cache");
	(void)erased;
}

void DeviceImpl::unregisterPipelineLayout(hash_t hash_value) VERA_NOEXCEPT
{
	if (hash_value) {
		size_t erased = pipelineLayoutCacheMap.erase(hash_value);
		VERA_ASSERT_MSG(erased == 1, "pipeline layout not found in cache");
		(void)erased;
	}
}

void DeviceImpl::unregisterPipelineLayoutWithShaders(hash_t hash_value) VERA_NOEXCEPT
{
	if (hash_value) {
		size_t erased = pipelineLayoutCacheMapWithShader.erase(hash_value);
		VERA_ASSERT_MSG(erased == 1, "pipeline layout with shaders not found in cache");
		(void)erased;
	}
}

void DeviceImpl::unregisterPipeline(hash_t hash_value) VERA_NOEXCEPT
{
	size_t erased = pipelineCacheMap.erase(hash_value);
	VERA_ASSERT_MSG(erased == 1, "pipeline not found in cache");
	(void)erased;
}

void DeviceImpl::unregisterSampler(hash_t hash_value) VERA_NOEXCEPT
{
	size_t erased = samplerCacheMap.erase(hash_value);
	VERA_ASSERT_MSG(erased == 1, "sampler not found in cache");
	(void)erased;
}

VERA_NAMESPACE_END
