#include "../../include/vera/core/fence.h"
#include "../impl/fence_impl.h"

#include "../../include/vera/core/device.h"

VERA_NAMESPACE_BEGIN

static bool check_same_device(std::span<obj<Fence>> fences)
{
	auto vk_device = CoreObject::getImpl(fences.front()).device;

	for (size_t i = 1; i < fences.size(); ++i)
		if (CoreObject::getImpl(fences[i]).device != vk_device)
			return false;
	return true;
}

static bool wait_fences(vk::Device vk_device, std::span<obj<Fence>> fences, bool wait_all, uint64_t timeout)
{
	static std::vector<vk::Fence> s_fences;

	s_fences.reserve(fences.size());
	for (auto fence : fences)
		s_fences.push_back(CoreObject::getImpl(fence.get()).vkFence);

	auto result = vk_device.waitForFences(s_fences, wait_all, timeout);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;
	
	throw Exception("failed to wait fences");
}

const vk::Fence& get_vk_fence(const_ref<Fence> fence) VERA_NOEXCEPT
{
	return CoreObject::getImpl(fence).vkFence;
}

vk::Fence& get_vk_fence(ref<Fence> fence) VERA_NOEXCEPT
{
	return CoreObject::getImpl(fence).vkFence;
}

bool Fence::waitAll(std::span<obj<Fence>> fences, uint64_t timeout)
{
	if (fences.empty())
		return true;

	VERA_ASSERT_MSG(check_same_device(fences), "fences don't share same device");

	auto vk_device = get_vk_device(getImpl(fences.front()).device);

	return wait_fences(vk_device, fences, true, timeout);
}

bool Fence::waitAny(std::span<obj<Fence>> fences, uint64_t timeout)
{
	if (fences.empty())
		return true;

	VERA_ASSERT_MSG(check_same_device(fences), "fences don't share same device");

	auto vk_device = get_vk_device(getImpl(fences.front()).device);

	return wait_fences(vk_device, fences, false, timeout);
}

obj<Fence> Fence::create(obj<Device> device, bool signaled)
{
	auto  obj       = createNewCoreObject<Fence>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	vk::FenceCreateInfo fence_info;

	if (signaled)
		fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

	impl.device  = std::move(device);
	impl.vkFence = vk_device.createFence(fence_info);

	return obj;
}

Fence::~Fence() VERA_NOEXCEPT
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.destroy(impl.vkFence);

	destroyObjectImpl(this);
}

obj<Device> Fence::getDevice()
{
	return getImpl(this).device;
}

bool Fence::signaled() const
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	auto result = vk_device.getFenceStatus(impl.vkFence);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eNotReady)
		return false;
	
	throw Exception("failed to get fence status");
}

void Fence::reset()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.resetFences(impl.vkFence);
}

bool Fence::wait(uint64_t timeout) const
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	auto result = vk_device.waitForFences(impl.vkFence, true, timeout);
	
	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;

	throw Exception("failed to wait fence");
}

VERA_NAMESPACE_END