#pragma once

#include "../graphics/color.h"

VERA_NAMESPACE_BEGIN

struct Colormaps
{
	enum Colormap VERA_ENUM
	{
		Magma,
		Inferno,
		Plasma,
		Viridis,
		Cividis,
		Twilight,
		Turbo,
		Berlin,
		Managua,
		Vanimo
	};

	VERA_NODISCARD static Color map(Colormap colormap, uint8_t value);
	VERA_NODISCARD static Color map(Colormap colormap, float value);

	VERA_NODISCARD static Color magma(uint8_t value);
	VERA_NODISCARD static Color inferno(uint8_t value);
	VERA_NODISCARD static Color plasma(uint8_t value);
	VERA_NODISCARD static Color viridis(uint8_t value);
	VERA_NODISCARD static Color cividis(uint8_t value);
	VERA_NODISCARD static Color twilight(uint8_t value);
	VERA_NODISCARD static Color turbo(uint8_t value);
	VERA_NODISCARD static Color berlin(uint8_t value);
	VERA_NODISCARD static Color managua(uint8_t value);
	VERA_NODISCARD static Color vanimo(uint8_t value);

	VERA_NODISCARD static Color magma(float value);
	VERA_NODISCARD static Color inferno(float value);
	VERA_NODISCARD static Color plasma(float value);
	VERA_NODISCARD static Color viridis(float value);
	VERA_NODISCARD static Color cividis(float value);
	VERA_NODISCARD static Color twilight(float value);
	VERA_NODISCARD static Color turbo(float value);
	VERA_NODISCARD static Color berlin(float value);
	VERA_NODISCARD static Color managua(float value);
	VERA_NODISCARD static Color vanimo(float value);
};

VERA_NAMESPACE_END