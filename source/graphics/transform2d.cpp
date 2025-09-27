#include "../../include/vera/graphics/transform2d.h"

VERA_NAMESPACE_BEGIN

Transform2D::Transform2D() :
	m_mat(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f) {}

Transform2D::Transform2D(const float3x4& mat) :
	m_mat(mat) {}

Transform2D& Transform2D::translate(float x, float y)
{
	m_mat[2][0] += m_mat[0][0] * x + m_mat[1][0] * y;
	m_mat[2][1] += m_mat[0][1] * x + m_mat[1][1] * y;
	m_mat[2][2] += m_mat[0][2] * x + m_mat[1][2] * y;

	return *this;
}

Transform2D& Transform2D::translate(const float2& offset)
{
	return Transform2D::translate(offset.x, offset.y);
}

Transform2D& Transform2D::rotate(radian rad)
{
	float3x3 copy = m_mat;
	float    cos  = cosf(rad);
	float    sin  = sinf(rad);

	// TODO: check calculation
	m_mat[0][0] = copy[0][0] * cos + copy[1][0] * -sin;
	m_mat[1][0] = copy[0][0] * sin + copy[1][0] * cos;
	m_mat[2][0] = copy[2][0];
	m_mat[0][1] = copy[0][1] * cos + copy[1][1] * -sin;
	m_mat[1][1] = copy[0][1] * sin + copy[1][1] * cos;
	m_mat[2][1] = copy[2][1];
	m_mat[0][2] = copy[0][2] * cos + copy[1][2] * -sin;
	m_mat[1][2] = copy[0][2] * sin + copy[1][2] * cos;
	m_mat[2][2] = copy[2][2];

	return *this;
}

Transform2D& Transform2D::rotate(radian rad, float center_x, float center_y)
{
	float3x3 copy = m_mat;
	float    cos  = cosf(rad);
	float    sin  = sinf(rad);
	float    c02  = center_x * (1 - cos) + center_y * sin;
	float    c12  = center_y * (1 - cos) - center_x * sin;

	// TODO: check calculation
	m_mat[0][0] = copy[0][0] * cos + copy[1][0] * -sin + copy[2][0] * c02;
	m_mat[1][0] = copy[0][0] * sin + copy[1][0] * cos + copy[2][0] * c12;
	m_mat[2][0] = copy[2][0];
	m_mat[0][1] = copy[0][1] * cos + copy[1][1] * -sin + copy[2][1] * c02;
	m_mat[1][1] = copy[0][1] * sin + copy[1][1] * cos + copy[2][1] * c12;
	m_mat[2][1] = copy[2][1];
	m_mat[0][2] = copy[0][2] * cos + copy[1][2] * -sin + copy[2][2] * c02;
	m_mat[1][2] = copy[0][2] * sin + copy[1][2] * cos + copy[2][2] * c12;
	m_mat[2][2] = copy[2][2];

	return *this;
}

Transform2D& Transform2D::rotate(radian rad, const float2& center)
{
	return Transform2D::rotate(rad, center.x, center.y);
}

Transform2D& Transform2D::scale(float scale)
{
	m_mat[0][0] *= scale;
	m_mat[0][1] *= scale;
	m_mat[0][2] *= scale;
	m_mat[1][0] *= scale;
	m_mat[1][1] *= scale;
	m_mat[1][2] *= scale;

	return *this;
}

Transform2D& Transform2D::scale(float scale_x, float scale_y)
{
	m_mat[0][0] *= scale_x;
	m_mat[0][1] *= scale_x;
	m_mat[0][2] *= scale_x;
	m_mat[1][0] *= scale_y;
	m_mat[1][1] *= scale_y;
	m_mat[1][2] *= scale_y;

	return *this;
}

Transform2D& Transform2D::scale(const float2& scale)
{
	return Transform2D::scale(scale.x, scale.y);
}

Transform2D& Transform2D::shear(radian rad_x, radian rad_y)
{
	auto t_x = tanf(rad_x);
	auto t_y = tanf(rad_y);

	m_mat[1][0] += t_x * m_mat[0][0];
	m_mat[1][1] += t_x * m_mat[0][1];
	m_mat[1][2] += t_x * m_mat[0][2];
	m_mat[0][0] += t_y * m_mat[1][0];
	m_mat[0][1] += t_y * m_mat[1][1];
	m_mat[0][2] += t_y * m_mat[1][2];

	return *this;
}

Transform2D& Transform2D::shearX(radian rad)
{
	auto t = tanf(rad);

	m_mat[1][0] += t * m_mat[0][0];
	m_mat[1][1] += t * m_mat[0][1];
	m_mat[1][2] += t * m_mat[0][2];

	return *this;
}

