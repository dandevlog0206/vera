#pragma once

#include "core_object.h"
#include "../util/version.h"
#include <memory>
#include <vector>
#include <string_view>

VERA_NAMESPACE_BEGIN

enum class DeviceType VERA_ENUM
{
	Default    = 0,
	Integrated = 1,
	Discrete   = 2,
};

enum class QueueType VERA_ENUM
{
	Graphics,
	Compute,
	Transfer
};

struct InstanceLayerInfo
{
	const char* name;
	const char* description;
	Version     implementaionVersion;
	Version     specVersion;
};

struct InstanceExtensionInfo
{
	const char* name;
	Version     specVersion;
};

struct ContextCreateInfo
{
	std::string_view              applicationName          = "vera application";
	Version                       applicationVersion       = { 1, 0, 0 };

	std::vector<std::string_view> instanceLayers           = {};
	std::vector<std::string_view> instanceExtensions       = {};

	bool                          enableValidation         = VERA_IS_DEBUG;
	bool                          enableDebugUtils         = VERA_IS_DEBUG;
	uint32_t                      debugUtilsMessengerLevel = 0;
	bool                          enableDeviceFault        = false;
};

class Context : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Context)
public:
	static std::vector<InstanceLayerInfo> enumerateInstanceLayers();
	static std::vector<InstanceExtensionInfo> enumerateInstanceExtensions();

	static obj<Context> create(const ContextCreateInfo& info = {});
	~Context() VERA_NOEXCEPT override;

	uint32_t findDeviceByType(DeviceType type) const;
};

VERA_NAMESPACE_END