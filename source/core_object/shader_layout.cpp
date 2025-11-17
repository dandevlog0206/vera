#include "../../include/vera/core/shader_layout.h"
#include "../impl/device_impl.h"
#include "../impl/shader_layout_impl.h"

VERA_NAMESPACE_BEGIN

static hash_t hash_shader_layout(const ShaderLayoutCreateInfo& info)
{
	hash_t seed = 0;

	hash_combine(seed, info.stage);

	hash_combine(seed, info.bindings.size());
	for (const auto& binding : info.bindings) {
		hash_combine(seed, binding.set);
		hash_combine(seed, binding.binding);
		hash_combine(seed, static_cast<uint32_t>(binding.descriptorType));
		hash_combine(seed, binding.descriptorCount);
	}

	hash_combine(seed, info.pushConstantOffset);
	hash_combine(seed, info.pushConstantSize);

	return seed;
}

obj<ShaderLayout> ShaderLayout::create(obj<Device> device, const ShaderLayoutCreateInfo& create_info)
{
	static const auto cmp_binding =
		[](const ShaderLayoutBinding& lhs, const ShaderLayoutBinding& rhs) {
		return lhs.set != rhs.set ? lhs.set < rhs.set : lhs.binding < rhs.binding;
	};

	static const auto eq_binding =
		[](const ShaderLayoutBinding& lhs, const ShaderLayoutBinding& rhs) {
		return lhs.set == rhs.set && lhs.binding == rhs.binding;
	};

	auto& device_impl = getImpl(device);
	auto  hash_value  = hash_shader_layout(create_info);

	if (auto it = device_impl.shaderLayoutCacheMap.find(hash_value);
		it != device_impl.shaderLayoutCacheMap.end()) {
		return unsafe_obj_cast<ShaderLayout>(it->second);
	}

	auto  obj  = createNewCoreObject<ShaderLayout>();
	auto& impl = getImpl(obj);

	impl.device                       = std::move(device);
	impl.bindings                     = create_info.bindings;
	impl.pushConstantRange.offset     = create_info.pushConstantOffset;
	impl.pushConstantRange.size       = create_info.pushConstantSize;
	impl.pushConstantRange.stageFlags = create_info.stage;
	impl.hashValue                    = hash_value;

	std::sort(VERA_SPAN(impl.bindings), cmp_binding);
	auto last_unique = std::unique(VERA_SPAN(impl.bindings), eq_binding);
	impl.bindings.erase(last_unique, impl.bindings.end());

	return obj;
}

ShaderLayout::~ShaderLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterShaderLayout(impl.hashValue);

	destroyObjectImpl(this);
}

ShaderStageFlags ShaderLayout::getShaderStageFlags() const VERA_NOEXCEPT
{
	return getImpl(this).pushConstantRange.stageFlags;
}

array_view<ShaderLayoutBinding> ShaderLayout::enumerateBindings() const VERA_NOEXCEPT
{
	return getImpl(this).bindings;
}

PushConstantRange ShaderLayout::getPushConstantRange() const VERA_NOEXCEPT
{
	return getImpl(this).pushConstantRange;
}

hash_t ShaderLayout::hash() const VERA_NOEXCEPT
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END