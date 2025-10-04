#include "../../include/vera/graphics/transform3d.h"

#include "../../include/vera/math/quaternion.h"
#include "../../include/vera/math/vector_math.h"

VERA_NAMESPACE_BEGIN

Transform3D::Transform3D() :
	m_mat(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f) {
}

Transform3D::Transform3D(const TransformDesc3D& desc)
{
	const auto qx = Quaternion::fromAxisAngle(float3(1.f, 0.f, 0.f), desc.rotation.x);
	const auto qy = Quaternion::fromAxisAngle(float3(0.f, 1.f, 0.f), desc.rotation.y);
	const auto qz = Quaternion::fromAxisAngle(float3(0.f, 0.f, 1.f), desc.rotation.z);

	const auto t0 = float4x4{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		desc.position.x, desc.position.y, desc.position.z, 1.f
	};

	const auto r = (qx * qy * qz).toMatrix4x4();

	const auto s = float4x4{
		desc.scale.x, 0.f, 0.f, 0.f,
		0.f, desc.scale.y, 0.f, 0.f,
		0.f, 0.f, desc.scale.z, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	const auto t1 = float4x4{
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		-desc.origin.x, -desc.origin.y, -desc.origin.z, 1.f
	};

	m_mat = t0 * r * s * t1;
}

Transform3D::Transform3D(const float4x4& mat) :
	m_mat(mat) {}

Transform3D& Transform3D::translate(float x, float y, float z)
{
	m_mat *= float4x4(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		  x,   y,   z, 1.f);

	return *this;
}

Transform3D& Transform3D::translate(const float3& offset)
{
	m_mat *= float4x4(
		     1.f,      0.f,      0.f, 0.f,
		     0.f,      1.f,      0.f, 0.f,
		     0.f,      0.f,      1.f, 0.f,
		offset.x, offset.y, offset.z, 1.f);

	return *this;
}

Transform3D& Transform3D::rotateX(radian radian_x)
{
	const float s = sinf(radian_x);
	const float c = cosf(radian_x);
	
	m_mat *= float4x4(
		1.f, 0.f, 0.f, 0.f,
		0.f,   c,   s, 0.f,
		0.f,  -s,   c, 0.f,
		0.f, 0.f, 0.f, 1.f);

	return *this;
}

Transform3D& Transform3D::rotateY(radian radian_y)
{
	const float s = sinf(radian_y);
	const float c = cosf(radian_y);
	
	m_mat *= float4x4(
		  c, 0.f,  -s, 0.f,
		0.f, 1.f, 0.f, 0.f,
		  s, 0.f,   c, 0.f,
		0.f, 0.f, 0.f, 1.f);

	return *this;
}

Transform3D& Transform3D::rotateZ(radian radian_z)
{
	const float s = sinf(radian_z);
	const float c = cosf(radian_z);
	
	m_mat *= float4x4(
		  c,   s, 0.f, 0.f,
		 -s,   c, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);

	return *this;
}

Transform3D& Transform3D::rotate(const float3& u, float radian)
{
	const float l   = length(u);
	const float s   = sinf(radian);
	const float c   = cosf(radian);
	const float oms = 1.f - s;
	const float omc = 1.f - c;
	const float ux  = u.x / l;
	const float uy  = u.y / l;
	const float uz  = u.z / l;
	
	m_mat *= float4x4(
		ux * ux * omc * c,      ux * uy * omc + uz * s, ux * uz * omc - uy * s, 0.f,
		ux * uy * omc - uz * s,      uy * uy * omc + c, uy * uz * omc - ux * s, 0.f,
		ux * uz * omc + uy * s, uy * uz * omc + ux * s,      uz * uz * omc + c, 0.f,
		                   0.f,                    0.f,                    0.f, 1.f);

	return *this;
}

Transform3D& Transform3D::rotate(float radian_x, float radian_y, float radian_z)
{
	return *this;
}

Transform3D& Transform3D::rotate(const float3& radian_xyz)
{
	return *this;
}

Transform3D& Transform3D::scale(float scale)
{
	m_mat[0][0] *= scale;
	m_mat[1][1] *= scale;
	m_mat[2][2] *= scale;

	return *this;
}

Transform3D& Transform3D::scale(float scale_x, float scale_y, float scale_z)
{
	m_mat[0][0] *= scale_x;
	m_mat[1][1] *= scale_y;
	m_mat[2][2] *= scale_z;

	return *this;
}

Transform3D& Transform3D::scale(const float3& scale)
{
	m_mat[0][0] *= scale.x;
	m_mat[1][1] *= scale.y;
	m_mat[2][2] *= scale.z;

	return *this;
}

Transform3D& Transform3D::clear()
{
	m_mat = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f };

	return *this;
}

const float4x4& Transform3D::getMatrix() const
{
	return m_mat;
}

Transform3D& Transform3D::operator*=(const Transform3D& rhs)
{
	m_mat *= rhs.m_mat;

	return *this;
}

Transform3D Transform3D::operator*(const Transform3D& rhs) const
{
	return m_mat * rhs.m_mat;
}

float3 Transform3D::operator*(const float3& rhs) const
{
	float4 v = m_mat * float4(rhs, 1.f);
	
	return { v.x / v.w, v.y / v.w, v.z / v.w };
}

bool Transform3D::operator==(const Transform3D& rhs) const
{
	return m_mat == rhs.m_mat;
}

bool Transform3D::operator!=(const Transform3D& rhs) const
{
	return m_mat != rhs.m_mat;
}

VERA_NAMESPACE_END