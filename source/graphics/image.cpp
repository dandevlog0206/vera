#include "../../include/vera/graphics/image.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
////////// custom allocator for stb_image /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// allocate memory with align of sizeof(size_t)
static void* malloc_impl(size_t size)
{
	size_t  total_size = (size - 1) / sizeof(size_t) + 2;
	size_t* new_memory = new size_t[total_size];
	
	// first element of memory block contains size
	*new_memory = size;

	return new_memory + 1;
}

static void* realloc_impl(void* ptr, size_t new_size)
{
	if (void* new_memory = malloc_impl(new_size)) {
		if (!ptr) return new_memory;
		
		size_t* my_ptr = reinterpret_cast<size_t*>(ptr) - 1;
		size_t  my_size = my_ptr[0];

		memcpy(new_memory, ptr, std::min(my_size, new_size));
		delete[] my_ptr;

		return new_memory;
	}

	return nullptr;
}

static void free_impl(void* ptr)
{
	if (!ptr) return;

	delete[] (reinterpret_cast<size_t*>(ptr) - 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#define __STDC_LIB_EXT1__
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MALLOC(sz)           malloc_impl(sz)
#define STBI_REALLOC(p,newsz)     realloc_impl(p,newsz)
#define STBI_FREE(p)              free_impl(p)

#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/assertion.h"
#include "../../include/vera/graphics/format_traits.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <fstream>
#include <filesystem>

VERA_NAMESPACE_BEGIN

Image Image::loadFromFile(std::string_view path)
{
	Image result;
	auto  file_path = std::filesystem::path(path);
	auto  extension = file_path.extension().generic_string();

	if (extension == ".png" || extension == ".jpg" || extension == ".bmp") {
		int   width, height, comp;
		void* ptr;

		if (ptr = stbi_load(path.data(), &width, &height, &comp, 4)) {
			result.m_width  = width;
			result.m_height = height;

			if (comp == 1) {
				size_t pixel_count = static_cast<size_t>(width * height);

				result.m_format    = Format::RGBA8Unorm;
				result.m_allocated = pixel_count * 4;
				result.m_ptr       = malloc_impl(result.m_allocated);

				auto* pixel = reinterpret_cast<uint8_t*>(result.m_ptr);
				auto* color = reinterpret_cast<uint8_t*>(ptr);

				for (size_t i = 0; i < pixel_count; ++i) {
					pixel[4 * i + 0] = color[i];
					pixel[4 * i + 1] = color[i];
					pixel[4 * i + 2] = color[i];
					pixel[4 * i + 3] = 255;
				}

				stbi_image_free(ptr);
			} else if (comp == 2) {
				size_t pixel_count = static_cast<size_t>(width * height);

				result.m_format    = Format::RGBA8Unorm;
				result.m_allocated = pixel_count * 4;
				result.m_ptr       = malloc_impl(result.m_allocated);

				auto* pixel = reinterpret_cast<uint8_t*>(result.m_ptr);

				for (size_t i = 0; i < pixel_count; ++i) {
					auto color = reinterpret_cast<uint8_t*>(ptr)[2 * i + 0];
					auto alpha = reinterpret_cast<uint8_t*>(ptr)[2 * i + 1];
					
					pixel[4 * i + 0] = color;
					pixel[4 * i + 1] = color;
					pixel[4 * i + 2] = color;
					pixel[4 * i + 3] = alpha;
				}

				stbi_image_free(ptr);
			} else if (comp == 3) {
				result.m_ptr       = ptr;
				result.m_format    = Format::RGBA8Unorm;
				result.m_allocated = width * height * get_format_size(result.m_format);
			} else /* comp == 4 */ {
				result.m_ptr       = ptr;
				result.m_format    = Format::RGBA8Unorm;
				result.m_allocated = width * height * get_format_size(result.m_format);
			}

			return result;
		}

		throw Exception("unable to load image at " + std::string(path));
	}

	throw Exception("unsupported image extension");
}

Image Image::loadFromFile(uint32_t width, uint32_t height, Format format, std::string_view path)
{
	// TODO: implement
	return Image();
}

Image Image::loadFromMemory(uint32_t width, uint32_t height, Format format, void* ptr, size_t size)
{
	VERA_ASSERT(width * height * get_format_size(format) <= size);

	Image result;
	result.m_width     = width;
	result.m_height    = height;
	result.m_format    = format;
	result.m_allocated = size;
	result.m_ptr       = ptr;

	return result;
}

Image::Image() :
	m_width(0),
	m_height(0),
	m_format(Format::Unknown),
	m_allocated(0),
	m_ptr(nullptr) {}

Image::Image(std::string_view path) :
	Image(loadFromFile(path)) {}

Image::Image(uint32_t width, uint32_t height, Format format) :
	m_width(width),
	m_height(height),
	m_format(format),
	m_allocated(width* height* get_format_size(format)),
	m_ptr(malloc_impl(m_allocated)) {}

Image::Image(uint32_t width, uint32_t height, Format format, const void* ptr) :
	m_width(width),
	m_height(height),
	m_format(format),
	m_allocated(width* height* get_format_size(format)),
	m_ptr(malloc_impl(m_allocated))
{
	memcpy(m_ptr, ptr, m_allocated);
}

Image::Image(const Image& rhs) :
	m_width(0),
	m_height(0),
	m_format(Format::Unknown),
	m_allocated(0),
	m_ptr(nullptr) {}

Image::Image(Image&& rhs) noexcept :
	m_width(std::exchange(rhs.m_width, 0)),
	m_height(std::exchange(rhs.m_height, 0)),
	m_format(std::exchange(rhs.m_format, Format::Unknown)),
	m_allocated(std::exchange(rhs.m_allocated, 0)),
	m_ptr(std::exchange(rhs.m_ptr, nullptr)) {}

Image::~Image()
{
	free_impl(m_ptr);
}

Image& Image::operator=(const Image& rhs)
{
	auto new_size = rhs.size();

	m_width  = rhs.m_width;
	m_height = rhs.m_height;
	m_format = rhs.m_format;
	
	if (m_allocated < new_size) {
		free_impl(m_ptr);
		m_ptr       = malloc_impl(new_size);
		m_allocated = new_size;
	}

	memcpy(m_ptr, rhs.m_ptr, new_size);

	return *this;
}

Image& Image::operator=(Image&& rhs) noexcept
{
	clear();

	m_width     = std::exchange(rhs.m_width, 0);
	m_height    = std::exchange(rhs.m_height, 0);
	m_format    = std::exchange(rhs.m_format, Format::Unknown);
	m_allocated = std::exchange(rhs.m_allocated, 0);
	m_ptr       = std::exchange(rhs.m_ptr, nullptr);

	return *this;
}

void Image::clear()
{
	m_width     = 0;
	m_height    = 0;
	m_format    = Format::Unknown;
	m_allocated = 0;
	free_impl(std::exchange(m_ptr, nullptr));
}

size_t Image::size() const
{
	return m_width * m_height * get_format_size(m_format);
}

size_t Image::capacity() const
{
	return m_allocated;
}

uint32_t Image::width() const
{
	return m_width;
}

uint32_t Image::height() const
{
	return m_height;
}

Format Image::format() const
{
	return m_format;
}

void* Image::data()
{
	return m_ptr;
}

const void* Image::data() const
{
	return m_ptr;
}

void Image::saveToFile(std::string_view path)
{
	if (!m_ptr)
		throw Exception("cannot save empty image");

	auto file_path = std::filesystem::path(path);
	auto extension = file_path.extension().generic_string();
	int  comp      = get_format_component_count(m_format);

	if (extension == "png") {
		if (!stbi_write_png(path.data(), m_width, m_height, comp, m_ptr, 0))
			throw Exception("failed to save image at " + std::string(path));
	} else if (extension == "jpg") {
		if (!stbi_write_jpg(path.data(), m_width, m_height, comp, m_ptr, 0))
			throw Exception("failed to save image at " + std::string(path));
	} else if (extension == "bmp") {
		if (!stbi_write_bmp(path.data(), m_width, m_height, comp, m_ptr))
			throw Exception("failed to save image at " + std::string(path));
	} else
		throw Exception("unsupported image extension");
}

bool Image::empty() const
{
	return !m_ptr;
}

bool Image::hasAlpha() const
{
	return format_has_alpha(m_format);
}

void Image::swap(Image& rhs) noexcept
{
	std::swap(m_width, rhs.m_width);
	std::swap(m_height, rhs.m_height);
	std::swap(m_format, rhs.m_format);
	std::swap(m_allocated, rhs.m_allocated);
	std::swap(m_ptr, rhs.m_ptr);
}

VERA_NAMESPACE_END