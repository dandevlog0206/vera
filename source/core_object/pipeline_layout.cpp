#include "../../include/vera/core/pipeline_layout.h"
#include "../impl/device_impl.h"
#include "../impl/pipeline_layout_impl.h"
#include "../impl/resource_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/util/hash.h"
#include "../../include/vera/util/static_vector.h"

VERA_NAMESPACE_BEGIN

static size_t hash_pipeline_layout(const PipelineLayoutCreateInfo& info)
{
	size_t seed = 0;
	
	hash_combine(seed, info.resourceLayouts.size());
	for (const auto& layout : info.resourceLayouts)
		hash_combine(seed, CoreObject::getImpl(layout).hashValue);
	
	hash_combine(seed, info.pushConstantRanges.size());
	for (const auto& range : info.pushConstantRanges) {
		hash_combine(seed, range.offset);
		hash_combine(seed, range.size);
		hash_combine(seed, static_cast<size_t>(range.stageFlags));
	}

	return seed;
}

vk::PipelineLayout& get_vk_pipeline_layout(ref<PipelineLayout> pipeline_layout)
{
	return CoreObject::getImpl(pipeline_layout).layout;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, const PipelineLayoutCreateInfo& info)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_pipeline_layout(info);

	if (auto it = device_impl.pipelineLayoutMap.find(hash_value);
		it != device_impl.pipelineLayoutMap.end()) {
		return it->second;
	}

	auto  obj  = createNewObject<PipelineLayout>();
	auto& impl = getImpl(obj);

	static_vector<vk::DescriptorSetLayout, 20> vk_layouts;
	static_vector<vk::PushConstantRange, 20>   vk_constants;

	for (const auto& layout : info.resourceLayouts)
		vk_layouts.push_back(getImpl(layout).layout);
	for (const auto& range : info.pushConstantRanges)
		vk_constants.push_back(get_push_constant_range(range));

	vk::PipelineLayoutCreateInfo pipeline_layout_info;
	pipeline_layout_info.setLayoutCount         = static_cast<uint32_t>(vk_layouts.size());
	pipeline_layout_info.pSetLayouts            = vk_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(vk_constants.size());
	pipeline_layout_info.pPushConstantRanges    = vk_constants.data();

	impl.device    = std::move(device);
	impl.layout    = device_impl.device.createPipelineLayout(pipeline_layout_info);
	impl.hashValue = hash_value;
	impl.resourceLayout.assign(info.resourceLayouts.begin(), info.resourceLayouts.end());
	impl.pushConstantRanges.assign(info.pushConstantRanges.begin(), info.pushConstantRanges.end());

	return device_impl.pipelineLayoutMap[hash_value] = obj;
}

PipelineLayout::~PipelineLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.device.destroy(impl.layout);

	destroyObjectImpl(this);
}

obj<Device> PipelineLayout::getDevice()
{
	return getImpl(this).device;
}

const std::vector<obj<ResourceLayout>>& PipelineLayout::getResourceLayouts() const
{
	return getImpl(this).resourceLayout;
}

const std::vector<PushConstantRange>& PipelineLayout::getPushConstantRanges() const
{
	return getImpl(this).pushConstantRanges;
}

size_t PipelineLayout::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END