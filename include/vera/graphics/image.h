#pragma once

#include "format.h"
#include <string_view>

VERA_NAMESPACE_BEGIN

class Image
{
public:
	static Image loadFromFile(std::string_view path);
	static Image loadFromFile(uint32_t width, uint32_t height, Format format, std::string_view path);
	static Image loadFromMemory(uint32_t width, uint32_t height, Format format, void* ptr, size_t size);

	Image();
	Image(uint32_t width, uint32_t height, Format format, const void* ptr);
	Image(const Image& rhs);
	Image(Image&& rhs) noexcept;
	~Image();

	Image& operator=(const Image& rhs);
	Image& operator=(Image&& rhs) noexcept;

	void clear();

	size_t size() const;
	size_t capacity() const;
	uint32_t width() const;
	uint32_t height() const;
	Format format() const;
	void* data();
	const void* data() const;

	void saveToFile(std::string_view path);

	bool empty() const;
	bool hasAlpha() const;

	void swap(Image& rhs) noexcept;

private:
	uint32_t m_width;
	uint32_t m_height;
	Format   m_format;
	size_t   m_allocated;
	void*    m_ptr;
};

VERA_NAMESPACE_END
