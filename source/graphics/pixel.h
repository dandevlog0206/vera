#pragma once

#include "../../include/vera/core/assertion.h"
#include "../../include/vera/math/vector_types.h"

#define NOALPHA 1.f

#define UNORM2F(x) ((x) / max_value_v<decltype(x)>)
#define SNORM2F(x) (0 < (x) ? (x) / max_value_v<decltype(x)> : (x) / min_value_v<decltype(x)>)
#define USCALED2F(x) static_cast<float>(x)
#define SSCALED2F(x) static_cast<float>(x)
#define UINT2F(x) static_cast<float>(x)
#define SINT2F(x) static_cast<float>(x)
#define DOUBLE2F(x) static_cast<float>(x)

#define F2UNORM(type, x) (static_cast<type>((x) * (max_value_v<type> + 0.99f)))
#define F2SNORM(type, x) (static_cast<type>((x) * (0.f < (x) ? (max_value_v<type> + 0.99f) : (min_value_v<type> + 0.99f))))
#define F2SSCALED(type, x) static_cast<type>(x)
#define F2USCALED(type, x) static_cast<type>(x)
#define F2UINT(type, x) static_cast<type>(x)
#define F2SINT(type, x) static_cast<type>(x)
#define F2DOUBLE(x) static_cast<double>(x)

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
template <> static constexpr float max_value_v<uint8_t>  = 255.f;
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
	return *(reinterpret_cast<const T*>(ptr) + width * y + x);
}

template <class T>
static T& fetch_pixel(void* ptr, uint32_t width, uint32_t x, uint32_t y)
{
	return *(reinterpret_cast<T*>(ptr) + width * y + x);
}

