#pragma once

#include "array_view.h"
#include "extent.h"
#include "rect.h"
#include <memory>
#include <vector>
#include <span>

VERA_NAMESPACE_BEGIN

enum class PackingMethod VERA_ENUM
{
	Shelf
};

class RectPacker
{
public:
	VERA_NODISCARD static std::unique_ptr<RectPacker> createUnique(
		PackingMethod method,
		uint32_t      width,
		uint32_t      height,
		uint32_t      padding = 0) VERA_NOEXCEPT;

	RectPacker(uint32_t width, uint32_t height, uint32_t padding = 0) VERA_NOEXCEPT;
	virtual ~RectPacker() VERA_NOEXCEPT = default;

	VERA_NODISCARD uint32_t getWidth() const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t getHeight() const VERA_NOEXCEPT;
	VERA_NODISCARD extent2d getExtent() const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t getPadding() const VERA_NOEXCEPT;

	VERA_NODISCARD virtual uint32_t pack(const extent2d& extent, urect2d& out_rect) VERA_NOEXCEPT = 0;

	VERA_NODISCARD virtual uint32_t pack(
		array_view<extent2d>  extents,
		std::vector<urect2d>& out_rects) VERA_NOEXCEPT = 0;

	VERA_NODISCARD virtual uint32_t packInplace(std::span<urect2d> out_rects) VERA_NOEXCEPT = 0;

	virtual void clear() VERA_NOEXCEPT = 0;

protected:
	const uint32_t m_width;
	const uint32_t m_height;
	const uint32_t m_padding;
};

class ShelfPacker : public RectPacker
{
public:
	ShelfPacker(uint32_t width, uint32_t height, uint32_t padding = 0) VERA_NOEXCEPT;

	VERA_NODISCARD uint32_t pack(const extent2d& extent, urect2d& out_rect) VERA_NOEXCEPT override;

	VERA_NODISCARD uint32_t pack(
		array_view<extent2d>  extents,
		std::vector<urect2d>& out_rects
	) VERA_NOEXCEPT override;

	VERA_NODISCARD uint32_t packInplace(std::span<urect2d> out_rects) VERA_NOEXCEPT override;

	void clear() VERA_NOEXCEPT override;

private:
	uint2    m_cursor;
	uint32_t m_max_height;
};

VERA_NAMESPACE_END
