#pragma once

#include "vector_types.h"
#include "matrix_types.h"
#include "radian.h"
#include <cmath>

VERA_NAMESPACE_BEGIN

class Quaternion
{
public:
	static VERA_NODISCARD VERA_INLINE Quaternion fromAxisAngle(float x, float y, float z, radian angle) VERA_NOEXCEPT
	{
		const float half = angle * 0.5f;
		const float sin  = sinf(half);
		const float cos  = cosf(half);
		const float n2   = x * x + y * y + z * z;

		if (n2 <= 0.f)
			return { 0.f, 0.f, 0.f, cos };

		const float inv_len = 1.f / sqrtf(n2);
		const float sin_len = inv_len * sin;

		return { cos, x * sin_len, y * sin_len, z * sin_len };
	}

	static VERA_NODISCARD VERA_INLINE Quaternion fromAxisAngle(const float3& u, radian angle) VERA_NOEXCEPT
	{
		return fromAxisAngle(u.x, u.y, u.z, angle);
	}

	VERA_CONSTEXPR Quaternion() VERA_NOEXCEPT :
		w(0.f), x(0.f), y(0.f), z(0.f) {}

	VERA_CONSTEXPR Quaternion(const Quaternion& rhs) VERA_NOEXCEPT :
		w(rhs.w), x(rhs.x), y(rhs.y), z(rhs.z) {}

	VERA_CONSTEXPR Quaternion(float x, float y, float z) VERA_NOEXCEPT :
		w(0.f), x(x), y(y), z(z) {}

	VERA_CONSTEXPR Quaternion(float w, float x, float y, float z) VERA_NOEXCEPT :
		w(w), x(x), y(y), z(z) {}

	VERA_CONSTEXPR Quaternion(const float3& v) VERA_NOEXCEPT :
		w(0.f), x(v.x), y(v.y), z(v.z) {}

	VERA_CONSTEXPR Quaternion(const float4& v) VERA_NOEXCEPT :
		w(v.w), x(v.x), y(v.y), z(v.z) {}

	VERA_CONSTEXPR Quaternion& operator=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		w = rhs.w;
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;

		return *this;
	}

	VERA_NODISCARD VERA_CONSTEXPR float normSquared() const VERA_NOEXCEPT
	{
		return w * w + x * x + y * y + z * z;
	}

	VERA_NODISCARD VERA_INLINE float norm() const VERA_NOEXCEPT
	{
		return sqrtf(w * w + x * x + y * y + z * z);
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
		return { w, -x, -y, -z };
	}

	VERA_NODISCARD VERA_CONSTEXPR float3x3 toMatrix3x3() const VERA_NOEXCEPT
	{
		return {
			1.f - 2.f * y * y - 2.f * z * z, 2.f * x * y + 2.f * z * w, 2.f * x * z - 2.f * y * w,
			2.f * x * y - 2.f * z * w, 1.f - 2.f * x * x - 2.f * z * z, 2.f * y * z - 2.f * x * w,
			2.f * x * z - 2.f * y * w, 2.f * y * z + 2.f * x * w, 1.f - 2.f * x * x - 2.f * y * y
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR float4x4 toMatrix4x4() const VERA_NOEXCEPT
	{
		return {
			1.f - 2.f * y * y - 2.f * z * z, 2.f * x * y + 2.f * z * w, 2.f * x * z - 2.f * y * w, 0.f,
			2.f * x * y - 2.f * z * w, 1.f - 2.f * x * x - 2.f * z * z, 2.f * y * z - 2.f * x * w, 0.f,
			2.f * x * z - 2.f * y * w, 2.f * y * z + 2.f * x * w, 1.f - 2.f * x * x - 2.f * y * y, 0.f,
			0.f, 0.f, 0.f, 1.f
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator+(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return { w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z };
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator-(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return { w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z };
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator*(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		Quaternion result;

		const float w2 = rhs.w, x2 = rhs.x, y2 = rhs.y, z2 = rhs.z;

		result.x = w * x2 + x * w2 + y * z2 - z * y2;
		result.y = w * y2 + y * w2 + z * x2 - x * z2;
		result.z = w * z2 + z * w2 + x * y2 - y * x2;
		result.w = w * w2 - x * x2 - y * y2 - z * z2;

		return result;
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator*(float scale) const VERA_NOEXCEPT
	{
		return { w * scale, x * scale, y * scale, z * scale };
	}

	friend VERA_NODISCARD VERA_CONSTEXPR Quaternion operator*(float scale, const Quaternion& rhs) VERA_NOEXCEPT
	{
		return { scale * rhs.w, scale * rhs.x, scale * rhs.y, scale * rhs.z };
	}

	VERA_NODISCARD VERA_CONSTEXPR Quaternion operator/(float scale) const VERA_NOEXCEPT
	{
		return { w / scale, x / scale, y / scale, z / scale };
	}

	VERA_CONSTEXPR Quaternion operator+=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		w += rhs.w;
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator-=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		w -= rhs.w;
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator*=(const Quaternion& rhs) VERA_NOEXCEPT
	{
		const float w1 = w, x1 = x, y1 = y, z1 = z;
		const float w2 = rhs.w, x2 = rhs.x, y2 = rhs.y, z2 = rhs.z;

		w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;
		x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
		y = w1 * y2 + y1 * w2 + z1 * x2 - x1 * z2;
		z = w1 * z2 + z1 * w2 + x1 * y2 - y1 * x2;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator*=(float scale) VERA_NOEXCEPT
	{
		w *= scale;
		x *= scale;
		y *= scale;
		z *= scale;

		return *this;
	}

	VERA_CONSTEXPR Quaternion& operator/=(float scale) VERA_NOEXCEPT
	{
		w /= scale;
		x /= scale;
		y /= scale;
		z /= scale;

		return *this;
	}

	VERA_NODISCARD VERA_INLINE bool equal(const Quaternion& rhs, float eps = 1e-7) const VERA_NOEXCEPT
	{
		return
			fabsf(w - rhs.w) < eps &&
			fabsf(x - rhs.x) < eps &&
			fabsf(y - rhs.y) < eps &&
			fabsf(z - rhs.z) < eps;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return w == rhs.w && x == rhs.x && y == rhs.y && z == rhs.z;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Quaternion& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}

	VERA_NODISCARD VERA_CONSTEXPR operator float4() const VERA_NOEXCEPT
	{
		return { w, x, y, z };
	}

	float w;
	float x;
	float y;
	float z;
};

VERA_NAMESPACE_END