static float4 fetch_components(const void* ptr, uint32_t width, uint32_t x, uint32_t y, Format format)
{
	float4 result;

	switch (format) {
	case Format::R8Unorm: {
		auto pixel = fetch_pixel<uint8_t>(ptr, width, x, y);
		return { UNORM2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R8Snorm: {
		auto pixel = fetch_pixel<int8_t>(ptr, width, x, y);
		return { SNORM2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R8Uscaled: {
		auto pixel = fetch_pixel<uint8_t>(ptr, width, x, y);
		return { USCALED2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R8Sscaled: {
		auto pixel = fetch_pixel<int8_t>(ptr, width, x, y);
		return { SSCALED2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R8Uint: {
		auto pixel = fetch_pixel<uint8_t>(ptr, width, x, y);
		return { UINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R8Sint: {
		auto pixel = fetch_pixel<int8_t>(ptr, width, x, y);
		return { SINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	//case Format::R8Srgb: {
	// 
	//}
	case Format::R16Unorm: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return { UNORM2F(pixel), 0.f, 0.f, NOALPHA};
	}
	case Format::R16Snorm: {
		auto pixel = fetch_pixel<int16_t>(ptr, width, x, y);
		return { SNORM2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R16Uscaled: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return { USCALED2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R16Sscaled: {
		auto pixel = fetch_pixel<int16_t>(ptr, width, x, y);
		return { SSCALED2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R16Uint: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return { UINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R16Sint: {
		auto pixel = fetch_pixel<int16_t>(ptr, width, x, y);
		return { SINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	//case Format::R16Float: {
	//
	//}
	case Format::R32Uint: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return { UINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R32Sint: {
		auto pixel = fetch_pixel<int32_t>(ptr, width, x, y);
		return { SINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R32Float: {
		auto pixel = fetch_pixel<float>(ptr, width, x, y);
		return { pixel, 0.f, 0.f, NOALPHA };
	}
	case Format::R64Uint: {
		auto pixel = fetch_pixel<uint64_t>(ptr, width, x, y);
		return { UINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R64Sint: {
		auto pixel = fetch_pixel<int64_t>(ptr, width, x, y);
		return { SINT2F(pixel), 0.f, 0.f, NOALPHA };
	}
	case Format::R64Float: {
		auto pixel = fetch_pixel<double>(ptr, width, x, y);
		return { DOUBLE2F(pixel), 0.f, 0.f, NOALPHA};
	}
	case Format::RG8Unorm: {
		auto pixel = fetch_pixel<uchar2>(ptr, width, x, y);
		return { UNORM2F(pixel.r), UNORM2F(pixel.g), 0.f, NOALPHA};
	}
	case Format::RG8Snorm: {
		auto pixel = fetch_pixel<char2>(ptr, width, x, y);
		return { SNORM2F(pixel.r), SNORM2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG8Uscaled: {
		auto pixel = fetch_pixel<uchar2>(ptr, width, x, y);
		return { USCALED2F(pixel.r), USCALED2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG8Sscaled: {
		auto pixel = fetch_pixel<char2>(ptr, width, x, y);
		return { SSCALED2F(pixel.r), SSCALED2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG8Uint: {
		auto pixel = fetch_pixel<uchar2>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG8Sint: {
		auto pixel = fetch_pixel<char2>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), 0.f, NOALPHA };
	}
	//case Format::RG8Srgb: {
	//
	//}
	case Format::RG16Unorm: {
		auto pixel = fetch_pixel<ushort2>(ptr, width, x, y);
		return { UNORM2F(pixel.r), UNORM2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG16Snorm: {
		auto pixel = fetch_pixel<short2>(ptr, width, x, y);
		return { SNORM2F(pixel.r), SNORM2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG16Uscaled: {
		auto pixel = fetch_pixel<ushort2>(ptr, width, x, y);
		return { USCALED2F(pixel.r), USCALED2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG16Sscaled: {
		auto pixel = fetch_pixel<short2>(ptr, width, x, y);
		return { SSCALED2F(pixel.r), SSCALED2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG16Uint: {
		auto pixel = fetch_pixel<ushort2>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG16Sint: {
		auto pixel = fetch_pixel<short2>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), 0.f, NOALPHA };
	}
	//case Format::RG16Float: {
	//
	//}
	case Format::RG32Uint: {
		auto pixel = fetch_pixel<uint2>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG32Sint: {
		auto pixel = fetch_pixel<int2>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG32Float: {
		auto pixel = fetch_pixel<float2>(ptr, width, x, y);
		return { pixel.r, pixel.g, 0.f, NOALPHA };
	}
	case Format::RG64Uint: {
		auto pixel = fetch_pixel<ulong2>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG64Sint: {
		auto pixel = fetch_pixel<long2>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RG64Float: {
		auto pixel = fetch_pixel<double2>(ptr, width, x, y);
		return { DOUBLE2F(pixel.r), DOUBLE2F(pixel.g), 0.f, NOALPHA };
	}
	case Format::RGB8Unorm: {
		auto pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		return { UNORM2F(pixel.r), UNORM2F(pixel.g), UNORM2F(pixel.b), NOALPHA};
	}
	case Format::RGB8Snorm: {
		auto pixel = fetch_pixel<char3>(ptr, width, x, y);
		return { SNORM2F(pixel.r), SNORM2F(pixel.g), SNORM2F(pixel.b), NOALPHA };
	}
	case Format::RGB8Uscaled: {
		auto pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		return { USCALED2F(pixel.r), USCALED2F(pixel.g), USCALED2F(pixel.b), NOALPHA };
	}
	case Format::RGB8Sscaled: {
		auto pixel = fetch_pixel<char3>(ptr, width, x, y);
		return { SSCALED2F(pixel.r), SSCALED2F(pixel.g), SSCALED2F(pixel.b), NOALPHA };
	}
	case Format::RGB8Uint: {
		auto pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), NOALPHA };
	}
	case Format::RGB8Sint: {
		auto pixel = fetch_pixel<char3>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), NOALPHA };
	}
	//case Format::RGB8Srgb: {
	//
	//}
	case Format::BGR8Unorm: {
		auto pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		return { UNORM2F(pixel.b), UNORM2F(pixel.g), UNORM2F(pixel.r), NOALPHA };
	}
	case Format::BGR8Snorm: {
		auto pixel = fetch_pixel<char3>(ptr, width, x, y);
		return { SNORM2F(pixel.b), SNORM2F(pixel.g), SNORM2F(pixel.r), NOALPHA };
	}
	case Format::BGR8Uscaled: {
		auto pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		return { USCALED2F(pixel.b), USCALED2F(pixel.g), USCALED2F(pixel.r), NOALPHA };
	}
	case Format::BGR8Sscaled: {
		auto pixel = fetch_pixel<char3>(ptr, width, x, y);
		return { SSCALED2F(pixel.b), SSCALED2F(pixel.g), SSCALED2F(pixel.r), NOALPHA };
	}
	case Format::BGR8Uint: {
		auto pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		return { UINT2F(pixel.b), UINT2F(pixel.g), UINT2F(pixel.r), NOALPHA };
	}
	case Format::BGR8Sint: {
		auto pixel = fetch_pixel<char3>(ptr, width, x, y);
		return { SINT2F(pixel.b), SINT2F(pixel.g), SINT2F(pixel.r), NOALPHA };
	}
	//case Format::BGR8Srgb: {
	//
	//}
	case Format::RGB16Unorm: {
		auto pixel = fetch_pixel<ushort3>(ptr, width, x, y);
		return { UNORM2F(pixel.r), UNORM2F(pixel.g), UNORM2F(pixel.b), NOALPHA };
	}
	case Format::RGB16Snorm: {
		auto pixel = fetch_pixel<short3>(ptr, width, x, y);
		return { SNORM2F(pixel.r), SNORM2F(pixel.g), SNORM2F(pixel.b), NOALPHA };
	}
	case Format::RGB16Uscaled: {
		auto pixel = fetch_pixel<ushort3>(ptr, width, x, y);
		return { USCALED2F(pixel.r), USCALED2F(pixel.g), USCALED2F(pixel.b), NOALPHA };
	}
	case Format::RGB16Sscaled: {
		auto pixel = fetch_pixel<short3>(ptr, width, x, y);
		return { SSCALED2F(pixel.r), SSCALED2F(pixel.g), SSCALED2F(pixel.b), NOALPHA };
	}
	case Format::RGB16Uint: {
		auto pixel = fetch_pixel<ushort3>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), NOALPHA };
	}
	case Format::RGB16Sint: {
		auto pixel = fetch_pixel<short3>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), NOALPHA };
	}
	//case Format::RGB16Float: {
	//
	//}
	case Format::RGB32Uint: {
		auto pixel = fetch_pixel<uint3>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), NOALPHA };
	}
	case Format::RGB32Sint: {
		auto pixel = fetch_pixel<int3>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), NOALPHA };
	}
	case Format::RGB32Float: {
		auto pixel = fetch_pixel<float3>(ptr, width, x, y);
		return { pixel.r, pixel.g, pixel.b, NOALPHA };
	}
	case Format::RGB64Uint: {
		auto pixel = fetch_pixel<ulong3>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), NOALPHA };
	}
	case Format::RGB64Sint: {
		auto pixel = fetch_pixel<long3>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), NOALPHA };
	}
	case Format::RGB64Float: {
		auto pixel = fetch_pixel<double3>(ptr, width, x, y);
		return { DOUBLE2F(pixel.r), DOUBLE2F(pixel.g), DOUBLE2F(pixel.b), NOALPHA };
	}
	case Format::RGBA8Unorm: {
		auto pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		return { UNORM2F(pixel.r), UNORM2F(pixel.g), UNORM2F(pixel.b), UNORM2F(pixel.a)};
	}
	case Format::RGBA8Snorm: {
		auto pixel = fetch_pixel<char4>(ptr, width, x, y);
		return { SNORM2F(pixel.r), SNORM2F(pixel.g), SNORM2F(pixel.b), SNORM2F(pixel.a) };
	}
	case Format::RGBA8Uscaled: {
		auto pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		return { USCALED2F(pixel.r), USCALED2F(pixel.g), USCALED2F(pixel.b), USCALED2F(pixel.a) };
	}
	case Format::RGBA8Sscaled: {
		auto pixel = fetch_pixel<char4>(ptr, width, x, y);
		return { SSCALED2F(pixel.r), SSCALED2F(pixel.g), SSCALED2F(pixel.b), SSCALED2F(pixel.a) };
	}
	case Format::RGBA8Uint: {
		auto pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), UINT2F(pixel.a) };
	}
	case Format::RGBA8Sint: {
		auto pixel = fetch_pixel<char4>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), SINT2F(pixel.a) };
	}
	//case Format::RGBA8Srgb: {
	//
	//}
	case Format::BGRA8Unorm: {
		auto pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		return { UNORM2F(pixel.b), UNORM2F(pixel.g), UNORM2F(pixel.r), UNORM2F(pixel.a) };
	}
	case Format::BGRA8Snorm: {
		auto pixel = fetch_pixel<char4>(ptr, width, x, y);
		return { SNORM2F(pixel.b), SNORM2F(pixel.g), SNORM2F(pixel.r), SNORM2F(pixel.a) };
	}
	case Format::BGRA8Uscaled: {
		auto pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		return { USCALED2F(pixel.b), USCALED2F(pixel.g), USCALED2F(pixel.r), USCALED2F(pixel.a) };
	}
	case Format::BGRA8Sscaled: {
		auto pixel = fetch_pixel<char4>(ptr, width, x, y);
		return { SSCALED2F(pixel.b), SSCALED2F(pixel.g), SSCALED2F(pixel.r), SSCALED2F(pixel.a) };
	}
	case Format::BGRA8Uint: {
		auto pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		return { UINT2F(pixel.b), UINT2F(pixel.g), UINT2F(pixel.r), UINT2F(pixel.a) };
	}
	case Format::BGRA8Sint: {
		auto pixel = fetch_pixel<char4>(ptr, width, x, y);
		return { SINT2F(pixel.b), SINT2F(pixel.g), SINT2F(pixel.r), SINT2F(pixel.a) };
	}
	//case Format::BGRA8Srgb: {
	//
	//}
	case Format::RGBA16Unorm: {
		auto pixel = fetch_pixel<ushort4>(ptr, width, x, y);
		return { UNORM2F(pixel.r), UNORM2F(pixel.g), UNORM2F(pixel.b), UNORM2F(pixel.a) };
	}
	case Format::RGBA16Snorm: {
		auto pixel = fetch_pixel<short4>(ptr, width, x, y);
		return { SNORM2F(pixel.r), SNORM2F(pixel.g), SNORM2F(pixel.b), SNORM2F(pixel.a) };
	}
	case Format::RGBA16Uscaled: {
		auto pixel = fetch_pixel<ushort4>(ptr, width, x, y);
		return { USCALED2F(pixel.r), USCALED2F(pixel.g), USCALED2F(pixel.b), USCALED2F(pixel.a) };
	}
	case Format::RGBA16Sscaled: {
		auto pixel = fetch_pixel<short4>(ptr, width, x, y);
		return { SSCALED2F(pixel.r), SSCALED2F(pixel.g), SSCALED2F(pixel.b), SSCALED2F(pixel.a) };
	}
	case Format::RGBA16Uint: {
		auto pixel = fetch_pixel<ushort4>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), UINT2F(pixel.a) };
	}
	case Format::RGBA16Sint: {
		auto pixel = fetch_pixel<short4>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), SINT2F(pixel.a) };
	}
	//case Format::RGBA16Float: {
	//
	//}
	case Format::RGBA32Uint: {
		auto pixel = fetch_pixel<uint4>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), UINT2F(pixel.a) };
	}
	case Format::RGBA32Sint: {
		auto pixel = fetch_pixel<int4>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), SINT2F(pixel.a) };
	}
	case Format::RGBA32Float: {
		auto pixel = fetch_pixel<float4>(ptr, width, x, y);
		return { pixel.r, pixel.g, pixel.b, pixel.a };
	}
	case Format::RGBA64Uint: {
		auto pixel = fetch_pixel<ulong4>(ptr, width, x, y);
		return { UINT2F(pixel.r), UINT2F(pixel.g), UINT2F(pixel.b), UINT2F(pixel.a) };
	}
	case Format::RGBA64Sint: {
		auto pixel = fetch_pixel<long4>(ptr, width, x, y);
		return { SINT2F(pixel.r), SINT2F(pixel.g), SINT2F(pixel.b), SINT2F(pixel.a) };
	}
	case Format::RGBA64Float: {
		auto pixel = fetch_pixel<double4>(ptr, width, x, y);
		return { DOUBLE2F(pixel.r), DOUBLE2F(pixel.g), DOUBLE2F(pixel.b), DOUBLE2F(pixel.a) };
	}
	case Format::RG4UnormPack8: {
		auto pixel = fetch_pixel<uint8_t>(ptr, width, x, y);
		return {
			(pixel & 0xf0 >> 4) / 15.f,
			(pixel & 0x0f >> 0) / 15.f,
			0.f,
			NOALPHA
		};
	}
	case Format::RGBA4UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0xf000 >> 12) / 15.f,
			(pixel & 0x0f00 >> 8) / 15.f,
			(pixel & 0x00f0 >> 4) / 15.f,
			(pixel & 0x000f >> 0) / 15.f
		};
	}
	case Format::BGRA4UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0x00f0 >> 4) / 15.f,
			(pixel & 0x0f00 >> 8) / 15.f,
			(pixel & 0xf000 >> 12) / 15.f,
			(pixel & 0x000f >> 0) / 15.f
		};
	}
	case Format::R5G6B5UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0xf800 >> 11) / 31.f,
			(pixel & 0x07e0 >> 5) / 63.f,
			(pixel & 0x001f >> 0) / 31.f,
			NOALPHA
		};
	}
	case Format::B5G6R5UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0x001f >> 0) / 31.f,
			(pixel & 0x07e0 >> 5) / 63.f,
			(pixel & 0xf800 >> 11) / 31.f,
			NOALPHA
		};
	}
	case Format::R5G5B5A1UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0xf800 >> 11) / 31.f,
			(pixel & 0x07c0 >> 6) / 31.f,
			(pixel & 0x003e >> 1) / 31.f,
			static_cast<float>(pixel & 1)
		};
	}
	case Format::B5G5R5A1UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0x003e >> 1) / 31.f,
			(pixel & 0x07c0 >> 6) / 31.f,
			(pixel & 0xf800 >> 11) / 31.f,
			static_cast<float>(pixel & 1)
		};
	}
	case Format::A1R5G5B5UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0x7c00 >> 10) / 31.f,
			(pixel & 0x03e0 >> 5) / 31.f,
			(pixel & 0x001f >> 0) / 31.f,
			static_cast<float>(pixel & 0x8000 >> 15)
		};
	}
	case Format::A4R4G4B4UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0x0f00 >> 8) / 15.f,
			(pixel & 0x00f0 >> 4) / 15.f,
			(pixel & 0x000f >> 0) / 15.f,
			(pixel & 0xf000 >> 12) / 15.f
		};
	}
	case Format::A4B4G4R4UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0x000f >> 0) / 15.f,
			(pixel & 0x00f0 >> 4) / 15.f,
			(pixel & 0x0f00 >> 8) / 15.f,
			(pixel & 0xf000 >> 12) / 15.f
		};
	}
	case Format::A1B5G5R5UnormPack16: {
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
		return {
			(pixel & 0x001f >> 0) / 31.f,
			(pixel & 0x7c00 >> 10) / 31.f,
			(pixel & 0x03e0 >> 5) / 31.f,
			static_cast<float>(pixel & 0x8000 >> 15)
		};
	}
	case Format::ABGR8UnormPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			UNORM2F(static_cast<uint8_t>(pixel & 0x000000ff >> 0)),
			UNORM2F(static_cast<uint8_t>(pixel & 0x0000ff00 >> 8)),
			UNORM2F(static_cast<uint8_t>(pixel & 0x00ff0000 >> 16)),
			UNORM2F(static_cast<uint8_t>(pixel & 0xff000000 >> 24))
		};
	}
	case Format::ABGR8SnormPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SNORM2F(static_cast<int8_t>(pixel & 0x000000ff >> 0)),
			SNORM2F(static_cast<int8_t>(pixel & 0x0000ff00 >> 8)),
			SNORM2F(static_cast<int8_t>(pixel & 0x00ff0000 >> 16)),
			SNORM2F(static_cast<int8_t>(pixel & 0xff000000 >> 24))
		};
	}
	case Format::ABGR8UscaledPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			USCALED2F(static_cast<uint8_t>(pixel & 0x000000ff >> 0)),
			USCALED2F(static_cast<uint8_t>(pixel & 0x0000ff00 >> 8)),
			USCALED2F(static_cast<uint8_t>(pixel & 0x00ff0000 >> 16)),
			USCALED2F(static_cast<uint8_t>(pixel & 0xff000000 >> 24))
		};
	}
	case Format::ABGR8SscaledPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SSCALED2F(static_cast<int8_t>(pixel & 0x000000ff >> 0)),
			SSCALED2F(static_cast<int8_t>(pixel & 0x0000ff00 >> 8)),
			SSCALED2F(static_cast<int8_t>(pixel & 0x00ff0000 >> 16)),
			SSCALED2F(static_cast<int8_t>(pixel & 0xff000000 >> 24))
		};
	}
	case Format::ABGR8UintPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			UINT2F(static_cast<uint8_t>(pixel & 0x000000ff >> 0)),
			UINT2F(static_cast<uint8_t>(pixel & 0x0000ff00 >> 8)),
			UINT2F(static_cast<uint8_t>(pixel & 0x00ff0000 >> 16)),
			UINT2F(static_cast<uint8_t>(pixel & 0xff000000 >> 24))
		};
	}
	case Format::ABGR8SintPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SINT2F(static_cast<int8_t>(pixel & 0x000000ff >> 0)),
			SINT2F(static_cast<int8_t>(pixel & 0x0000ff00 >> 8)),
			SINT2F(static_cast<int8_t>(pixel & 0x00ff0000 >> 16)),
			SINT2F(static_cast<int8_t>(pixel & 0xff000000 >> 24))
		};
	}
	//case Format::ABGR8SrgbPack32: {
	//
	//}
	case Format::A2RGB10UnormPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			(pixel & 0x3ff00000 >> 20) / 1023.f,
			(pixel & 0x000ffc00 >> 10) / 1023.f,
			(pixel & 0x000003ff >> 0) / 1023.f,
			(pixel & 0xc0000000 >> 30) / 3.f,
		};
	}
	case Format::A2RGB10SnormPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SNORM2F(SINT10(pixel & 0x3ff00000 >> 20)),
			SNORM2F(SINT10(pixel & 0x000ffc00 >> 10)),
			SNORM2F(SINT10(pixel & 0x000003ff >> 0)),
			SNORM2F(SINT10(pixel & 0xc0000000 >> 30)),
		};
	}
	case Format::A2RGB10UscaledPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			USCALED2F(pixel & 0x3ff00000 >> 20),
			USCALED2F(pixel & 0x000ffc00 >> 10),
			USCALED2F(pixel & 0x000003ff >> 0),
			USCALED2F(pixel & 0xc0000000 >> 30),
		};
	}
	case Format::A2RGB10SscaledPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SSCALED2F(SINT10(pixel & 0x3ff00000 >> 20)),
			SSCALED2F(SINT10(pixel & 0x000ffc00 >> 10)),
			SSCALED2F(SINT10(pixel & 0x000003ff >> 0)),
			SSCALED2F(SINT10(pixel & 0xc0000000 >> 30)),
		};
	}
	case Format::A2RGB10UintPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			UINT2F(pixel & 0x3ff00000 >> 20),
			UINT2F(pixel & 0x000ffc00 >> 10),
			UINT2F(pixel & 0x000003ff >> 0),
			UINT2F(pixel & 0xc0000000 >> 30),
		};
	}
	case Format::A2RGB10SintPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SINT2F(SINT10(pixel & 0x3ff00000 >> 20)),
			SINT2F(SINT10(pixel & 0x000ffc00 >> 10)),
			SINT2F(SINT10(pixel & 0x000003ff >> 0)),
			SINT2F(SINT10(pixel & 0xc0000000 >> 30)),
		};
	}
	case Format::A2BGR10UnormPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			(pixel & 0x000003ff >> 0) / 1023.f,
			(pixel & 0x000ffc00 >> 10) / 1023.f,
			(pixel & 0x3ff00000 >> 20) / 1023.f,
			(pixel & 0xc0000000 >> 30) / 3.f,
		};
	}
	case Format::A2BGR10SnormPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SNORM2F(SINT10(pixel & 0x000003ff >> 0)),
			SNORM2F(SINT10(pixel & 0x000ffc00 >> 10)),
			SNORM2F(SINT10(pixel & 0x3ff00000 >> 20)),
			SNORM2F(SINT10(pixel & 0xc0000000 >> 30)),
		};
	}
	case Format::A2BGR10UscaledPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			USCALED2F(pixel & 0x000003ff >> 0),
			USCALED2F(pixel & 0x000ffc00 >> 10),
			USCALED2F(pixel & 0x3ff00000 >> 20),
			USCALED2F(pixel & 0xc0000000 >> 30)
		};
	}
	case Format::A2BGR10SscaledPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SSCALED2F(SINT10(pixel & 0x000003ff >> 0)),
			SSCALED2F(SINT10(pixel & 0x000ffc00 >> 10)),
			SSCALED2F(SINT10(pixel & 0x3ff00000 >> 20)),
			SSCALED2F(SINT10(pixel & 0xc0000000 >> 30)),
		};
	}
	case Format::A2BGR10UintPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			UINT2F(pixel & 0x000003ff >> 0),
			UINT2F(pixel & 0x000ffc00 >> 10),
			UINT2F(pixel & 0x3ff00000 >> 20),
			UINT2F(pixel & 0xc0000000 >> 30)
		};
	}
	case Format::A2BGR10SintPack32: {
		auto pixel = fetch_pixel<uint32_t>(ptr, width, x, y);
		return {
			SINT2F(SINT10(pixel & 0x000003ff >> 0)),
			SINT2F(SINT10(pixel & 0x000ffc00 >> 10)),
			SINT2F(SINT10(pixel & 0x3ff00000 >> 20)),
			SINT2F(SINT10(pixel & 0xc0000000 >> 30)),
		};
	}
	case Format::A8Unorm: {
		auto pixel = fetch_pixel<uint8_t>(ptr, width, x, y);
		return { 0.f, 0.f, 0.f, UNORM2F(pixel) };
	}
	}

	VERA_ASSERT_MSG(false, "unsupported format");
	return {};
}