Transform2D& Transform2D::shearY(radian rad)
{
	auto t = tanf(rad);

	m_mat[0][0] += t * m_mat[1][0];
	m_mat[0][1] += t * m_mat[1][1];
	m_mat[0][2] += t * m_mat[1][2];
	
	return *this;
}

Transform2D& Transform2D::clear()
{
	m_mat = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f };
	
	return *this;
}

const float3x4& Transform2D::getMatrix() const
{
	return m_mat;
}

Transform2D& Transform2D::operator*=(const Transform2D& rhs)
{
	float3x3 copy = m_mat;

	m_mat[0][0] = copy[0][0] * rhs.m_mat[0][0] + copy[1][0] * rhs.m_mat[0][1] + copy[2][0] * rhs.m_mat[0][2];
	m_mat[1][0] = copy[0][0] * rhs.m_mat[1][0] + copy[1][0] * rhs.m_mat[1][1] + copy[2][0] * rhs.m_mat[1][2];
	m_mat[2][0] = copy[0][0] * rhs.m_mat[2][0] + copy[1][0] * rhs.m_mat[2][1] + copy[2][0] * rhs.m_mat[2][2];
	m_mat[0][1] = copy[0][1] * rhs.m_mat[0][0] + copy[1][1] * rhs.m_mat[0][1] + copy[2][1] * rhs.m_mat[0][2];
	m_mat[1][1] = copy[0][1] * rhs.m_mat[1][0] + copy[1][1] * rhs.m_mat[1][1] + copy[2][1] * rhs.m_mat[1][2];
	m_mat[2][1] = copy[0][1] * rhs.m_mat[2][0] + copy[1][1] * rhs.m_mat[2][1] + copy[2][1] * rhs.m_mat[2][2];
	m_mat[0][2] = copy[0][2] * rhs.m_mat[0][0] + copy[1][2] * rhs.m_mat[0][1] + copy[2][2] * rhs.m_mat[0][2];
	m_mat[1][2] = copy[0][2] * rhs.m_mat[1][0] + copy[1][2] * rhs.m_mat[1][1] + copy[2][2] * rhs.m_mat[1][2];
	m_mat[2][2] = copy[0][2] * rhs.m_mat[2][0] + copy[1][2] * rhs.m_mat[2][1] + copy[2][2] * rhs.m_mat[2][2];

	return *this;
}

Transform2D Transform2D::operator*(const Transform2D& rhs) const
{
	Transform2D result;

	result.m_mat[0][0] = m_mat[0][0] * rhs.m_mat[0][0] + m_mat[1][0] * rhs.m_mat[0][1] + m_mat[2][0] * rhs.m_mat[0][2];
	result.m_mat[1][0] = m_mat[0][0] * rhs.m_mat[1][0] + m_mat[1][0] * rhs.m_mat[1][1] + m_mat[2][0] * rhs.m_mat[1][2];
	result.m_mat[2][0] = m_mat[0][0] * rhs.m_mat[2][0] + m_mat[1][0] * rhs.m_mat[2][1] + m_mat[2][0] * rhs.m_mat[2][2];
	result.m_mat[0][1] = m_mat[0][1] * rhs.m_mat[0][0] + m_mat[1][1] * rhs.m_mat[0][1] + m_mat[2][1] * rhs.m_mat[0][2];
	result.m_mat[1][1] = m_mat[0][1] * rhs.m_mat[1][0] + m_mat[1][1] * rhs.m_mat[1][1] + m_mat[2][1] * rhs.m_mat[1][2];
	result.m_mat[2][1] = m_mat[0][1] * rhs.m_mat[2][0] + m_mat[1][1] * rhs.m_mat[2][1] + m_mat[2][1] * rhs.m_mat[2][2];
	result.m_mat[0][2] = m_mat[0][2] * rhs.m_mat[0][0] + m_mat[1][2] * rhs.m_mat[0][1] + m_mat[2][2] * rhs.m_mat[0][2];
	result.m_mat[1][2] = m_mat[0][2] * rhs.m_mat[1][0] + m_mat[1][2] * rhs.m_mat[1][1] + m_mat[2][2] * rhs.m_mat[1][2];
	result.m_mat[2][2] = m_mat[0][2] * rhs.m_mat[2][0] + m_mat[1][2] * rhs.m_mat[2][1] + m_mat[2][2] * rhs.m_mat[2][2];

	return result;
}

float2 Transform2D::operator*(const float2& rhs) const
{
	auto tmp = m_mat * float4(rhs.x, rhs.y, 1.f, 1.f);
	return { tmp.x / tmp.z, tmp.y / tmp.z };
}

bool Transform2D::operator==(const Transform2D& rhs) const
{
	return m_mat == rhs.m_mat;
}

bool Transform2D::operator!=(const Transform2D& rhs) const
{
	return m_mat != rhs.m_mat;
}

VERA_NAMESPACE_END
