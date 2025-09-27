#include "../../include/vera/graphics/image_sampler.h"

#include "pixel.h"
#include "../../include/vera/graphics/image.h"
#include "../../include/vera/core/exception.h"
#include <algorithm>

#define NOALPHA 1.f

VERA_NAMESPACE_BEGIN

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
		auto pixel = fetch_pixel<uint16_t>(ptr, width, x, y);
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

	throw Exception("unsupported format");
}

template <ImageSamplerAddressMode Mode>
static float4 address_mode(float x, float size)
{
	switch constexpr (Mode) {
	case ImageSamplerAddressMode::Repeat: {
		float mod = fmodf(x, size);
		return 0.f < x ? mod : size - mod;
	}
	case ImageSamplerAddressMode::MirroredRepeat: {
		float width2 = 2.f * size;
		float mod    = fmodf(x, width2);
		return 0.f < x ? mod : width2 - mod;
	}
	case ImageSamplerAddressMode::ClampToEdge: {
		return std::clamp(x, 0.f, size);
	}
	case ImageSamplerAddressMode::ClampToBorder: {
		if (x < 0.f || size < x) return NAN;
		return x;
	}
	case ImageSamplerAddressMode::MirrorClampToEdge: {
		// TODO: implement
		return std::clamp(x, 0.f, size);
	}
	}
}

static float4 nearest_filtering(const Image& image, float u, float v)
{
	auto x = static_cast<uint32_t>(roundf(u));
	auto y = static_cast<uint32_t>(roundf(v));

	return fetch_components(image.data(), image.width(), x, y, image.format());
}

static float4 linear_filtering(const Image& image, float u, float v)
{
	// TODO: implement
	return {};
}

static float4 area_filtering(const Image& image, float u, float v)
{
	// TODO: implement
	return {};
}

static float repeatf(float x, float size)
{
	return modf(x + 0.5f, size) - 0.5f;
}

static float mirrored_repeatf(float x, float size)
{
	return -fabsf(repeatf(x, 2.f * size) - size + 0.5f) + size - 0.5f;
}

static float clamp_edge(float x, float size)
{
	if (x < 0.5f) return -0.5f;
	if (size - 0.5f < x) return size - 0.5f - FLT_EPSILON;
	return x;
}

static float clamp_border(float x, float size)
{
	if (x < -0.5f || size - 0.5f < x) return NAN;
	return x;
}

template <ImageSamplerAddressMode ModeU, ImageSamplerAddressMode ModeV, bool Unnormalized>
static float2 sample_linear(float x, float size)
{
	float tmp;

	switch (ModeU) {
	case ImageSamplerAddressMode::Repeat: {
		tmp = floorf(repeatf(x, size));
		return { repeatf(tmp), repeatf(tmp + 1.f) };
	} break;
	case ImageSamplerAddressMode::MirroredRepeat: {
		tmp = floorf(mirrored_repeatf(x, size));
		return { mirrored_repeatf(tmp), mirrored_repeatf(tmp + 1.f) };
	} break;
	case ImageSamplerAddressMode::ClampToEdge:
		tmp = floorf(clamp_edge(x, size));
		return { clamp_edge(tmp), clamp_edge(tmp + 1.f) };
	case ImageSamplerAddressMode::ClampToBorder:
		
	case ImageSamplerAddressMode::MirrorClampToEdge:
		// TODO: implement ImageSamplerAddressMode::MirrorClampToEdge
		break;
	}

	switch (ModeV) {
	case ImageSamplerAddressMode::Repeat:

		break;
	case ImageSamplerAddressMode::MirroredRepeat:

		break;
	case ImageSamplerAddressMode::ClampToEdge:

		break;
	case ImageSamplerAddressMode::ClampToBorder:

		break;
	case ImageSamplerAddressMode::MirrorClampToEdge:
		// TODO: implement ImageSamplerAddressMode::MirrorClampToEdge
		break;
	}
}

ImageSampler::ImageSampler() :
	ImageSampler(ImageSamplerCreateInfo{}) { }

ImageSampler::ImageSampler(const ImageSamplerCreateInfo& info)
{

}

float4 ImageSampler::sample(const Image& image, const float2& uv) const
{
	return sample(image, uv.x, uv.y);
}

float4 ImageSampler::sample(const Image& image, float u, float v) const
{
	// TODO: optimize more

	u = m_adress_u_fptr(u, image.width());
	v = m_adress_u_fptr(v, image.height());

	if (isnan(u) || isnan(v))
		return m_border_color;

	switch (ImageSamplerFilter{}) {
	case ImageSamplerFilter::Nearest: {
	} break;
	case ImageSamplerFilter::Linear: {
	} break;
	case ImageSamplerFilter::Area: {
	} break;
	}

	return float4();
}

VERA_NAMESPACE_END