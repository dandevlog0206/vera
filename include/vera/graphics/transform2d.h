#pragma once

#include "../math/matrix_types.h"
#include "../math/vector_types.h"
#include "../util/radian.h"

VERA_NAMESPACE_BEGIN

class Transform2D
{
public:
	Transform2D();
	Transform2D(const float3x3& mat);
	Transform2D(const Transform2D& rhs) = default;

	Transform2D& operator=(const Transform2D& rhs) = default;

	Transform2D& translate(float x, float y);
	Transform2D& translate(const float2& offset);

	Transform2D& rotate(radian rad);
	Transform2D& rotate(radian rad, float center_x, float center_y);
	Transform2D& rotate(radian rad, const float2& center);

	Transform2D& scale(float scale);
	Transform2D& scale(float scale_x, float scale_y);
	Transform2D& scale(const float2& scale);

	Transform2D& shear(radian rad_x, radian rad_y);
	Transform2D& shearX(radian rad);
	Transform2D& shearY(radian rad);

	Transform2D& clear();

	const float3x3& getMatrix() const;

	Transform2D& operator*=(const Transform2D& rhs);
	Transform2D operator*(const Transform2D& rhs) const;
	float2 operator*(const float2& rhs) const;

	bool operator==(const Transform2D& rhs) const;
	bool operator!=(const Transform2D& rhs) const;

private:
	float3x3 m_mat;
};

VERA_NAMESPACE_END
