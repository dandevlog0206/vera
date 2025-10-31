#pragma once

#include "extent.h"
#include "../math/math_util.h"
#include <type_traits>

VERA_NAMESPACE_BEGIN

template <class T, bool Integral>
struct rect_base
{
	using value_type  = T;
	using size_type   = T;
	using vector_type = vector_base<2, value_type, packed_highp>;
	using extent_type = extent_base<2, size_type>; 

	VERA_CONSTEXPR rect_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR rect_base(value_type x, value_type y, size_type width, size_type height) VERA_NOEXCEPT :
		x(x), y(y), width(width), height(height) {};
	
	VERA_CONSTEXPR rect_base(const value_type& position, const extent_type& extent) VERA_NOEXCEPT :
		x(position.x), y(position.y), width(extent.width), height(extent.height) {};

	value_type x;
	value_type y;
	size_type  width;
	size_type  height;
};

template <class T>
struct rect_base<T, true>
{
	using value_type  = T;
	using size_type   = std::make_unsigned_t<T>;
	using point_type  = vector_base<2, value_type, packed_highp>;
	using extent_type = extent_base<2, size_type>; 

	VERA_CONSTEXPR rect_base() VERA_NOEXCEPT = default;

	VERA_CONSTEXPR rect_base(value_type x, value_type y, size_type width, size_type height) VERA_NOEXCEPT :
		x(x), y(y), width(width), height(height) {};
	
	VERA_CONSTEXPR rect_base(const point_type& position, const extent_type& extent) VERA_NOEXCEPT :
		x(position.x), y(position.y), width(extent.width), height(extent.height) {};

	value_type x;
	value_type y;
	size_type  width;
	size_type  height;
};

template <class T>
struct rect : public rect_base<T, std::is_integral_v<T>>
{
	using base_type = rect_base<T, std::is_integral_v<T>>;

	using value_type  = typename base_type::value_type;
	using size_type   = typename base_type::size_type;
	using point_type  = typename base_type::point_type;
	using extent_type = typename base_type::extent_type;

	using base_type::base_type;

	VERA_NODISCARD VERA_CONSTEXPR point_type position() const VERA_NOEXCEPT
	{
		return { this->x, this->y };
	}
	
	VERA_NODISCARD VERA_CONSTEXPR extent_type extent() const VERA_NOEXCEPT
	{
		return { this->width, this->height };
	}

	VERA_NODISCARD VERA_CONSTEXPR size_type area() const VERA_NOEXCEPT
	{
		return this->width * this->height;
	}

	VERA_NODISCARD VERA_CONSTEXPR point_type center() const VERA_NOEXCEPT
	{
		return {
			static_cast<value_type>(this->x + this->width / static_cast<size_type>(2)),
			static_cast<value_type>(this->y + this->height / static_cast<size_type>(2)),
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR value_type min_x() const VERA_NOEXCEPT
	{
		return this->x;
	}

	VERA_NODISCARD VERA_CONSTEXPR value_type max_x() const VERA_NOEXCEPT
	{
		return this->x + this->width;
	}

	VERA_NODISCARD VERA_CONSTEXPR value_type min_y() const VERA_NOEXCEPT
	{
		return this->y;
	}

	VERA_NODISCARD VERA_CONSTEXPR value_type max_y() const VERA_NOEXCEPT
	{
		return this->y + this->height;
	}

	VERA_NODISCARD VERA_CONSTEXPR point_type upper_left() const VERA_NOEXCEPT
	{
		return {
			static_cast<value_type>(this->x),
			static_cast<value_type>(this->y),
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR point_type upper_right() const VERA_NOEXCEPT
	{
		return {
			static_cast<value_type>(this->x + this->width),
			static_cast<value_type>(this->y),
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR point_type bottom_left() const VERA_NOEXCEPT
	{
		return {
			static_cast<value_type>(this->x),
			static_cast<value_type>(this->y + this->height),
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR point_type bottom_right() const VERA_NOEXCEPT
	{
		return {
			static_cast<value_type>(this->x + this->width),
			static_cast<value_type>(this->y + this->height),
		};
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const rect& rect) const VERA_NOEXCEPT
	{
		const auto upper_left   = rect.upper_left();
		const auto bottom_right = rect.bottom_right();

		return in_range(upper_left.x, this->x, this->x + this->width) &&
			in_range(upper_left.y, this->y, this->y + this->height) &&
			in_range(bottom_right.x, this->x, this->x + this->width) &&
			in_range(bottom_right.y, this->y, this->y + this->height);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool contain(const point_type& pos) const VERA_NOEXCEPT
	{
		return in_range<value_type>(pos.x, this->x, this->x + this->width) &&
			in_range<value_type>(pos.y, this->y, this->y + this->height);
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const rect& rhs) const VERA_NOEXCEPT
	{
		return this->x == rhs.x && this->y == rhs.y && this->width == rhs.width && this->height == rhs.height;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const rect& rhs) const VERA_NOEXCEPT
	{
		return !(*this == rhs);
	}
};

typedef rect<float>    rect2d;
typedef rect<int32_t>  irect2d;
typedef rect<uint32_t> urect2d;

VERA_NAMESPACE_END