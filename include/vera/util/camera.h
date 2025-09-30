#pragma once

#include "../math/vector_types.h"
#include "../math/matrix_types.h"
#include "../os/window_event.h"

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

class WindowEvent;

VERA_OS_NAMESPACE_END

class Camera abstract
{
public:
	virtual ~Camera() {};

	virtual void updateEvent(const os::WindowEvent& e, float dt) {};
	virtual void updateAsync(float dt) {};
	virtual float4x4 getMatrix() const = 0;
};

VERA_NAMESPACE_END