#pragma once

// core
#include "core/context.h"
#include "core/core_object.h"
#include "core/coredefs.h"
#include "core/device.h"
#include "core/device_memory.h"
#include "core/exception.h"
#include "core/fence.h"
#include "core/graphics_state.h"
#include "core/logger.h"
#include "core/pipeline.h"
#include "core/pipeline_layout.h"
#include "core/render_command.h"
#include "core/render_context.h"
#include "core/resource_layout.h"
#include "core/sampler.h"
#include "core/semaphore.h"
#include "core/shader.h"
#include "core/shader_reflection.h"
#include "core/swapchain.h"
#include "core/texture.h"
#include "core/texture_view.h"

// graphics
#include "graphics/color.h"
#include "graphics/colormap.h"
#include "graphics/format.h"
#include "graphics/format_traits.h"
#include "graphics/image.h"
#include "graphics/image_edit.h"
#include "graphics/image_sampler.h"
#include "graphics/vertex_input.h"

// math
#include "math/vector_types.h"

// os
#include "os/keyboard.h"
#include "os/mouse.h"
#include "os/window.h"
#include "os/window_event.h"

// pass
#include "pass/graphics_pass.h"

// shader
#include "shader/shader_parameter.h"
#include "shader/shader_variable.h"

// util
#include "util/enum_traits.h"
#include "util/extent.h"
#include "util/flag.h"
#include "util/hash.h"
#include "util/property.h"
#include "util/rect.h"
#include "util/small_vector.h"
#include "util/static_vector.h"
#include "util/uuid.h"
#include "util/version.h"