#pragma once

#include "../geometry/path.h"
#include "../geometry/aabb.h"
#include "../util/array_view.h"

VERA_NAMESPACE_BEGIN

typedef uint32_t GlyphID;

struct GlyphPoint
{
	float2 position;
	bool   onCurve;
};

class Glyph
{
public:
	using ContourType = std::vector<GlyphPoint>;
	using ContourList = std::vector<ContourType>;

	GlyphID             glyphID;
	AABB2D              aabb;
	ContourList         contours;
	array_view<uint8_t> instructions;
};

VERA_NAMESPACE_END
