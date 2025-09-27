#pragma once

#include "../../include/vera/core/coredefs.h"

#define UNORM2F(x) ((x) / max_value_v<decltype(x)>)
#define SNORM2F(x) (0 < (x) ? (x) / max_value_v<decltype(x)> : (x) / min_value_v<decltype(x)>)
#define SSCALED2F(x) static_cast<float>(x)
#define USCALED2F(x) static_cast<float>(x)
#define SINT2F(x) static_cast<float>(x)
#define UINT2F(x) static_cast<float>(x)
#define DOUBLE2F(x) static_cast<float>(x)

#define SINT10(x) static_cast<int16_t>(((x) ^ 0x0200) - 0x0200)

VERA_NAMESPACE_BEGIN

template <class T>
static constexpr float min_value_v = NAN;
template <> static constexpr float min_value_v<int8_t>   = 128.f;
template <> static constexpr float min_value_v<int16_t>  = 32768.f;
template <> static constexpr float min_value_v<int32_t>  = 2147483648.f;
template <> static constexpr float min_value_v<int64_t>  = 9223372036854775808.f;

template <class T>
static constexpr float max_value_v = NAN;
template <> static constexpr float max_value_v<uint8_t>  = 256.f;
template <> static constexpr float max_value_v<int8_t>   = 127.f;
template <> static constexpr float max_value_v<uint16_t> = 65535.f;
template <> static constexpr float max_value_v<int16_t>  = 32767.f;
template <> static constexpr float max_value_v<uint32_t> = 4294967295.f;
template <> static constexpr float max_value_v<int32_t>  = 2147483647.f;
template <> static constexpr float max_value_v<uint64_t> = 18446744073709551615.f;
template <> static constexpr float max_value_v<int64_t>  = 9223372036854775807.f;

template <class T>
static const T& fetch_pixel(const void* ptr, uint32_t i)
{
	return reinterpret_cast<T*>(ptr) + i;
}

template <class T>
static T& fetch_pixel(void* ptr, uint32_t i)
{
	return reinterpret_cast<T*>(ptr) + i;
}

template <class T>
static const T& fetch_pixel(const void* ptr, uint32_t width, uint32_t x, uint32_t y)
{
	return reinterpret_cast<T*>(ptr) + width * y + x;
}

template <class T>
static T& fetch_pixel(void* ptr, uint32_t width, uint32_t x, uint32_t y)
{
	return reinterpret_cast<T*>(ptr) + width * y + x;
}

VERA_NAMESPACE_END
