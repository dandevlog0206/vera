#include "../../include/vera/core/semaphore.h"
#include "../impl/semaphore_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/util/static_vector.h"

#define WAIT_ALL true
#define WAIT_ANY false

VERA_NAMESPACE_BEGIN

static bool check_same_device(std::span<obj<Semaphore>> semaphores)
{
	auto vk_device = CoreObject::getImpl(semaphores.front()).device;

	for (size_t i = 1; i < semaphores.size(); ++i)
		if (CoreObject::getImpl(semaphores[i]).device != vk_device)
			return false;
	return true;
}

static bool wait_semaphores(
	vk::Device                vk_device,
	std::span<obj<Semaphore>> semaphores,
	uint64_t                  timeout,
	bool                      wait_all)
{
	static_vector<vk::Semaphore, 128> vk_semaphores;

	for (auto semaphore : semaphores)
		vk_semaphores.push_back(CoreObject::getImpl(semaphore).semaphore);

	vk::SemaphoreWaitInfo wait_info;
	wait_info.flags          = wait_all ? vk::SemaphoreWaitFlagBits{} : vk::SemaphoreWaitFlagBits::eAny;
	wait_info.semaphoreCount = static_cast<uint32_t>(vk_semaphores.size());
	wait_info.pSemaphores    = vk_semaphores.data();

	auto result = vk_device.waitSemaphores(wait_info, timeout);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;

	throw Exception("failed to wait semaphore");
}

static bool wait_timeline_semaphores(
	vk::Device                vk_device,
	std::span<obj<Semaphore>> semaphores,
	uint64_t                  value,
	uint64_t                  timeout,
	bool                      wait_all
) {
	static_vector<vk::Semaphore, 128> vk_semaphores;
	static_vector<uint64_t, 128> wait_values;

	for (auto semaphore : semaphores)
		vk_semaphores.push_back(CoreObject::getImpl(semaphore).semaphore);

	wait_values.resize(semaphores.size(), value);

	vk::SemaphoreWaitInfo wait_info;
	wait_info.flags          = wait_all ? vk::SemaphoreWaitFlagBits{} : vk::SemaphoreWaitFlagBits::eAny;
	wait_info.semaphoreCount = static_cast<uint32_t>(vk_semaphores.size());
	wait_info.pSemaphores    = vk_semaphores.data();
	wait_info.pValues        = wait_values.data();

	auto result = vk_device.waitSemaphores(wait_info, timeout);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;

	throw Exception("failed to wait semaphore");
}

const vk::Semaphore& get_vk_semaphore(const_ref<Semaphore> semaphore) VERA_NOEXCEPT
{
	return CoreObject::getImpl(semaphore).semaphore;
}

vk::Semaphore& get_vk_semaphore(ref<Semaphore> semaphore) VERA_NOEXCEPT
{
	return CoreObject::getImpl(semaphore).semaphore;
}

bool Semaphore::waitAll(std::span<obj<Semaphore>> semaphores, uint64_t timeout)
{
	if (semaphores.empty())
		return true;

	VERA_ASSERT_MSG(check_same_device(semaphores), "semaphores don't share same device");

	auto vk_device = get_vk_device(getImpl(semaphores.front()).device);

	return wait_semaphores(vk_device, semaphores, timeout, WAIT_ALL);
}

bool Semaphore::waitAny(std::span<obj<Semaphore>> semaphores, uint64_t timeout)
{
	if (semaphores.empty())
		return true;

	VERA_ASSERT_MSG(check_same_device(semaphores), "semaphores don't share same device");

	auto vk_device = get_vk_device(getImpl(semaphores.front()).device);

	return wait_semaphores(vk_device, semaphores, timeout, WAIT_ANY);
}

obj<Semaphore> Semaphore::create(obj<Device> device)
{
	auto  obj       = createNewCoreObject<Semaphore>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	impl.device        = std::move(device);
	impl.semaphore     = vk_device.createSemaphore({});

	return obj;
}

Semaphore::~Semaphore()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk_device.destroy(impl.semaphore);

	destroyObjectImpl(this);
}

obj<Device> Semaphore::getDevice()
{
	return getImpl(this).device;
}

bool Semaphore::wait(uint64_t timeout) const
{
	auto&    impl      = getImpl(this);
	auto     vk_device = get_vk_device(impl.device);

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

bool TimelineSemaphore::waitAll(std::span<obj<Semaphore>> semaphores, uint64_t value, uint64_t timeout)
{
	if (semaphores.empty())
		return true;

	VERA_ASSERT_MSG(check_same_device(semaphores), "semaphores don't share same device");

	auto vk_device = get_vk_device(getImpl(semaphores.front()).device);

	return wait_timeline_semaphores(vk_device, semaphores, value, timeout, WAIT_ALL);
}

bool TimelineSemaphore::waitAny(std::span<obj<Semaphore>> semaphores, uint64_t value, uint64_t timeout)
{
	if (semaphores.empty())
		return true;

	VERA_ASSERT_MSG(check_same_device(semaphores), "semaphores don't share same device");

	auto vk_device = get_vk_device(getImpl(semaphores.front()).device);

	return wait_timeline_semaphores(vk_device, semaphores, value, timeout, WAIT_ANY);
}

obj<TimelineSemaphore> TimelineSemaphore::create(obj<Device> device, uint64_t initial_value)
{
	auto  obj       = createNewCoreObject<TimelineSemaphore>();
	auto& impl      = getImpl(obj);
	auto  vk_device = get_vk_device(device);

	vk::SemaphoreTypeCreateInfo semaphore_type_info;
	semaphore_type_info.semaphoreType = vk::SemaphoreType::eTimeline;
	semaphore_type_info.initialValue  = initial_value;
	
	vk::SemaphoreCreateInfo semaphore_info;
	semaphore_info.pNext = &semaphore_type_info;

	impl.device        = std::move(device);
	impl.semaphore     = vk_device.createSemaphore(semaphore_info);
	
	return obj;
}

TimelineSemaphore::~TimelineSemaphore()
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);
	vk_device.destroy(impl.semaphore);
	destroyObjectImpl(this);
}

bool TimelineSemaphore::wait(uint64_t value, uint64_t timeout) const
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk::SemaphoreWaitInfo wait_info;
	wait_info.semaphoreCount = 1;
	wait_info.pSemaphores    = &impl.semaphore;
	wait_info.pValues        = &value;

	auto result = vk_device.waitSemaphores(wait_info, timeout);

	if (result == vk::Result::eSuccess)
		return true;
	if (result == vk::Result::eTimeout)
		return false;

	throw Exception("failed to wait semaphore");
}

void TimelineSemaphore::signal(uint64_t value)
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	vk::SemaphoreSignalInfo signal_info;
	signal_info.semaphore = impl.semaphore;
	signal_info.value     = value;

	vk_device.signalSemaphore(signal_info);
}

uint64_t TimelineSemaphore::value() const
{
	auto& impl      = getImpl(this);
	auto  vk_device = get_vk_device(impl.device);

	return vk_device.getSemaphoreCounterValue(impl.semaphore);
}

VERA_NAMESPACE_END