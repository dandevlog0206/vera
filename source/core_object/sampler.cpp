#include "../../include/vera/core/sampler.h"
#include "../impl/device_impl.h"
#include "../impl/sampler_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/util/hash.h"

VERA_NAMESPACE_BEGIN

static vk::BorderColor get_border_color(const Color& color, bool& need_bord_info)
{
	if (color == Colors::TransparentBlack)
		return vk::BorderColor::eFloatTransparentBlack;
	if (color == Colors::Black)
		return vk::BorderColor::eFloatOpaqueBlack;
	if (color == Colors::White)
		return vk::BorderColor::eFloatOpaqueWhite;

	need_bord_info = true;

	return vk::BorderColor::eFloatCustomEXT;
}

static size_t hash_sampler(const SamplerCreateInfo& info)
{
	size_t seed = 0;

	hash_combine(seed, static_cast<size_t>(info.magFilter));
	hash_combine(seed, static_cast<size_t>(info.minFilter));
	hash_combine(seed, static_cast<size_t>(info.mipmapMode));
	hash_combine(seed, static_cast<size_t>(info.addressModeU));
	hash_combine(seed, static_cast<size_t>(info.addressModeV));
	hash_combine(seed, static_cast<size_t>(info.addressModeW));
	hash_combine(seed, info.mipLodBias);
	hash_combine(seed, info.anisotropyEnable);
	hash_combine(seed, info.maxAnisotropy);
	hash_combine(seed, info.compareEnable);
	hash_combine(seed, static_cast<size_t>(info.compareOp));
	hash_combine(seed, info.minLod);
	hash_combine(seed, info.maxLod);
	hash_combine(seed, info.borderColor.pack());
	hash_combine(seed, info.unnormalizedCoordinates);

	return seed;
}

vk::Sampler& get_vk_sampler(ref<Sampler> sampler)
{
	return CoreObject::getImpl(sampler).sampler;
}

obj<Sampler> Sampler::create(obj<Device> device, const SamplerCreateInfo& info)
{
	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_sampler(info);

	if (auto it = device_impl.samplerMap.find(hash_value);
		it != device_impl.samplerMap.end()) {
		return it->second;
	}

	auto   obj         = createNewCoreObject<Sampler>();
	auto&  impl        = getImpl(obj);
	bool   need_border = false;

	vk::SamplerCreateInfo sampler_info;
	sampler_info.magFilter               = to_vk_filter(info.magFilter);
	sampler_info.minFilter               = to_vk_filter(info.minFilter);
	sampler_info.mipmapMode              = to_vk_sampler_mipmap_mode(info.mipmapMode);
	sampler_info.addressModeU            = to_vk_sampler_address_mode(info.addressModeU);
	sampler_info.addressModeV            = to_vk_sampler_address_mode(info.addressModeV);
	sampler_info.addressModeW            = to_vk_sampler_address_mode(info.addressModeW);
	sampler_info.mipLodBias              = info.mipLodBias;
	sampler_info.anisotropyEnable        = info.anisotropyEnable;
	sampler_info.maxAnisotropy           = info.maxAnisotropy;
	sampler_info.compareEnable           = info.compareEnable;
	sampler_info.compareOp               = to_vk_compare_op(info.compareOp);
	sampler_info.minLod                  = info.minLod;
	sampler_info.maxLod                  = info.maxLod;
	sampler_info.borderColor             = get_border_color(info.borderColor, need_border);
	sampler_info.unnormalizedCoordinates = info.unnormalizedCoordinates;

	vk::SamplerCustomBorderColorCreateInfoEXT border_info;
	if (need_border) {
		sampler_info.pNext = &border_info;

		border_info.customBorderColor.float32 = std::array<float, 4>{
			info.borderColor.r / 255.f,
			info.borderColor.g / 255.f,
			info.borderColor.b / 255.f,
			info.borderColor.a / 255.f
		};
		border_info.format = vk::Format::eR8G8B8A8Unorm;
	}

	impl.device    = std::move(device);
	impl.sampler   = device_impl.device.createSampler(sampler_info);
	impl.hashValue = hash_value;
	impl.info      = info;

	return device_impl.samplerMap[hash_value] = obj;
}

Sampler::~Sampler()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);
	
	device_impl.device.destroy(impl.sampler);
	
	destroyObjectImpl(this);
}

obj<Device> Sampler::getDevice()
{
	return getImpl(this).device;
}

const SamplerCreateInfo& Sampler::getInfo() const
{
	return getImpl(this).info;
}

size_t Sampler::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END