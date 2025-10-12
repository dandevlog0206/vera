#pragma once

// asset
#include "asset/asset_loader.h"

// core
#include "core/context.h"
#include "core/core_object.h"
#include "core/coredefs.h"
#include "core/device.h"
#include "core/device_memory.h"
#include "core/exception.h"
#include "core/fence.h"
#include "core/logger.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/command_buffer.h"
#include "core/render_context.h"
#include "core/resource_layout.h"
#include "core/sampler.h"
#include "core/semaphore.h"
#include "core/shader.h"
#include "core/swapchain.h"
#include "core/texture.h"
#include "core/texture_view.h"

// graphics
#include "graphics/color.h"
#include "graphics/colormap.h"
#include "graphics/format.h"
#include "graphics/format_traits.h"
#include "graphics/graphics_state.h"
#include "graphics/image.h"
#include "graphics/image_edit.h"
#include "graphics/image_sampler.h"
#include "graphics/model_loader.h"
#include "graphics/shader_parameter.h"
#include "graphics/transform2d.h"
#include "graphics/transform3d.h"
#include "graphics/vertex_input.h"

// math
#include "math/constant.h"
#include "math/math_util.h"
#include "math/matrix_types.h"
#include "math/quaternion.h"
#include "math/radian.h"
#include "math/vector_types.h"
#include "math/vector_math.h"

// os
#include "os/keyboard.h"
#include "os/mouse.h"
#include "os/window.h"
#include "os/window_event.h"

// pass
#include "pass/graphics_pass.h"

// scene
#include "scene/attribute.h"
#include "scene/axis_system.h"
#include "scene/mesh_attribute.h"
#include "scene/node.h"
#include "scene/scene.h"

// util
#include "util/arcball.h"
#include "util/enum_traits.h"
#include "util/extent.h"
#include "util/flag.h"
#include "util/flycam.h"
#include "util/hash.h"
#include "util/property.h"
#include "util/rect.h"
#include "util/result_message.h"
#include "util/small_vector.h"
#include "util/static_vector.h"
#include "util/timer.h"
#include "util/uuid.h"
#include "util/version.h"