#include "../../include/vera/core/context.h"
#include "../impl/context_impl.h"

#include "../../include/vera/core/logger.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

VERA_NAMESPACE_BEGIN

static uint32_t get_debug_level(vk::DebugUtilsMessageSeverityFlagBitsEXT flags)
{
	switch (flags) {
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:   return 0;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: return 1;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:    return 2;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: return 3;
	}

	throw Exception("unknown debug message severity");
}

static VKAPI_ATTR VkBool32 vk_debug_callback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT             messageTypes,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*                                         pUserData
) {
	auto& ctx  = *reinterpret_cast<ContextImpl*>(pUserData);
	auto& data = *pCallbackData;

	 if (get_debug_level(messageSeverity) < ctx.debugUtilsMessengerLevel)
	 	return VK_FALSE;

	switch (messageSeverity) {
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		Logger::error(data.pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		Logger::warn(data.pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
		Logger::info(data.pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
		Logger::verbose(data.pMessage);
		break;
	}

	vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

	return VK_FALSE;
}

vk::Instance& get_vk_instance(ref<Context> context)
{
	return CoreObject::getImpl(context).instance;
}

std::vector<InstanceLayerInfo> Context::enumerateInstanceLayers()
{
	std::vector<InstanceLayerInfo> result;

	for (const auto& prop : vk::enumerateInstanceLayerProperties())
		result.emplace_back(InstanceLayerInfo{
			.name                 = prop.layerName,
			.description          = prop.description,
			.implementaionVersion = prop.implementationVersion,
			.specVersion          = prop.specVersion
		});

	return result;
}

std::vector<InstanceExtensionInfo> Context::enumerateInstanceExtensions()
{
	std::vector<InstanceExtensionInfo> result;

	for (const auto& prop : vk::enumerateInstanceExtensionProperties())
		result.emplace_back(InstanceExtensionInfo{
			.name        = prop.extensionName,
			.specVersion = prop.specVersion
		});

	return result;
}

obj<Context> Context::create(const ContextCreateInfo& info)
{
	auto  obj  = createNewObject<Context>();
	auto& impl = getImpl(obj);

	vk::ApplicationInfo app_info;
	app_info.pApplicationName   = info.applicationName.data();
	app_info.applicationVersion = info.applicationVersion;
	app_info.pEngineName        = "vera";
	app_info.engineVersion      = VERA_VERSION;
	app_info.apiVersion         = VK_API_VERSION_1_4;

	std::vector<const char*> instance_layers;
	std::vector<const char*> instance_extensions;

	for (const auto& name : info.instanceLayers)
		instance_layers.push_back(name.data());
	for (const auto& name : info.instanceExtensions)
		instance_extensions.push_back(name.data());

	if (info.enableValidation)
		instance_layers.push_back("VK_LAYER_KHRONOS_validation");
	if (info.enableDebugUtils)
		instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instance_extensions.push_back("VK_KHR_win32_surface");

	VULKAN_HPP_DEFAULT_DISPATCHER.init();

	vk::InstanceCreateInfo instance_info;
	instance_info.pApplicationInfo        = &app_info;
	instance_info.enabledLayerCount       = static_cast<uint32_t>(instance_layers.size());
	instance_info.ppEnabledLayerNames     = instance_layers.data();
	instance_info.enabledExtensionCount   = static_cast<uint32_t>(instance_extensions.size());
	instance_info.ppEnabledExtensionNames = instance_extensions.data();

	impl.instance = vk::createInstance(instance_info);

	VULKAN_HPP_DEFAULT_DISPATCHER.init(impl.instance);

	if (info.enableDebugUtils) {
		vk::DebugUtilsMessengerCreateInfoEXT debug_info;
		debug_info.messageSeverity =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
		debug_info.messageType     =
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		debug_info.pfnUserCallback = vk_debug_callback;
		debug_info.pUserData       = &impl;

		impl.debugUtilsMessenger      = impl.instance.createDebugUtilsMessengerEXT(debug_info);
		impl.debugUtilsMessengerLevel = info.debugUtilsMessengerLevel;
	}
	
	return obj;
}

Context::~Context()
{
	auto& impl = getImpl(this);

	impl.instance.destroy(impl.debugUtilsMessenger);
	impl.instance.destroy();

	destroyObjectImpl(this);
}

uint32_t Context::findDeviceByType(DeviceType type) const
{
	auto& impl             = getImpl(this);
	auto  physical_devices = impl.instance.enumeratePhysicalDevices();
	auto  required_type    = vk::PhysicalDeviceType::eDiscreteGpu;

	switch (type) {
	case DeviceType::Integrated:
		required_type = vk::PhysicalDeviceType::eIntegratedGpu;
		break;
	case DeviceType::Discrete:
		required_type = vk::PhysicalDeviceType::eDiscreteGpu;
		break;
	}

	for (uint32_t i = 0; i < physical_devices.size(); ++i)
		if (physical_devices[i].getProperties().deviceType == required_type)
			return i;

	return 0;
}

VERA_NAMESPACE_END
