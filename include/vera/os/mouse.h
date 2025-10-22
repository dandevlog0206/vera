#pragma once

#include "../math/vector_types.h"

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

class Window;

class Mouse
{
public:
	enum Button
	{
		Unknown,
		LButton,
		RButton,
		MButton,
	};

	static void setPosition(const int2& pos, const Window& rel_wnd);
	static void setPosition(const int2& pos);
	VERA_NODISCARD static int2 getPosition(const Window& rel_wnd);
	VERA_NODISCARD static int2 getPosition();

	VERA_NODISCARD static bool isDown(Button button);
	VERA_NODISCARD static bool isUp(Button button);
};

VERA_OS_NAMESPACE_END
VERA_NAMESPACE_END