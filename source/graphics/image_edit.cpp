#include "../../include/vera/graphics/image_edit.h"

#include "pixel.h"
#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/assertion.h"
#include "../../include/vera/graphics/format_traits.h"
#include "../../include/vera/graphics/image_sampler.h"
#include "../../include/vera/math/math_util.h"
#include "../../include/vera/math/vector_math.h"
#include <algorithm>

#define ASSIGN1BYTES(lhs, rhs)  *reinterpret_cast<uint8_t*>(dst) = *reinterpret_cast<const uint8_t*>(src);
#define ASSIGN2BYTES(lhs, rhs)  *reinterpret_cast<uint16_t*>(dst) = *reinterpret_cast<const uint16_t*>(src);
#define ASSIGN3BYTES(lhs, rhs)  *reinterpret_cast<uchar3*>(dst) = *reinterpret_cast<const uchar3*>(src);
#define ASSIGN4BYTES(lhs, rhs)  *reinterpret_cast<uint32_t*>(dst) = *reinterpret_cast<const uint32_t*>(src);
#define ASSIGN5BYTES(lhs, rhs)  *reinterpret_cast<storage5*>(dst) = *reinterpret_cast<const storage5*>(src);
#define ASSIGN6BYTES(lhs, rhs)  *reinterpret_cast<ushort3*>(dst) = *reinterpret_cast<const ushort3*>(src);
#define ASSIGN8BYTES(lhs, rhs)  *reinterpret_cast<uint64_t*>(dst) = *reinterpret_cast<const uint64_t*>(src);
#define ASSIGN12BYTES(lhs, rhs) *reinterpret_cast<uint3*>(dst) = *reinterpret_cast<const uint3*>(src);
#define ASSIGN16BYTES(lhs, rhs) *reinterpret_cast<ulong2*>(dst) = *reinterpret_cast<const ulong2*>(src);
#define ASSIGN24BYTES(lhs, rhs) *reinterpret_cast<ulong3*>(dst) = *reinterpret_cast<const ulong3*>(src);
#define ASSIGN32BYTES(lhs, rhs) *reinterpret_cast<ulong4*>(dst) = *reinterpret_cast<const ulong4*>(src);

#define SWAP1BYTES(lhs, rhs) std::swap(*reinterpret_cast<uint8_t*>(lhs), *reinterpret_cast<uint8_t*>(rhs));
#define SWAP2BYTES(lhs, rhs) std::swap(*reinterpret_cast<uint16_t*>(lhs), *reinterpret_cast<uint16_t*>(rhs));
#define SWAP3BYTES(lhs, rhs) std::swap(*reinterpret_cast<uchar3*>(lhs), *reinterpret_cast<uchar3*>(rhs));
#define SWAP4BYTES(lhs, rhs) std::swap(*reinterpret_cast<uint32_t*>(lhs), *reinterpret_cast<uint32_t*>(rhs));
#define SWAP5BYTES(lhs, rhs) std::swap(*reinterpret_cast<storage5*>(lhs), *reinterpret_cast<storage5*>(rhs));
#define SWAP6BYTES(lhs, rhs) std::swap(*reinterpret_cast<ushort3*>(lhs), *reinterpret_cast<ushort3*>(rhs));
#define SWAP8BYTES(lhs, rhs) std::swap(*reinterpret_cast<uint64_t*>(lhs), *reinterpret_cast<uint64_t*>(rhs));
#define SWAP12BYTES(lhs, rhs) std::swap(*reinterpret_cast<uint3*>(lhs), *reinterpret_cast<uint3*>(rhs));
#define SWAP16BYTES(lhs, rhs) std::swap(*reinterpret_cast<ulong2*>(lhs), *reinterpret_cast<ulong2*>(rhs));
#define SWAP24BYTES(lhs, rhs) std::swap(*reinterpret_cast<ulong3*>(lhs), *reinterpret_cast<ulong3*>(rhs));
#define SWAP32BYTES(lhs, rhs) std::swap(*reinterpret_cast<ulong4*>(lhs), *reinterpret_cast<ulong4*>(rhs));

VERA_NAMESPACE_BEGIN

struct storage5
{
	void operator=(const storage5& rhs)
	{
		*reinterpret_cast<uint32_t*>(data) = *reinterpret_cast<const uint32_t*>(rhs.data);
		data[4] = rhs.data[4];
	}

	uint8_t data[5];
};

