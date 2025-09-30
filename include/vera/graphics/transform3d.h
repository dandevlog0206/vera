#pragma once

#include "../math/matrix_types.h"
#include "../math/vector_types.h"
#include "../util/radian.h"

VERA_NAMESPACE_BEGIN

class Transform3D
{
public:
	Transform3D();
	Transform3D(const float4x4& mat);
	Transform3D(const Transform3D& rhs) = default;

	Transform3D& operator=(const Transform3D& rhs) = default;

	Transform3D& translate(float x, float y, float z);
	Transform3D& translate(const float3& offset);

	Transform3D& rotateX(radian radian_x);
	Transform3D& rotateY(radian radian_y);
	Transform3D& rotateZ(radian radian_z);
	Transform3D& rotate(const float3& u, float theta);
	Transform3D& rotate(float radian_x, float radian_y, float radian_z);
	Transform3D& rotate(const float3& radian_xyz);

	Transform3D& scale(float scale);
	Transform3D& scale(float scale_x, float scale_y, float scale_z);
	Transform3D& scale(const float3& scale);

	Transform3D& clear();

	const float4x4& getMatrix() const;

	Transform3D& operator*=(const Transform3D& rhs);
	Transform3D operator*(const Transform3D& rhs) const;
	float3 operator*(const float3& rhs) const;

	bool operator==(const Transform3D& rhs) const;
	bool operator!=(const Transform3D& rhs) const;

private:
	float4x4 m_mat;
};

VERA_NAMESPACE_END
