#pragma once

#include "../core/coredefs.h"

VERA_NAMESPACE_BEGIN

template <class T>
struct numeric_const
{
	static VERA_CONSTEXPR T zero = T();
	static VERA_CONSTEXPR T one  = ++T();
};

template <>
struct numeric_const<float>
{
	static VERA_CONSTEXPR float zero         = 0.f;
	static VERA_CONSTEXPR float one          = 1.f;
	static VERA_CONSTEXPR float pi           = 3.14159265358979323846264338327926900f;
	static VERA_CONSTEXPR float tau          = 6.28318530717958647692528676655900576f;
	static VERA_CONSTEXPR float e            = 2.71828182845904523536f;
	static VERA_CONSTEXPR float euler        = 0.577215664901532860606f;
	static VERA_CONSTEXPR float golden_ratio = 1.61803398874989484820458683436563811f;
	static VERA_CONSTEXPR float root2        = 1.41421356237309504880168872420969808f;
	static VERA_CONSTEXPR float root3        = 1.73205080756887729352744634150587236f;
	static VERA_CONSTEXPR float root5        = 2.23606797749978969640917366873127623f;
	static VERA_CONSTEXPR float ln2          = 0.69314718055994530941723212145817656f;
	static VERA_CONSTEXPR float lnln2        = -0.3665129205816643f;
	static VERA_CONSTEXPR float	ln10         = 2.30258509299404568401799145468436421f;
};

template <>
struct numeric_const<double>
{
	static VERA_CONSTEXPR double zero         = 0.;
	static VERA_CONSTEXPR double one          = 1.;
	static VERA_CONSTEXPR double pi           = 3.14159265358979323846264338327926900;
	static VERA_CONSTEXPR double tau          = 6.28318530717958647692528676655900576;
	static VERA_CONSTEXPR double e            = 2.71828182845904523536;
	static VERA_CONSTEXPR double euler        = 0.577215664901532860606;
	static VERA_CONSTEXPR double golden_ratio = 1.61803398874989484820458683436563811;
	static VERA_CONSTEXPR double root2        = 1.41421356237309504880168872420969808;
	static VERA_CONSTEXPR double root3        = 1.73205080756887729352744634150587236;
	static VERA_CONSTEXPR double root5        = 2.23606797749978969640917366873127623;
	static VERA_CONSTEXPR double ln2          = 0.69314718055994530941723212145817656;
	static VERA_CONSTEXPR double lnln2        = -0.3665129205816643;
	static VERA_CONSTEXPR double ln10         = 2.30258509299404568401799145468436421;
};

VERA_NAMESPACE_END