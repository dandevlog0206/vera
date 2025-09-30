#pragma once

#include "vector_types.h"
#include <cmath>

VERA_NAMESPACE_BEGIN

class Quaternion
{
public:
	static VERA_NODISCARD VERA_INLINE Quaternion fromAxisAngle(float x, float y, float z, float angle) VERA_NOEXCEPT
	{
		const float half = angle * 0.5f;
		const float sin = sinf(half);
		const float cos = cosf(half);
		const float n2 = x * x + y * y + z * z;

		if (n2 <= 0.f)
			return { 0.f, 0.f, 0.f, cos };

		const float inv_len = 1.f / sqrtf(n2);
		const float sin_len = inv_len * sin;

		return { x * sin_len, y * sin_len, z * sin_len, cos };
	}

	VERA_CONSTEXPR Quaternion() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR Quaternion(const Quaternion& rhs) VERA_NOEXCEPT :
		x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}

	VERA_CONSTEXPR Quaternion(float x, float y, float z) VERA_NOEXCEPT :
		x(x), y(y), z(z), w(0.f) {}

	VERA_CONSTEXPR Quaternion(float x, float y, float z, float w) VERA_NOEXCEPT :
		x(x), y(y), z(z), w(w) {}

	VERA_CONSTEXPR Quaternion(const float3& v) VERA_NOEXCEPT :
		x(v.x), y(v.y), z(v.z), w(0.f) {}

	VERA_CONSTEXPR Quaternion(const float4& v) VERA_NOEXCEPT :
		x(v.x), y(v.y), z(v.z), w(v.w) {}

	VERA_CONSTEXPR Quaternion& operator=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		w = rhs.w;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR float normSquared() const VERA_NOEXCEPT
	{
		return x * x + y * y + z * z + w * w;
	}

	VERA_NODISCARD VERA_INLINE float norm() const VERA_NOEXCEPT
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	VERA_NODISCARD VERA_INLINE Quaternion normalize() const VERA_NOEXCEPT
	{
		Quaternion result;

		const float inv = 1.f / norm();
		
		result.w *= inv;
		result.x *= inv;
		result.y *= inv;
		result.z *= inv;
		
		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion conjugate() const VERA_NOEXCEPT
	{
		return { -x, -y, -z, w };
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator+(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator-(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator*(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		Quaternion result;

		const float x1 = x, y1 = y, z1 = z, w1 = w;
		const float x2 = rhs.x, y2 = rhs.y, z2 = rhs.z, w2 = rhs.w;

		result.x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
		result.y = w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2;
		result.z = w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2;
		result.w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;

		return result;
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator*(float scale) const VERA_NOEXCEPT
	{
		return { x * scale, y * scale, z * scale, w * scale };
	}

	friend VERA_NODISCARD VERA_CONSTEXPR Quaternion operator*(float scale, const Quaternion& rhs) VERA_NOEXCEPT
	{
		return { scale * rhs.x, scale * rhs.y, scale * rhs.z, scale * rhs.w };
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator/(float scale) const VERA_NOEXCEPT
	{
		return { x / scale, y / scale, z / scale, w / scale };
	}

	VERA_CONSTEXPR Quaternion operator+=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator-=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator*=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		const float x1 = x, y1 = y, z1 = z, w1 = w;
		const float x2 = rhs.x, y2 = rhs.y, z2 = rhs.z, w2 = rhs.w;

		x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
		y = w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2;
		z = w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2;
		w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator*=(float scale) VERA_NOEXCEPT
	{
		x *= scale;
		y *= scale;
		z *= scale;
		w *= scale;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator/=(float scale) VERA_NOEXCEPT
	{
		x /= scale;
		y /= scale;
		z /= scale;
		w /= scale;

		return *this;
	}

	VERA_NODISCARD VERA_INLINE bool equal(const Quaternion& rhs, float eps = 1e-7) const VERA_NOEXCEPT
	{
		return
			fabsf(x - rhs.x) < eps &&
			fabsf(y - rhs.y) < eps &&
			fabsf(z - rhs.z) < eps &&
			fabsf(w - rhs.w) < eps;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR operator float4() const VERA_NOEXCEPT
	{
		return { x, y, z, w };
	}

	float x;
	float y;
	float z;
	float w;
};

VERA_NAMESPACE_END
