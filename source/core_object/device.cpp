#include "../../include/vera/core/device.h"
#include "../impl/context_impl.h"
#include "../impl/device_impl.h"

#include "../../include/vera/core/context.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/pipeline.h"
#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/resource_layout.h"
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

static void clean_device_cache(DeviceImpl& impl)
{
	for (auto it = impl.pipelineMap.begin(); it != impl.pipelineMap.end();) {
		if (it->second.count() == 1)
			it = impl.pipelineMap.erase(it);
		else
			++it;
	}

	for (auto it = impl.shaderMap.begin(); it != impl.shaderMap.end();) {
		if (it->second.count() == 1)
			it = impl.shaderMap.erase(it);
		else
			++it;
	}

	for (auto it = impl.samplerMap.begin(); it != impl.samplerMap.end();) {
		if (it->second.count() == 1)
			it = impl.samplerMap.erase(it);
		else
			++it;
	}

	for (auto it = impl.pipelineLayoutMap.begin(); it != impl.pipelineLayoutMap.end();) {
		if (it->second.count() == 1)
			it = impl.pipelineLayoutMap.erase(it);
		else
			++it;
	}

	for (auto it = impl.resourceLayoutMap.begin(); it != impl.resourceLayoutMap.end();) {
		if (it->second.count() == 1)
			it = impl.resourceLayoutMap.erase(it);
		else
			++it;
	}
}

const vk::Device& get_vk_device(const_ref<Device> device)
{
	return CoreObject::getImpl(device).device;
}

vk::Device& get_vk_device(ref<Device> device)
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

	for (const auto& layer : info.deviceLayers)
		device_layers.push_back(layer.data());
	for (const auto& ext : info.deviceExtensions)
		device_extensions.push_back(ext.data());

	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	device_extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
	device_extensions.push_back("VK_EXT_extended_dynamic_state3");

	vk::PhysicalDeviceFeatures device_features;

	vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering;
	dynamic_rendering.dynamicRendering = true;

	vk::DeviceCreateInfo device_info;
	device_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size());
	device_info.pQueueCreateInfos       = queue_create_infos.data();
	device_info.enabledLayerCount       = static_cast<uint32_t>(device_layers.size());
	device_info.ppEnabledLayerNames     = device_layers.data();
	device_info.enabledExtensionCount   = static_cast<uint32_t>(device_extensions.size());
	device_info.ppEnabledExtensionNames = device_extensions.data();
	device_info.pEnabledFeatures        = &device_features;
	device_info.pNext                   = &dynamic_rendering;

	impl.physicalDeviceProperties = physical_device.getProperties();
	impl.deviceMemoryProperties   = physical_device.getMemoryProperties();
	impl.physicalDevice           = physical_device;
	impl.device                   = physical_device.createDevice(device_info);
	impl.graphicsQueue            = impl.device.getQueue(graphics_family, 0);
	impl.computeQueue             = impl.device.getQueue(compute_family, 0);
	impl.transferQueue            = impl.device.getQueue(transfer_family, 0);

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

	if (impl.pipelineCache && !impl.pipelineCacheFilePath.empty()) {
		std::ofstream file(impl.pipelineCacheFilePath.data(), std::ios::binary);

		auto cache_data = impl.device.getPipelineCacheData(impl.pipelineCache);

		file.write(reinterpret_cast<const char*>(cache_data.data()), cache_data.size());
	}

	impl.device.destroy(impl.pipelineCache);
	impl.device.destroy();

	destroyObjectImpl<Device>(this);
}

obj<Context> Device::getContext()
{
	return getImpl(this).context;
}

const std::vector<DeviceMemoryType>& Device::getMemoryTypes() const
{
	return getImpl(this).memoryTypes;
}

void Device::submitCommand(ref<CommandBuffer> command)
{
	auto& impl = getImpl(this);
	
	vk::SubmitInfo submit_info;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers    = &get_vk_command_buffer(command);

	impl.graphicsQueue.submit(submit_info, nullptr);
}

void Device::waitIdle() const
{
	getImpl(this).device.waitIdle();
}

VERA_NAMESPACE_END
