#include "../../include/vera/core/shader_storage.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/pipeline_layout_impl.h"

#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/device.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/shader_reflection.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/texture.h"

VERA_NAMESPACE_BEGIN

obj<ShaderStorage> ShaderStorage::create(obj<ShaderReflection> reflection)
{
	VERA_ASSERT_MSG(reflection, "empty shader reflection");
	auto  obj             = createNewCoreObject<ShaderStorage>();
	auto& reflection_impl = getImpl(reflection);
	auto& device_impl     = getImpl(reflection_impl.device);


	return obj;
}

ShaderStorage::~ShaderStorage()
{
	auto& impl = getImpl(this);

	

	destroyObjectImpl(this);
}

VERA_NAMESPACE_END