static void memswap(void* ptr0, void* ptr1, size_t size)
{
	auto* pch0 = reinterpret_cast<uint8_t*>(ptr0);
	auto* pch1 = reinterpret_cast<uint8_t*>(ptr1);
	
	// go slow way
	if ((intptr_t)pch0 % sizeof(size_t) != (intptr_t)pch1 % sizeof(size_t)) {
		for (size_t i = 0; i < size; ++i)
			std::swap(pch0[i], pch1[i]);
		return;
	}

	for (;(intptr_t)pch0 % sizeof(size_t) != 0; --size)
		std::iter_swap(pch0++, pch1++);

	while (sizeof(size_t) <= size) {
		std::iter_swap(
			reinterpret_cast<size_t*>(pch0),
			reinterpret_cast<size_t*>(pch1));
		pch0 += 8;
		pch1 += 8;
		size -= 8;
	}

	for (;0 < size; --size)
		std::iter_swap(pch0++, pch1++);
}

Image ImageEdit::flip(const Image& image, bool horizontal, bool vertical)
{
////////// define some macros /////////////////////////////////////////////////////////////////////

#define FOR_EACH_PIXEL0(pixel_size, content) \
	for (uint32_t i = 0; i < n; ++i) { \
		auto*       dst = dst_ptr + size - pixel_size * (i + 1) ; \
		const auto* src = src_ptr + pixel_size * i; \
		content \
	}

#define FOR_EACH_PIXEL1(pixel_size, content) \
	for (uint32_t y = 0; y < height; ++y) { \
		for (uint32_t x = 0; x < width; ++x) { \
			auto*       dst = dst_ptr + pixel_size * (width * y + (width - x - 1)); \
			const auto* src = src_ptr + (pixel_size) * (width * y + x); \
			content \
		} \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	if (!horizontal && !vertical)
		return image;

	const auto format     = image.format();
	const auto width      = image.width();
	const auto height     = image.height();
	const auto pixel_size = get_format_size(format);
	const auto row_bytes  = width * pixel_size;

	Image result(width, height, format);

	const auto* src_ptr = reinterpret_cast<const uint8_t*>(image.data());
	auto*       dst_ptr = reinterpret_cast<uint8_t*>(result.data());

	if (horizontal && vertical) {
		uint32_t size = height * row_bytes;
		uint32_t n    = width * height;

		switch (pixel_size) {
		case 1:  FOR_EACH_PIXEL0(1,  ASSIGN1BYTES(dst, src))  break;
		case 2:  FOR_EACH_PIXEL0(2,  ASSIGN2BYTES(dst, src))  break;
		case 3:  FOR_EACH_PIXEL0(3,  ASSIGN3BYTES(dst, src))  break;
		case 4:  FOR_EACH_PIXEL0(4,  ASSIGN4BYTES(dst, src))  break;
		case 5:  FOR_EACH_PIXEL0(5,  ASSIGN5BYTES(dst, src))  break;
		case 6:  FOR_EACH_PIXEL0(6,  ASSIGN6BYTES(dst, src))  break;
		case 8:  FOR_EACH_PIXEL0(8,  ASSIGN8BYTES(dst, src))  break;
		case 12: FOR_EACH_PIXEL0(12, ASSIGN12BYTES(dst, src)) break;
		case 16: FOR_EACH_PIXEL0(16, ASSIGN16BYTES(dst, src)) break;
		case 24: FOR_EACH_PIXEL0(24, ASSIGN24BYTES(dst, src)) break;
		case 32: FOR_EACH_PIXEL0(32, ASSIGN32BYTES(dst, src)) break;
		default: VERA_ASSERT_MSG(false, "invalid pixel size");
		}

	} else if (!horizontal && vertical) {
		for (uint32_t y = 0; y < height; ++y) {
			memcpy(
				dst_ptr + row_bytes * (height - y - 1),
				src_ptr + row_bytes * y,
				row_bytes);
		}
	} else if (horizontal && !vertical) {
		switch (pixel_size) {
		case 1:  FOR_EACH_PIXEL1(1,  ASSIGN1BYTES(dst, src))  break;
		case 2:  FOR_EACH_PIXEL1(2,  ASSIGN2BYTES(dst, src))  break;
		case 3:  FOR_EACH_PIXEL1(3,  ASSIGN3BYTES(dst, src))  break;
		case 4:  FOR_EACH_PIXEL1(4,  ASSIGN4BYTES(dst, src))  break;
		case 5:  FOR_EACH_PIXEL1(5,  ASSIGN5BYTES(dst, src))  break;
		case 6:  FOR_EACH_PIXEL1(6,  ASSIGN6BYTES(dst, src))  break;
		case 8:  FOR_EACH_PIXEL1(8,  ASSIGN8BYTES(dst, src))  break;
		case 12: FOR_EACH_PIXEL1(12, ASSIGN12BYTES(dst, src)) break;
		case 16: FOR_EACH_PIXEL1(16, ASSIGN16BYTES(dst, src)) break;
		case 24: FOR_EACH_PIXEL1(24, ASSIGN24BYTES(dst, src)) break;
		case 32: FOR_EACH_PIXEL1(32, ASSIGN32BYTES(dst, src)) break;
		default: VERA_ASSERT_MSG(false, "invalid pixel size");
		}
	}

	return result;

#undef FOR_EACH_PIXEL0
#undef FOR_EACH_PIXEL1
}

Image ImageEdit::flip(const Image& image, ImageFlipFlags flags)
{
	return ImageEdit::flip(
		image,
		flags.has(ImageFlipFlagBits::Horizontal),
		flags.has(ImageFlipFlagBits::Vertical));
}

Image ImageEdit::rotate(const Image& image, Rotation rot)
{
	switch (rot) {
	case Rotation::_90Deg:  return ImageEdit::rotateCW(image);
	case Rotation::_180Deg: return ImageEdit::flip(image, true, true);
	case Rotation::_270Deg: return ImageEdit::rotateCCW(image);
	default:                return image;
	}
}

Image ImageEdit::rotateCW(const Image& image)
{
////////// define some macros /////////////////////////////////////////////////////////////////////

#define FOR_EACH_PIXEL(pixel_size, content) \
	for (uint32_t y = 0; y < height; ++y) { \
		for (uint32_t x = 0; x < width; ++x) { \
			auto*       dst = dst_ptr + pixel_size * (height * x + (width - y - 1)); \
			const auto* src = src_ptr + pixel_size * (width * y + x); \
			content \
		}\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	const auto format     = image.format();
	const auto width      = image.width();
	const auto height     = image.height();
	const auto pixel_size = get_format_size(format);

	Image result(height, width, format);

	const auto* src_ptr = reinterpret_cast<const uint8_t*>(image.data());
	auto*       dst_ptr = reinterpret_cast<uint8_t*>(result.data());

	switch (pixel_size) {
	case 1:  FOR_EACH_PIXEL(1,  ASSIGN1BYTES(dst, src))  break;
	case 2:  FOR_EACH_PIXEL(2,  ASSIGN2BYTES(dst, src))  break;
	case 3:  FOR_EACH_PIXEL(3,  ASSIGN3BYTES(dst, src))  break;
	case 4:  FOR_EACH_PIXEL(4,  ASSIGN4BYTES(dst, src))  break;
	case 5:  FOR_EACH_PIXEL(5,  ASSIGN5BYTES(dst, src))  break;
	case 6:  FOR_EACH_PIXEL(6,  ASSIGN6BYTES(dst, src))  break;
	case 8:  FOR_EACH_PIXEL(8,  ASSIGN8BYTES(dst, src))  break;
	case 12: FOR_EACH_PIXEL(12, ASSIGN12BYTES(dst, src)) break;
	case 16: FOR_EACH_PIXEL(16, ASSIGN16BYTES(dst, src)) break;
	case 24: FOR_EACH_PIXEL(24, ASSIGN24BYTES(dst, src)) break;
	case 32: FOR_EACH_PIXEL(32, ASSIGN32BYTES(dst, src)) break;
	default: VERA_ASSERT_MSG(false, "invalid pixel size");
	}

	return result;

#undef FOR_EACH_PIXEL
}

Image ImageEdit::rotateCCW(const Image& image)
{
////////// define some macros /////////////////////////////////////////////////////////////////////

#define FOR_EACH_PIXEL(pixel_size, content) \
	for (uint32_t y = 0; y < height; ++y) { \
		for (uint32_t x = 0; x < width; ++x) { \
			auto*       dst = dst_ptr + pixel_size * (height * (width - x - 1) + y); \
			const auto* src = src_ptr + pixel_size * (width * y + x); \
			content \
		}\
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	const auto format     = image.format();
	const auto width      = image.width();
	const auto height     = image.height();
	const auto pixel_size = get_format_size(format);

	Image result(height, width, format);

	const auto* src_ptr = reinterpret_cast<const uint8_t*>(image.data());
	auto*       dst_ptr = reinterpret_cast<uint8_t*>(result.data());

	switch (pixel_size) {
	case 1:  FOR_EACH_PIXEL(1,  ASSIGN1BYTES(dst, src))  break;
	case 2:  FOR_EACH_PIXEL(2,  ASSIGN2BYTES(dst, src))  break;
	case 3:  FOR_EACH_PIXEL(3,  ASSIGN3BYTES(dst, src))  break;
	case 4:  FOR_EACH_PIXEL(4,  ASSIGN4BYTES(dst, src))  break;
	case 5:  FOR_EACH_PIXEL(5,  ASSIGN5BYTES(dst, src))  break;
	case 6:  FOR_EACH_PIXEL(6,  ASSIGN6BYTES(dst, src))  break;
	case 8:  FOR_EACH_PIXEL(8,  ASSIGN8BYTES(dst, src))  break;
	case 12: FOR_EACH_PIXEL(12, ASSIGN12BYTES(dst, src)) break;
	case 16: FOR_EACH_PIXEL(16, ASSIGN16BYTES(dst, src)) break;
	case 24: FOR_EACH_PIXEL(24, ASSIGN24BYTES(dst, src)) break;
	case 32: FOR_EACH_PIXEL(32, ASSIGN32BYTES(dst, src)) break;
	default: VERA_ASSERT_MSG(false, "invalid pixel size");
	}

	return result;

#undef FOR_EACH_PIXEL
}

Image ImageEdit::blit(const Image& image, const ImageSampler& sampler, const ImageBlitInfo& info)
{
	uint32_t dst_width  = info.dstWidth;
	uint32_t dst_height = info.dstHeight;
	Format   format     = image.format();
	float2   uv0        = info.uv0;
	float2   uv1        = info.uv1;
	float2   uv2        = info.uv2;
	float2   uv3        = info.uv3;

	Image result(dst_width, dst_height, format);

	void* ptr = result.data();

	for (uint32_t x = 0; x < dst_width; ++x) {
		for (uint32_t y = 0; y < dst_height; ++y) {
			float  tx = static_cast<float>(x) / dst_width;
			float  ty = static_cast<float>(y) / dst_height;
			float2 p0 = lerp(uv0, uv1, tx);
			float2 p1 = lerp(uv3, uv2, tx);
			float2 p  = lerp(p0, p1, ty);

			float4 color = sampler.sample(image, p.x, p.y);

			store_components(ptr, dst_width, x, y, format, color);
		}
	}

	return result;
}

Image ImageEdit::createMask(const Image& image, uint32_t at_x, uint32_t at_y, float alpha, float similarity)
{
	const auto  format      = image.format();
	const auto* ptr         = image.data();
	const auto  width       = image.width();
	const auto  height      = image.height();
	const auto  pixel_count = width * height;

	VERA_ASSERT(at_x < width && at_y < height);
	
	Image result(width, height, format);

	auto* result_ptr = result.data();

	if (!format_has_alpha(format) || format == Format::A8Unorm) return image;

	float4 target_color = fetch_components(ptr, width, at_x, at_y, format);

	for (uint32_t x = 0; x < width; ++x) {
		for (uint32_t y = 0; y < height; ++y) {
			float4 color = fetch_components(ptr, width, x, y, format);

			if (similarity < (dot(target_color, color) / length(target_color) / length(color)))
				color.w = alpha;
			
			store_components(result_ptr, width, x, y, format, color);
		}
	}

	return result;
}

void ImageEdit::flip(Image& result, const Image& image, bool horizontal, bool vertical)
{
////////// define some macros /////////////////////////////////////////////////////////////////////

#define FOR_EACH_PIXEL0(pixel_size, content) \
	for (uint32_t i = 0; i < n; ++i) { \
		uint8_t* ptr0 = ptr + pixel_size * i; \
		uint8_t* ptr1 = ptr + size - pixel_size * (i + 1); \
		content \
	}

#define FOR_EACH_PIXEL1(pixel_size, content) \
	for (uint32_t y = 0; y < height; ++y) { \
		for (uint32_t x = 0; x < n; ++x) { \
			uint8_t* ptr0 = ptr + pixel_size * (width * y + x); \
			uint8_t* ptr1 = ptr + pixel_size * (width * y + width - x - 1); \
			content \
		} \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////

	if (!horizontal && !vertical) {
		result = image;
		return;
	}

	if (std::addressof(result) != std::addressof(image)) {
		result = ImageEdit::flip(image, horizontal, vertical);
		return;
	}

	const Format   format     = image.format();
	const uint32_t width      = image.width();
	const uint32_t height     = image.height();
	const uint32_t pixel_size = get_format_size(format);
	const uint32_t row_bytes  = width * pixel_size;
	uint8_t*       ptr        = reinterpret_cast<uint8_t*>(result.data());

	if (horizontal && vertical) {
		uint32_t size = height * row_bytes;
		uint32_t n    = width * height / 2;

		switch (pixel_size) {
		case 1:  FOR_EACH_PIXEL0(1,  SWAP1BYTES(ptr0, ptr1))  break;
		case 2:  FOR_EACH_PIXEL0(2,  SWAP2BYTES(ptr0, ptr1))  break;
		case 3:  FOR_EACH_PIXEL0(3,  SWAP3BYTES(ptr0, ptr1))  break;
		case 4:  FOR_EACH_PIXEL0(4,  SWAP4BYTES(ptr0, ptr1))  break;
		case 5:  FOR_EACH_PIXEL0(5,  SWAP5BYTES(ptr0, ptr1))  break;
		case 6:  FOR_EACH_PIXEL0(6,  SWAP6BYTES(ptr0, ptr1))  break;
		case 8:  FOR_EACH_PIXEL0(8,  SWAP8BYTES(ptr0, ptr1))  break;
		case 12: FOR_EACH_PIXEL0(12, SWAP12BYTES(ptr0, ptr1)) break;
		case 16: FOR_EACH_PIXEL0(16, SWAP16BYTES(ptr0, ptr1)) break;
		case 24: FOR_EACH_PIXEL0(24, SWAP24BYTES(ptr0, ptr1)) break;
		case 32: FOR_EACH_PIXEL0(32, SWAP32BYTES(ptr0, ptr1)) break;
		default: VERA_ASSERT_MSG(false, "invalid pixel size");
		}
	} else if (!horizontal && vertical) {
		uint32_t mid_height = height / 2;

		for (uint32_t y = 0; y < mid_height; ++y)
			memswap(ptr + row_bytes * y, ptr + row_bytes * (height - y - 1), row_bytes);
	} else if (horizontal && !vertical) {
		uint32_t n = width / 2;

		switch (pixel_size) {
		case 1:  FOR_EACH_PIXEL1(1,  SWAP1BYTES(ptr0, ptr1))  break;
		case 2:  FOR_EACH_PIXEL1(2,  SWAP2BYTES(ptr0, ptr1))  break;
		case 3:  FOR_EACH_PIXEL1(3,  SWAP3BYTES(ptr0, ptr1))  break;
		case 4:  FOR_EACH_PIXEL1(4,  SWAP4BYTES(ptr0, ptr1))  break;
		case 5:  FOR_EACH_PIXEL1(5,  SWAP5BYTES(ptr0, ptr1))  break;
		case 6:  FOR_EACH_PIXEL1(6,  SWAP6BYTES(ptr0, ptr1))  break;
		case 8:  FOR_EACH_PIXEL1(8,  SWAP8BYTES(ptr0, ptr1))  break;
		case 12: FOR_EACH_PIXEL1(12, SWAP12BYTES(ptr0, ptr1)) break;
		case 16: FOR_EACH_PIXEL1(16, SWAP16BYTES(ptr0, ptr1)) break;
		case 24: FOR_EACH_PIXEL1(24, SWAP24BYTES(ptr0, ptr1)) break;
		case 32: FOR_EACH_PIXEL1(32, SWAP32BYTES(ptr0, ptr1)) break;
		default: VERA_ASSERT_MSG(false, "invalid pixel size");
		}
	}

#undef FOR_EACH_PIXEL0
#undef FOR_EACH_PIXEL1
}

void ImageEdit::flip(Image& result, const Image& image, ImageFlipFlags flags)
{
	ImageEdit::flip(
		result,
		image,
		flags.has(ImageFlipFlagBits::Horizontal),
		flags.has(ImageFlipFlagBits::Vertical));
}

void ImageEdit::rotate(Image& result, const Image& image, Rotation rot)
{
	switch (rot) {
	case Rotation::_90Deg:  ImageEdit::rotateCW(result, image); break;
	case Rotation::_180Deg: ImageEdit::flip(result, image, true, true); break;
	case Rotation::_270Deg: ImageEdit::rotateCCW(result, image); break;
	default:                result = image; break;
	}
}

void ImageEdit::rotateCW(Image& result, const Image& image)
{
	result = ImageEdit::rotateCW(image);
}

void ImageEdit::rotateCCW(Image& result, const Image& image)
{
	result = ImageEdit::rotateCCW(image);
}

VERA_NAMESPACE_END