static void store_components(void* ptr, uint32_t width, uint32_t x, uint32_t y, Format format, const float4& value)
{
	switch (format) {
	case Format::R8Unorm: {
		fetch_pixel<uint8_t>(ptr, width, x, y) = F2UNORM(uint8_t, value.r);
	} return;
	case Format::R8Snorm: {
		fetch_pixel<int8_t>(ptr, width, x, y) = F2SNORM(int8_t, value.r);
	} return;
	case Format::R8Uscaled: {
		fetch_pixel<uint8_t>(ptr, width, x, y) = F2USCALED(uint8_t, value.r);
	} return;
	case Format::R8Sscaled: {
		fetch_pixel<int8_t>(ptr, width, x, y) = F2SSCALED(int8_t, value.r);
	} return;
	case Format::R8Uint: {
		fetch_pixel<uint8_t>(ptr, width, x, y) = F2UINT(uint8_t, value.r);
	} return;
	case Format::R8Sint: {
		fetch_pixel<int8_t>(ptr, width, x, y) = F2SINT(int8_t, value.r);
	} return;
	//case Format::R8Srgb: {
	// 
	//} return;
	case Format::R16Unorm: {
		fetch_pixel<uint16_t>(ptr, width, x, y) = F2UNORM(uint16_t, value.r);
	} return;
	case Format::R16Snorm: {
		fetch_pixel<int16_t>(ptr, width, x, y) = F2SNORM(int16_t, value.r);
	} return;
	case Format::R16Uscaled: {
		fetch_pixel<uint16_t>(ptr, width, x, y) = F2USCALED(uint16_t, value.r);
	} return;
	case Format::R16Sscaled: {
		fetch_pixel<int16_t>(ptr, width, x, y) = F2SSCALED(uint16_t, value.r);
	} return;
	case Format::R16Uint: {
		fetch_pixel<uint16_t>(ptr, width, x, y) = F2UINT(uint16_t, value.r);
	} return;
	case Format::R16Sint: {
		fetch_pixel<int16_t>(ptr, width, x, y) = F2SINT(int16_t, value.r);
	} return;
	//case Format::R16Float: {
	//
	//} return;
	case Format::R32Uint: {
		fetch_pixel<uint32_t>(ptr, width, x, y) = F2UINT(uint32_t, value.r);
	} return;
	case Format::R32Sint: {
		fetch_pixel<int32_t>(ptr, width, x, y) = F2SINT(int32_t, value.r);
	} return;
	case Format::R32Float: {
		fetch_pixel<float>(ptr, width, x, y) = value.r;
	} return;
	case Format::R64Uint: {
		fetch_pixel<uint64_t>(ptr, width, x, y) = F2UINT(uint64_t, value.r);;
	} return;
	case Format::R64Sint: {
		fetch_pixel<int64_t>(ptr, width, x, y) = F2UINT(int64_t, value.r);
	} return;
	case Format::R64Float: {
		fetch_pixel<double>(ptr, width, x, y) = F2DOUBLE(value.r);
	} return;
	case Format::RG8Unorm: {
		auto& pixel = fetch_pixel<uchar2>(ptr, width, x, y);
		pixel.r = F2UNORM(uint8_t, value.r);
		pixel.g = F2UNORM(uint8_t, value.g);
	} return;
	case Format::RG8Snorm: {
		auto& pixel = fetch_pixel<char2>(ptr, width, x, y);
		pixel.r = F2UNORM(int8_t, value.r);
		pixel.g = F2UNORM(int8_t, value.g);
	} return;
	case Format::RG8Uscaled: {
		auto& pixel = fetch_pixel<uchar2>(ptr, width, x, y);
		pixel.r = F2USCALED(uint8_t, value.r);
		pixel.g = F2USCALED(uint8_t, value.g);
	} return;
	case Format::RG8Sscaled: {
		auto& pixel = fetch_pixel<char2>(ptr, width, x, y);
		pixel.r = F2SSCALED(int8_t, value.r);
		pixel.g = F2SSCALED(int8_t, value.g);
	} return;
	case Format::RG8Uint: {
		auto& pixel = fetch_pixel<uchar2>(ptr, width, x, y);
		pixel.r = F2UINT(uint8_t, value.r);
		pixel.g = F2UINT(uint8_t, value.g);
	} return;
	case Format::RG8Sint: {
		auto& pixel = fetch_pixel<char2>(ptr, width, x, y);
		pixel.r = F2SINT(int8_t, value.r);
		pixel.g = F2SINT(int8_t, value.g);
	} return;
	//case Format::RG8Srgb: {
	//
	//} return;
	case Format::RG16Unorm: {
		auto& pixel = fetch_pixel<ushort2>(ptr, width, x, y);
		pixel.r = F2UNORM(uint16_t, value.r);
		pixel.g = F2UNORM(uint16_t, value.g);
	} return;
	case Format::RG16Snorm: {
		auto& pixel = fetch_pixel<short2>(ptr, width, x, y);
		pixel.r = F2SNORM(int16_t, value.r);
		pixel.g = F2SNORM(int16_t, value.g);
	} return;
	case Format::RG16Uscaled: {
		auto& pixel = fetch_pixel<ushort2>(ptr, width, x, y);
		pixel.r = F2USCALED(uint16_t, value.r);
		pixel.g = F2USCALED(uint16_t, value.g);
	} return;
	case Format::RG16Sscaled: {
		auto& pixel = fetch_pixel<short2>(ptr, width, x, y);
		pixel.r = F2SSCALED(int16_t, value.r);
		pixel.g = F2SSCALED(int16_t, value.g);
	} return;
	case Format::RG16Uint: {
		auto& pixel = fetch_pixel<ushort2>(ptr, width, x, y);
		pixel.r = F2UINT(uint16_t, value.r);
		pixel.g = F2UINT(uint16_t, value.g);
	} return;
	case Format::RG16Sint: {
		auto& pixel = fetch_pixel<short2>(ptr, width, x, y);
		pixel.r = F2SINT(int16_t, value.r);
		pixel.g = F2SINT(int16_t, value.g);
	} return;
	//case Format::RG16Float: {
	//
	//} return;
	case Format::RG32Uint: {
		auto& pixel = fetch_pixel<uint2>(ptr, width, x, y);
		pixel.r = F2UINT(uint32_t, value.r);
		pixel.g = F2UINT(uint32_t, value.g);
	} return;
	case Format::RG32Sint: {
		auto& pixel = fetch_pixel<int2>(ptr, width, x, y);
		pixel.r = F2SINT(int32_t, value.r);
		pixel.g = F2SINT(int32_t, value.g);
	} return;
	case Format::RG32Float: {
		auto& pixel = fetch_pixel<float2>(ptr, width, x, y);
		pixel.r = value.r;
		pixel.g = value.g;
	} return;
	case Format::RG64Uint: {
		auto& pixel = fetch_pixel<ulong2>(ptr, width, x, y);
		pixel.r = F2UINT(uint64_t, value.r);
		pixel.g = F2UINT(uint64_t, value.g);
	} return;
	case Format::RG64Sint: {
		auto& pixel = fetch_pixel<long2>(ptr, width, x, y);
		pixel.r = F2SINT(int64_t, value.r);
		pixel.g = F2SINT(int64_t, value.g);
	} return;
	case Format::RG64Float: {
		auto& pixel = fetch_pixel<double2>(ptr, width, x, y);
		pixel.r = F2DOUBLE(value.r);
		pixel.g = F2DOUBLE(value.g);
	} return;
	case Format::RGB8Unorm: {
		auto& pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		pixel.r = F2UNORM(uint8_t, value.r);
		pixel.g = F2UNORM(uint8_t, value.g);
		pixel.b = F2UNORM(uint8_t, value.b);
	} return;
	case Format::RGB8Snorm: {
		auto& pixel = fetch_pixel<char3>(ptr, width, x, y);
		pixel.r = F2SNORM(int8_t, value.r);
		pixel.g = F2SNORM(int8_t, value.g);
		pixel.b = F2SNORM(int8_t, value.b);
	} return;
	case Format::RGB8Uscaled: {
		auto& pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		pixel.r = F2USCALED(uint8_t, value.r);
		pixel.g = F2USCALED(uint8_t, value.g);
		pixel.b = F2USCALED(uint8_t, value.b);
	} return;
	case Format::RGB8Sscaled: {
		auto& pixel = fetch_pixel<char3>(ptr, width, x, y);
		pixel.r = F2SSCALED(int8_t, value.r);
		pixel.g = F2SSCALED(int8_t, value.g);
		pixel.b = F2SSCALED(int8_t, value.b);
	} return;
	case Format::RGB8Uint: {
		auto& pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		pixel.r = F2UINT(uint8_t, value.r);
		pixel.g = F2UINT(uint8_t, value.g);
		pixel.b = F2UINT(uint8_t, value.b);
	} return;
	case Format::RGB8Sint: {
		auto& pixel = fetch_pixel<char3>(ptr, width, x, y);
		pixel.r = F2SINT(int8_t, value.r);
		pixel.g = F2SINT(int8_t, value.g);
		pixel.b = F2SINT(int8_t, value.b);
	} return;
	//case Format::RGB8Srgb: {
	//
	//} return;
	case Format::BGR8Unorm: {
		auto& pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		pixel.r = F2UNORM(uint8_t, value.b);
		pixel.g = F2UNORM(uint8_t, value.g);
		pixel.b = F2UNORM(uint8_t, value.r);
	} return;
	case Format::BGR8Snorm: {
		auto& pixel = fetch_pixel<char3>(ptr, width, x, y);
		pixel.r = F2SNORM(int8_t, value.b);
		pixel.g = F2SNORM(int8_t, value.g);
		pixel.b = F2SNORM(int8_t, value.r);
	} return;
	case Format::BGR8Uscaled: {
		auto& pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		pixel.r = F2USCALED(uint8_t, value.b);
		pixel.g = F2USCALED(uint8_t, value.g);
		pixel.b = F2USCALED(uint8_t, value.r);
	} return;
	case Format::BGR8Sscaled: {
		auto& pixel = fetch_pixel<char3>(ptr, width, x, y);
		pixel.r = F2SSCALED(int8_t, value.b);
		pixel.g = F2SSCALED(int8_t, value.g);
		pixel.b = F2SSCALED(int8_t, value.r);
	} return;
	case Format::BGR8Uint: {
		auto& pixel = fetch_pixel<uchar3>(ptr, width, x, y);
		pixel.r = F2UINT(uint8_t, value.b);
		pixel.g = F2UINT(uint8_t, value.g);
		pixel.b = F2UINT(uint8_t, value.r);
	} return;
	case Format::BGR8Sint: {
		auto& pixel = fetch_pixel<char3>(ptr, width, x, y);
		pixel.r = F2SINT(int8_t, value.b);
		pixel.g = F2SINT(int8_t, value.g);
		pixel.b = F2SINT(int8_t, value.r);
	} return;
	//case Format::BGR8Srgb: {
	//
	//} return;
	case Format::RGB16Unorm: {
		auto& pixel = fetch_pixel<ushort3>(ptr, width, x, y);
		pixel.r = F2UNORM(uint16_t, value.r);
		pixel.g = F2UNORM(uint16_t, value.g);
		pixel.b = F2UNORM(uint16_t, value.b);
	} return;
	case Format::RGB16Snorm: {
		auto& pixel = fetch_pixel<short3>(ptr, width, x, y);
		pixel.r = F2SNORM(int16_t, value.r);
		pixel.g = F2SNORM(int16_t, value.g);
		pixel.b = F2SNORM(int16_t, value.b);
	} return;
	case Format::RGB16Uscaled: {
		auto& pixel = fetch_pixel<ushort3>(ptr, width, x, y);
		pixel.r = F2USCALED(uint16_t, value.r);
		pixel.g = F2USCALED(uint16_t, value.g);
		pixel.b = F2USCALED(uint16_t, value.b);
	} return;
	case Format::RGB16Sscaled: {
		auto& pixel = fetch_pixel<short3>(ptr, width, x, y);
		pixel.r = F2SSCALED(int16_t, value.r);
		pixel.g = F2SSCALED(int16_t, value.g);
		pixel.b = F2SSCALED(int16_t, value.b);
	} return;
	case Format::RGB16Uint: {
		auto& pixel = fetch_pixel<ushort3>(ptr, width, x, y);
		pixel.r = F2UINT(uint16_t, value.r);
		pixel.g = F2UINT(uint16_t, value.g);
		pixel.b = F2UINT(uint16_t, value.b);
	} return;
	case Format::RGB16Sint: {
		auto& pixel = fetch_pixel<short3>(ptr, width, x, y);
		pixel.r = F2SINT(int16_t, value.r);
		pixel.g = F2SINT(int16_t, value.g);
		pixel.b = F2SINT(int16_t, value.b);
	} return;
	//case Format::RGB16Float: {
	//
	//} return;
	case Format::RGB32Uint: {
		auto& pixel = fetch_pixel<uint3>(ptr, width, x, y);
		pixel.r = F2UINT(uint32_t, value.r);
		pixel.g = F2UINT(uint32_t, value.g);
		pixel.b = F2UINT(uint32_t, value.b);
	} return;
	case Format::RGB32Sint: {
		auto& pixel = fetch_pixel<int3>(ptr, width, x, y);
		pixel.r = F2UINT(int32_t, value.r);
		pixel.g = F2UINT(int32_t, value.g);
		pixel.b = F2UINT(int32_t, value.b);
	} return;
	case Format::RGB32Float: {
		auto& pixel = fetch_pixel<float3>(ptr, width, x, y);
		pixel.r = value.r;
		pixel.g = value.g;
		pixel.b = value.b;
	} return;
	case Format::RGB64Uint: {
		auto& pixel = fetch_pixel<ulong3>(ptr, width, x, y);
		pixel.r = F2UINT(uint64_t, value.r);
		pixel.g = F2UINT(uint64_t, value.g);
		pixel.b = F2UINT(uint64_t, value.b);
	} return;
	case Format::RGB64Sint: {
		auto& pixel = fetch_pixel<long3>(ptr, width, x, y);
		pixel.r = F2SINT(int64_t, value.r);
		pixel.g = F2SINT(int64_t, value.g);
		pixel.b = F2SINT(int64_t, value.b);
	} return;
	case Format::RGB64Float: {
		auto& pixel = fetch_pixel<double3>(ptr, width, x, y);
		pixel.r = F2DOUBLE(value.r);
		pixel.g = F2DOUBLE(value.g);
		pixel.b = F2DOUBLE(value.b);
	} return;
	case Format::RGBA8Unorm: {
		auto& pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		pixel.r = F2UNORM(uint8_t, value.r);
		pixel.g = F2UNORM(uint8_t, value.g);
		pixel.b = F2UNORM(uint8_t, value.b);
		pixel.a = F2UNORM(uint8_t, value.a);
	} return;
	case Format::RGBA8Snorm: {
		auto& pixel = fetch_pixel<char4>(ptr, width, x, y);
		pixel.r = F2SNORM(int8_t, value.r);
		pixel.g = F2SNORM(int8_t, value.g);
		pixel.b = F2SNORM(int8_t, value.b);
		pixel.a = F2SNORM(int8_t, value.a);
	} return;
	case Format::RGBA8Uscaled: {
		auto& pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		pixel.r = F2USCALED(uint8_t, value.r);
		pixel.g = F2USCALED(uint8_t, value.g);
		pixel.b = F2USCALED(uint8_t, value.b);
		pixel.a = F2USCALED(uint8_t, value.a);
	} return;
	case Format::RGBA8Sscaled: {
		auto& pixel = fetch_pixel<char4>(ptr, width, x, y);
		pixel.r = F2SSCALED(int8_t, value.r);
		pixel.g = F2SSCALED(int8_t, value.g);
		pixel.b = F2SSCALED(int8_t, value.b);
		pixel.a = F2SSCALED(int8_t, value.a);
	} return;
	case Format::RGBA8Uint: {
		auto& pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		pixel.r = F2UINT(uint8_t, value.r);
		pixel.g = F2UINT(uint8_t, value.g);
		pixel.b = F2UINT(uint8_t, value.b);
		pixel.a = F2UINT(uint8_t, value.a);
	} return;
	case Format::RGBA8Sint: {
		auto& pixel = fetch_pixel<char4>(ptr, width, x, y);
		pixel.r = F2SINT(int8_t, value.r);
		pixel.g = F2SINT(int8_t, value.g);
		pixel.b = F2SINT(int8_t, value.b);
		pixel.a = F2SINT(int8_t, value.a);
	} return;
	//case Format::RGBA8Srgb: {
	//
	//} return;
	case Format::BGRA8Unorm: {
		auto& pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		pixel.r = F2UNORM(uint8_t, value.b);
		pixel.g = F2UNORM(uint8_t, value.g);
		pixel.b = F2UNORM(uint8_t, value.r);
		pixel.a = F2UNORM(uint8_t, value.a);
	} return;
	case Format::BGRA8Snorm: {
		auto& pixel = fetch_pixel<char4>(ptr, width, x, y);
		pixel.r = F2SNORM(int8_t, value.b);
		pixel.g = F2SNORM(int8_t, value.g);
		pixel.b = F2SNORM(int8_t, value.r);
		pixel.a = F2SNORM(int8_t, value.a);
	} return;
	case Format::BGRA8Uscaled: {
		auto& pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		pixel.r = F2USCALED(uint8_t, value.b);
		pixel.g = F2USCALED(uint8_t, value.g);
		pixel.b = F2USCALED(uint8_t, value.r);
		pixel.a = F2USCALED(uint8_t, value.a);
	} return;
	case Format::BGRA8Sscaled: {
		auto& pixel = fetch_pixel<char4>(ptr, width, x, y);
		pixel.r = F2SSCALED(int8_t, value.b);
		pixel.g = F2SSCALED(int8_t, value.g);
		pixel.b = F2SSCALED(int8_t, value.r);
		pixel.a = F2SSCALED(int8_t, value.a);
	} return;
	case Format::BGRA8Uint: {
		auto& pixel = fetch_pixel<uchar4>(ptr, width, x, y);
		pixel.r = F2UINT(uint8_t, value.b);
		pixel.g = F2UINT(uint8_t, value.g);
		pixel.b = F2UINT(uint8_t, value.r);
		pixel.a = F2UINT(uint8_t, value.a);
	} return;
	case Format::BGRA8Sint: {
		auto& pixel = fetch_pixel<char4>(ptr, width, x, y);
		pixel.r = F2SINT(int8_t, value.b);
		pixel.g = F2SINT(int8_t, value.g);
		pixel.b = F2SINT(int8_t, value.r);
		pixel.a = F2SINT(int8_t, value.a);
	} return;
	//case Format::BGRA8Srgb: {
	//
	//} return;
	case Format::RGBA16Unorm: {
		auto& pixel = fetch_pixel<ushort4>(ptr, width, x, y);
		pixel.r = F2UNORM(uint16_t, value.r);
		pixel.g = F2UNORM(uint16_t, value.g);
		pixel.b = F2UNORM(uint16_t, value.b);
		pixel.a = F2UNORM(uint16_t, value.a);
	} return;
	case Format::RGBA16Snorm: {
		auto& pixel = fetch_pixel<short4>(ptr, width, x, y);
		pixel.r = F2SNORM(int16_t, value.r);
		pixel.g = F2SNORM(int16_t, value.g);
		pixel.b = F2SNORM(int16_t, value.b);
		pixel.a = F2SNORM(int16_t, value.a);
	} return;
	case Format::RGBA16Uscaled: {
		auto& pixel = fetch_pixel<ushort4>(ptr, width, x, y);
		pixel.r = F2USCALED(uint16_t, value.r);
		pixel.g = F2USCALED(uint16_t, value.g);
		pixel.b = F2USCALED(uint16_t, value.b);
		pixel.a = F2USCALED(uint16_t, value.a);
	} return;
	case Format::RGBA16Sscaled: {
		auto& pixel = fetch_pixel<short4>(ptr, width, x, y);
		pixel.r = F2SSCALED(int16_t, value.r);
		pixel.g = F2SSCALED(int16_t, value.g);
		pixel.b = F2SSCALED(int16_t, value.b);
		pixel.a = F2SSCALED(int16_t, value.a);
	} return;
	case Format::RGBA16Uint: {
		auto& pixel = fetch_pixel<ushort4>(ptr, width, x, y);
		pixel.r = F2UINT(uint16_t, value.r);
		pixel.g = F2UINT(uint16_t, value.g);
		pixel.b = F2UINT(uint16_t, value.b);
		pixel.a = F2UINT(uint16_t, value.a);
	} return;
	case Format::RGBA16Sint: {
		auto& pixel = fetch_pixel<short4>(ptr, width, x, y);
		pixel.r = F2SINT(int16_t, value.r);
		pixel.g = F2SINT(int16_t, value.g);
		pixel.b = F2SINT(int16_t, value.b);
		pixel.a = F2SINT(int16_t, value.a);
	} return;
	//case Format::RGBA16Float: {
	//
	//} return;
	case Format::RGBA32Uint: {
		auto& pixel = fetch_pixel<uint4>(ptr, width, x, y);
		pixel.r = F2UINT(uint32_t, value.r);
		pixel.g = F2UINT(uint32_t, value.g);
		pixel.b = F2UINT(uint32_t, value.b);
		pixel.a = F2UINT(uint32_t, value.a);
	} return;
	case Format::RGBA32Sint: {
		auto& pixel = fetch_pixel<int4>(ptr, width, x, y);
		pixel.r = F2SINT(int32_t, value.r);
		pixel.g = F2SINT(int32_t, value.g);
		pixel.b = F2SINT(int32_t, value.b);
		pixel.a = F2SINT(int32_t, value.a);
	} return;
	case Format::RGBA32Float: {
		auto& pixel = fetch_pixel<float4>(ptr, width, x, y);
		pixel.r = value.r;
		pixel.g = value.g;
		pixel.b = value.b;
		pixel.a = value.a;
	} return;
	case Format::RGBA64Uint: {
		auto& pixel = fetch_pixel<ulong4>(ptr, width, x, y);
		pixel.r = F2UINT(uint64_t, value.r);
		pixel.g = F2UINT(uint64_t, value.g);
		pixel.b = F2UINT(uint64_t, value.b);
		pixel.a = F2UINT(uint64_t, value.a);
	} return;
	case Format::RGBA64Sint: {
		auto& pixel = fetch_pixel<long4>(ptr, width, x, y);
		pixel.r = F2SINT(int64_t, value.r);
		pixel.g = F2SINT(int64_t, value.g);
		pixel.b = F2SINT(int64_t, value.b);
		pixel.a = F2SINT(int64_t, value.a);
	} return;
	case Format::RGBA64Float: {
		auto& pixel = fetch_pixel<double4>(ptr, width, x, y);
		pixel.r = F2DOUBLE(value.r);
		pixel.g = F2DOUBLE(value.g);
		pixel.b = F2DOUBLE(value.b);
		pixel.a = F2DOUBLE(value.a);
	} return;
	case Format::RG4UnormPack8: { /////////////////////////////////////////////////
		fetch_pixel<uint8_t>(ptr, width, x, y) =
			(static_cast<uint8_t>(value.r * 15.99f) & 0x0f) << 4 |
			(static_cast<uint8_t>(value.g * 15.99f) & 0x0f) << 0;
	} return;
	case Format::RGBA4UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 15.99f) & 0x0f) << 12 |
			(static_cast<uint16_t>(value.g * 15.99f) & 0x0f) << 8 |
			(static_cast<uint16_t>(value.b * 15.99f) & 0x0f) << 4 |
			(static_cast<uint16_t>(value.a * 15.99f) & 0x0f) << 0;
	} return;
	case Format::BGRA4UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 15.99f) & 0x0f) << 4 |
			(static_cast<uint16_t>(value.g * 15.99f) & 0x0f) << 8 |
			(static_cast<uint16_t>(value.b * 15.99f) & 0x0f) << 12 |
			(static_cast<uint16_t>(value.a * 15.99f) & 0x0f) << 0;
	} return;
	case Format::R5G6B5UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 31.99f) & 0x1f) << 11 |
			(static_cast<uint16_t>(value.g * 63.99f) & 0x3f) << 5 |
			(static_cast<uint16_t>(value.b * 31.99f) & 0x1f) << 0;
	} return;
	case Format::B5G6R5UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 31.99f) & 0x1f) << 0 |
			(static_cast<uint16_t>(value.g * 63.99f) & 0x3f) << 5 |
			(static_cast<uint16_t>(value.b * 31.99f) & 0x1f) << 11;
	} return;
	case Format::R5G5B5A1UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 31.99f) & 0x1f) << 11 |
			(static_cast<uint16_t>(value.g * 31.99f) & 0x1f) << 6 |
			(static_cast<uint16_t>(value.b * 31.99f) & 0x1f) << 1 |
			(static_cast<uint16_t>(value.a) & 0x01) << 0;
	} return;
	case Format::B5G5R5A1UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 31.99f) & 0x1f) << 1 |
			(static_cast<uint16_t>(value.g * 31.99f) & 0x1f) << 6 |
			(static_cast<uint16_t>(value.b * 31.99f) & 0x1f) << 11 |
			(static_cast<uint16_t>(value.a) & 0x01) << 0;
	} return;
	case Format::A1R5G5B5UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 31.99f) & 0x1f) << 10 |
			(static_cast<uint16_t>(value.g * 31.99f) & 0x1f) << 5 |
			(static_cast<uint16_t>(value.b * 31.99f) & 0x1f) << 0 |
			(static_cast<uint16_t>(value.a) & 0x01) << 15;
	} return;
	case Format::A4R4G4B4UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 15.99f) & 0x0f) << 8 |
			(static_cast<uint16_t>(value.g * 15.99f) & 0x0f) << 4 |
			(static_cast<uint16_t>(value.b * 15.99f) & 0x0f) << 0 |
			(static_cast<uint16_t>(value.a * 15.99f) & 0x0f) << 12;
	} return;
	case Format::A4B4G4R4UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 15.99f) & 0x0f) << 0 |
			(static_cast<uint16_t>(value.g * 15.99f) & 0x0f) << 4 |
			(static_cast<uint16_t>(value.b * 15.99f) & 0x0f) << 8 |
			(static_cast<uint16_t>(value.a * 15.99f) & 0x0f) << 12;
	} return;
	case Format::A1B5G5R5UnormPack16: {
		fetch_pixel<uint16_t>(ptr, width, x, y) =
			(static_cast<uint16_t>(value.r * 31.99f) & 0x1f) << 0 |
			(static_cast<uint16_t>(value.g * 31.99f) & 0x1f) << 5 |
			(static_cast<uint16_t>(value.b * 31.99f) & 0x1f) << 10 |
			(static_cast<uint16_t>(value.a) & 0x01)<< 15;
	} return;
	case Format::ABGR8UnormPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r * 255.99f) & 0xff) << 0 |
			(static_cast<uint32_t>(value.g * 255.99f) & 0xff) << 8 |
			(static_cast<uint32_t>(value.b * 255.99f) & 0xff) << 16 |
			(static_cast<uint32_t>(value.a * 255.99f) & 0xff) << 24;
	} return;
	case Format::ABGR8SnormPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r * (0 < value.r ? 127.99f : -128.99f)) & 0xff) << 0 |
			(static_cast<int32_t>(value.g * (0 < value.g ? 127.99f : -128.99f)) & 0xff) << 8 |
			(static_cast<int32_t>(value.b * (0 < value.b ? 127.99f : -128.99f)) & 0xff) << 16 |
			(static_cast<int32_t>(value.a * (0 < value.a ? 127.99f : -128.99f)) & 0xff) << 24;
	} return;
	case Format::ABGR8UscaledPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r) & 0xff) << 0 |
			(static_cast<uint32_t>(value.g) & 0xff) << 8 |
			(static_cast<uint32_t>(value.b) & 0xff) << 16 |
			(static_cast<uint32_t>(value.a) & 0xff) << 24;
	} return;
	case Format::ABGR8SscaledPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r) & 0xff) << 0 |
			(static_cast<int32_t>(value.g) & 0xff) << 8 |
			(static_cast<int32_t>(value.b) & 0xff) << 16 |
			(static_cast<int32_t>(value.a) & 0xff) << 24;
	} return;
	case Format::ABGR8UintPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r) & 0xff) << 0 |
			(static_cast<uint32_t>(value.g) & 0xff) << 8 |
			(static_cast<uint32_t>(value.b) & 0xff) << 16 |
			(static_cast<uint32_t>(value.a) & 0xff) << 24;
	} return;
	case Format::ABGR8SintPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r) & 0xff) << 0 |
			(static_cast<int32_t>(value.g) & 0xff) << 8 |
			(static_cast<int32_t>(value.b) & 0xff) << 16 |
			(static_cast<int32_t>(value.a) & 0xff) << 24;
	} return;
	//case Format::ABGR8SrgbPack32: {
	//
	//}
	case Format::A2RGB10UnormPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r * 1023.99f) & 0x3ff) << 20 |
			(static_cast<uint32_t>(value.g * 1023.99f) & 0x3ff) << 10 |
			(static_cast<uint32_t>(value.b * 1023.99f) & 0x3ff) << 0 |
			(static_cast<uint32_t>(value.a * 3.99f) & 0x003) << 30;
	} return;
	case Format::A2RGB10SnormPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r * (0 < value.r ? 512.99f : -511.99f)) & 0x3ff) << 20 |
			(static_cast<int32_t>(value.g * (0 < value.g ? 512.99f : -511.99f)) & 0x3ff) << 10 |
			(static_cast<int32_t>(value.b * (0 < value.b ? 512.99f : -511.99f)) & 0x3ff) << 0 |
			(static_cast<int32_t>(value.a * (0 < value.a ? -2.99f : -1.99f)) & 0x003) << 30;
	} return;
	case Format::A2RGB10UscaledPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r) & 0x3ff) << 20 |
			(static_cast<uint32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<uint32_t>(value.b) & 0x3ff) << 0 |
			(static_cast<uint32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A2RGB10SscaledPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r) & 0x3ff) << 20 |
			(static_cast<int32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<int32_t>(value.b) & 0x3ff) << 0 |
			(static_cast<int32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A2RGB10UintPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r) & 0x3ff) << 20 |
			(static_cast<uint32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<uint32_t>(value.b) & 0x3ff) << 0 |
			(static_cast<uint32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A2RGB10SintPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r) & 0x3ff) << 20 |
			(static_cast<int32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<int32_t>(value.b) & 0x3ff) << 0 |
			(static_cast<int32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A2BGR10UnormPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r * 1023.99f) & 0x3ff) << 0 |
			(static_cast<uint32_t>(value.g * 1023.99f) & 0x3ff) << 10 |
			(static_cast<uint32_t>(value.b * 1023.99f) & 0x3ff) << 20 |
			(static_cast<uint32_t>(value.a * 3.99f) & 0x003) << 30;
	} return;
	case Format::A2BGR10SnormPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r * (0 < value.r ? 512.99f : -511.99f)) & 0x3ff) << 0 |
			(static_cast<int32_t>(value.g * (0 < value.g ? 512.99f : -511.99f)) & 0x3ff) << 10 |
			(static_cast<int32_t>(value.b * (0 < value.b ? 512.99f : -511.99f)) & 0x3ff) << 20 |
			(static_cast<int32_t>(value.a * (0 < value.a ? -2.99f : -1.99f)) & 0x003) << 30;
	} return;
	case Format::A2BGR10UscaledPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r) & 0x3ff) << 0 |
			(static_cast<uint32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<uint32_t>(value.b) & 0x3ff) << 20 |
			(static_cast<uint32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A2BGR10SscaledPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r) & 0x3ff) << 0 |
			(static_cast<int32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<int32_t>(value.b) & 0x3ff) << 20 |
			(static_cast<int32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A2BGR10UintPack32: {
		fetch_pixel<uint32_t>(ptr, width, x, y) =
			(static_cast<uint32_t>(value.r) & 0x3ff) << 0 |
			(static_cast<uint32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<uint32_t>(value.b) & 0x3ff) << 20 |
			(static_cast<uint32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A2BGR10SintPack32: {
		fetch_pixel<int32_t>(ptr, width, x, y) =
			(static_cast<int32_t>(value.r) & 0x3ff) << 0 |
			(static_cast<int32_t>(value.g) & 0x3ff) << 10 |
			(static_cast<int32_t>(value.b) & 0x3ff) << 20 |
			(static_cast<int32_t>(value.a) & 0x003) << 30;
	} return;
	case Format::A8Unorm: {
		fetch_pixel<uint8_t>(ptr, width, x, y) = F2UNORM(uint8_t, value.a);
	} return;
	}

	VERA_ASSERT_MSG(false, "unsupported format");
}

VERA_NAMESPACE_END
