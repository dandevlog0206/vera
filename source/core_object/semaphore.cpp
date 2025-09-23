#include "../../include/vera/core/semaphore.h"
#include "../impl/semaphore_impl.h"

#include "../../include/vera/core/device.h"

VERA_NAMESPACE_BEGIN

static bool wait_semaphores(vk::Device vk_device, std::span<ref<Semaphore>> semaphores, uint64_t timeout, bool wait_all)
{
	static std::vector<vk::Semaphore> s_semaphores;

	s_semaphores.reserve(semaphores.size());
	for (auto semaphore : semaphores)
		s_semaphores.push_back(CoreObject::getImpl(semaphore).semaphore);

	vk::SemaphoreWaitInfo wait_info;
	wait_info.flags          = wait_all ? vk::SemaphoreWaitFlagBits{} : vk::SemaphoreWaitFlagBits::eAny;
	wait_info.semaphoreCount = static_cast<uint32_t>(s_semaphores.size());
	wait_info.pSemaphores    = s_semaphores.data();

	auto result = vk_device.waitSemaphores(wait_info, timeout);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;

	throw Exception("failed to wait semaphore");
}

vk::Semaphore get_vk_semaphore(const ref<Semaphore>& semaphore)
{
	return CoreObject::getImpl(semaphore).semaphore;
}

vk::Semaphore& get_vk_semaphore(ref<Semaphore>& semaphore)
{
	return CoreObject::getImpl(semaphore).semaphore;
}

bool Semaphore::waitAll(std::span<ref<Semaphore>> semaphores, uint64_t timeout)
{
	if (semaphores.empty())
		return true;

	auto vk_device = get_vk_device(getImpl(semaphores.front()).device);

	return wait_semaphores(vk_device, semaphores, true, timeout);
}

bool Semaphore::waitAny(std::span<ref<Semaphore>> semaphores, uint64_t timeout)
{
	if (semaphores.empty())
		return true;

	auto vk_device = get_vk_device(getImpl(semaphores.front()).device);

	return wait_semaphores(vk_device, semaphores, false, timeout);
}

ref<Semaphore> Semaphore::create(ref<Device> device)
{
	auto  obj       = createNewObject<Semaphore>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	impl.device    = std::move(device);
	impl.semaphore = vk_device.createSemaphore({});

	return std::move(obj);
}

void Semaphore::signal(uint64_t value)
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk::SemaphoreSignalInfo signal_info;
	signal_info.semaphore = impl.semaphore;
	signal_info.value     = value;

	vk_device.signalSemaphore(signal_info);
}

uint64_t Semaphore::value()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	return vk_device.getSemaphoreCounterValue(impl.semaphore);
}

bool Semaphore::wait(uint64_t timeout)
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk::SemaphoreWaitInfo wait_info;
	wait_info.semaphoreCount = 1;
	wait_info.pSemaphores    = &impl.semaphore;

	auto result = vk_device.waitSemaphores(wait_info, timeout);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;

	throw Exception("failed to wait semaphore");
}

VERA_NAMESPACE_END
