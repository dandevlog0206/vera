#pragma once

#include "../util/lookup_table.h"
#include "bezier.h"
#include <vector>

VERA_NAMESPACE_BEGIN

class Path2D
{
public:
	Path2D();
	~Path2D();

	void moveTo(const float2& p);
	void moveTo(float x, float y);

	void lineTo(const float2& p);
	void lineTo(float x, float y);

	void quadTo(const float2& p1, const float2& p2);
	void quadTo(float x1, float y1, float x2, float y2);

	void cubicTo(const float2& p1, const float2& p2, const float2& p3);
	void cubicTo(float x1, float y1, float x2, float y2, float x3, float y3);

	void close();

	void clear();
	
	bool empty() const;

private:
	enum class Verb {
		Move,
		Line,
		Quad,
		Cubic,
		Close
	};

	std::vector<Verb>   m_verbs;
	std::vector<float2> m_points;
};

class Path3D
{
public:
};

VERA_NAMESPACE_END
