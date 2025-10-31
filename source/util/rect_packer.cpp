#include "../../include/vera/util/rect_packer.h"

VERA_NAMESPACE_BEGIN

std::unique_ptr<RectPacker> RectPacker::createUnique(
	PackingMethod method,
	uint32_t      width,
	uint32_t      height,
	uint32_t      padding
) VERA_NOEXCEPT {
	switch (method) {
	case PackingMethod::Shelf:
		return std::make_unique<ShelfPacker>(width, height, padding);
	}

	VERA_ASSERT_MSG(false, "Unsupported packing method");
	return nullptr;
}

RectPacker::RectPacker(uint32_t width, uint32_t height, uint32_t padding) VERA_NOEXCEPT :
	m_width(width),
	m_height(height),
	m_padding(padding) {}

uint32_t RectPacker::getWidth() const VERA_NOEXCEPT
{
	return m_width;
}

uint32_t RectPacker::getHeight() const VERA_NOEXCEPT
{
	return m_height;
}

extent2d RectPacker::getExtent() const VERA_NOEXCEPT
{
	return extent2d(m_width, m_height);
}

uint32_t RectPacker::getPadding() const VERA_NOEXCEPT
{
	return m_padding;
}

ShelfPacker::ShelfPacker(uint32_t width, uint32_t height, uint32_t padding) VERA_NOEXCEPT :
	RectPacker(width, height, padding),
	m_cursor(padding, padding),
	m_max_height(0) {}

VERA_NODISCARD uint32_t ShelfPacker::pack(const extent2d& extent, urect2d& out_rect) VERA_NOEXCEPT
{
	uint32_t width  = extent.width;
	uint32_t height = extent.height;

	if (width + m_padding * 2 > m_width || height + m_padding * 2 > m_height)
		return 0;

	if (m_cursor.x + width + m_padding > m_width) {
		m_cursor.x  = m_padding;
		m_cursor.y += m_max_height + m_padding;
	}

	if (m_cursor.y + height + m_padding > m_height)
		return 0;

	out_rect = urect2d(m_cursor.x, m_cursor.y, width, height);

	m_cursor.x  += width + m_padding;
	m_max_height = std::max(m_max_height, height);

	return 1;
}

VERA_NODISCARD uint32_t ShelfPacker::pack(
	array_view<extent2d>  extents,
	std::vector<urect2d>& out_rects
) VERA_NOEXCEPT {
	urect2d rect;
	
	out_rects.clear();
	out_rects.reserve(extents.size());

	for (size_t i = 0; i < extents.size(); ++i) {
		if (pack(extents[i], rect) == 0)
			return static_cast<uint32_t>(i);

		out_rects.push_back(rect);
	}

	return extents.size();
}

VERA_NODISCARD uint32_t ShelfPacker::packInplace(std::span<urect2d> out_rects) VERA_NOEXCEPT {
	urect2d rect;

	for (size_t i = 0; i < out_rects.size(); ++i) {
		if (pack(out_rects[i].extent(), rect) == 0)
			return static_cast<uint32_t>(i);

		out_rects[i] = rect;
	}

	return out_rects.size();
}

void ShelfPacker::clear() VERA_NOEXCEPT
{
	m_cursor     = uint2(m_padding, m_padding);
	m_max_height = 0;
}

VERA_NAMESPACE_END