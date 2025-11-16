#pragma once

#include "device.h"

VERA_NAMESPACE_BEGIN

struct PushConstantRange
{
	uint32_t         offset     = {};
	uint32_t         size       = {};
	ShaderStageFlags stageFlags = {};
};

struct ShaderLayoutBinding
{
	uint32_t       set;
	uint32_t       binding;
	DescriptorType descriptorType;
	uint32_t       descriptorCount;
};

class ShaderLayoutCreateInfo
{
public:
	ShaderStageFlagBits              stage;
	std::vector<ShaderLayoutBinding> bindings;
	uint32_t                         pushConstantOffset;
	uint32_t                         pushConstantSize;
};

class ShaderLayout : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(ShaderLayout)
public:
	static obj<ShaderLayout> create(obj<Device> device, const ShaderLayoutCreateInfo& create_info);
	~ShaderLayout();

	VERA_NODISCARD obj<Device> getDevice() const VERA_NOEXCEPT;

	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<ShaderLayoutBinding> getBindings() const VERA_NOEXCEPT;
	VERA_NODISCARD PushConstantRange getPushConstantRange() const VERA_NOEXCEPT;

	VERA_NODISCARD hash_t hash() const VERA_NOEXCEPT;
};

VERA_NAMESPACE_END