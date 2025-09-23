#include "../../include/vera/core/fence.h"
#include "../impl/fence_impl.h"

#include "../../include/vera/core/device.h"

VERA_NAMESPACE_BEGIN

static bool wait_fences(vk::Device vk_device, std::span<ref<Fence>> fences, bool wait_all, uint64_t timeout)
{
	static std::vector<vk::Fence> s_fences;

	s_fences.reserve(fences.size());
	for (auto fence : fences)
		s_fences.push_back(CoreObject::getImpl(fence.get()).fence);

	auto result = vk_device.waitForFences(s_fences, wait_all, timeout);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;
	
	throw Exception("failed to wait fences");
}

vk::Fence get_vk_fence(const ref<Fence>& fence)
{
	return CoreObject::getImpl(fence).fence;
}

vk::Fence& get_vk_fence(ref<Fence>& fence)
{
	return CoreObject::getImpl(fence).fence;
}

bool Fence::waitAll(std::span<ref<Fence>> fences, uint64_t timeout)
{
	if (fences.empty())
		return true;

	// TODO: check all fences are from same device;
	auto vk_device = get_vk_device(getImpl(fences.front()).device);

	return wait_fences(vk_device, fences, true, timeout);
}

bool Fence::waitAny(std::span<ref<Fence>> fences, uint64_t timeout)
{
	if (fences.empty())
		return true;

	// TODO: check all fences are from same device;
	auto vk_device = get_vk_device(getImpl(fences.front()).device);

	return wait_fences(vk_device, fences, false, timeout);
}

ref<Fence> Fence::create(ref<Device> device, bool signaled)
{
	auto  obj       = createNewObject<Fence>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	vk::FenceCreateInfo fence_info;
	if (signaled)
		fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

	impl.device = std::move(device);
	impl.fence  = vk_device.createFence(fence_info);

	return std::move(obj);
}

Fence::~Fence()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.destroy(impl.fence);

	destroyObjectImpl(this);
}

bool Fence::signaled() const
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	auto result = vk_device.getFenceStatus(impl.fence);

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

	vk_device.resetFences(impl.fence);
}

bool Fence::wait(uint64_t timeout) const
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	auto result = vk_device.waitForFences(impl.fence, true, timeout);
	
	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;

	throw Exception("failed to wait fence");
}

VERA_NAMESPACE_END