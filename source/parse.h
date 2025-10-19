#pragma once

#include "../include/vera/core/coredefs.h"
#include <type_traits>
#include <bit>

VERA_NAMESPACE_BEGIN

static int8_t parse_i8_le(const uint8_t* data, uint32_t& offset)
{
	return static_cast<int8_t>(data[offset++]);
}

static uint8_t parse_u8_le(const uint8_t* data, uint32_t& offset)
{
	return data[offset++];
}

static int16_t parse_i16_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const int16_t*>(data + offset);
		offset += 2;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = static_cast<int16_t>(data[offset]) |
					 static_cast<int16_t>(data[offset + 1] << 8);
		offset += 2;
		return value;
	}
}

static uint16_t parse_u16_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const uint16_t*>(data + offset);
		offset += 2;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = static_cast<uint16_t>(data[offset]) |
					 static_cast<uint16_t>(data[offset + 1] << 8);
		offset += 2;
		return value;
	}
}

static int32_t parse_i32_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const int32_t*>(data + offset);
		offset += 4;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = static_cast<int32_t>(data[offset]) |
					 static_cast<int32_t>(data[offset + 1] << 8) |
					 static_cast<int32_t>(data[offset + 2] << 16) |
					 static_cast<int32_t>(data[offset + 3] << 24);
		offset += 4;
		return value;
	}
}

static uint32_t parse_u32_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const uint32_t*>(data + offset);
		offset += 4;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = static_cast<uint32_t>(data[offset]) |
					 static_cast<uint32_t>(data[offset + 1] << 8) |
					 static_cast<uint32_t>(data[offset + 2] << 16) |
					 static_cast<uint32_t>(data[offset + 3] << 24);
		offset += 4;
		return value;
	}
}

static int64_t parse_i64_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const int64_t*>(data + offset);
		offset += 8;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = static_cast<int64_t>(data[offset]) |
					 static_cast<int64_t>(data[offset + 1]) << 8 |
					 static_cast<int64_t>(data[offset + 2]) << 16 |
					 static_cast<int64_t>(data[offset + 3]) << 24 |
					 static_cast<int64_t>(data[offset + 4]) << 32 |
					 static_cast<int64_t>(data[offset + 5]) << 40 |
					 static_cast<int64_t>(data[offset + 6]) << 48 |
					 static_cast<int64_t>(data[offset + 7]) << 56;
		offset += 8;
		return value;
	}
}

static uint64_t parse_u64_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const uint64_t*>(data + offset);
		offset += 8;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = static_cast<uint64_t>(data[offset]) |
					 static_cast<uint64_t>(data[offset + 1]) << 8 |
					 static_cast<uint64_t>(data[offset + 2]) << 16 |
					 static_cast<uint64_t>(data[offset + 3]) << 24 |
					 static_cast<uint64_t>(data[offset + 4]) << 32 |
					 static_cast<uint64_t>(data[offset + 5]) << 40 |
					 static_cast<uint64_t>(data[offset + 6]) << 48 |
					 static_cast<uint64_t>(data[offset + 7]) << 56;
		offset += 8;
		return value;
	}
}

static float parse_f32_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const float*>(data + offset);
		offset += 4;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		uint32_t int_value = parse_u32_le(data, offset);
		return *reinterpret_cast<float*>(&int_value);
	}
}

static double parse_f64_le(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = *reinterpret_cast<const double*>(data + offset);
		offset += 8;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		uint64_t int_value = parse_u64_le(data, offset);
		return *reinterpret_cast<double*>(&int_value);
	}
}

template <class EnumType>
static EnumType parse_enum_le(const uint8_t* data, uint32_t& offset)
{
	using underlying_type = std::underlying_type_t<EnumType>;

	if constexpr (std::is_same_v<underlying_type, int8_t>) {
		return static_cast<EnumType>(parse_i8_le(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint8_t>) {
		return static_cast<EnumType>(parse_u8_le(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, int16_t>) {
		return static_cast<EnumType>(parse_i16_le(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint16_t>) {
		return static_cast<EnumType>(parse_u16_le(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, int32_t>) {
		return static_cast<EnumType>(parse_i32_le(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint32_t>) {
		return static_cast<EnumType>(parse_u32_le(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, int64_t>) {
		return static_cast<EnumType>(parse_i64_le(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint64_t>) {
		return static_cast<EnumType>(parse_u64_le(data, offset));
	} else {
		static_assert("unsupported enum underlying type for parse_enum_le");
	}
}

template <class T>
static T* parse_array_le(T* arr, size_t count, const uint8_t* data, uint32_t& offset)
{
	for (size_t i = 0; i < count; i++) {
		if constexpr (std::is_same_v<T, int8_t>) {
			arr[i] = parse_i8_le(data, offset);
		} else if constexpr (std::is_same_v<T, uint8_t>) {
			arr[i] = parse_u8_le(data, offset);
		} else if constexpr (std::is_same_v<T, int16_t>) {
			arr[i] = parse_i16_le(data, offset);
		} else if constexpr (std::is_same_v<T, uint16_t>) {
			arr[i] = parse_u16_le(data, offset);
		} else if constexpr (std::is_same_v<T, int32_t>) {
			arr[i] = parse_i32_le(data, offset);
		} else if constexpr (std::is_same_v<T, uint32_t>) {
			arr[i] = parse_u32_le(data, offset);
		} else if constexpr (std::is_same_v<T, int64_t>) {
			arr[i] = parse_i64_le(data, offset);
		} else if constexpr (std::is_same_v<T, uint64_t>) {
			arr[i] = parse_u64_le(data, offset);
		} else {
			static_assert(sizeof(T) == 0, "unsupported type for parse_array_le");
		}
	}

	return arr;
}

static int8_t parse_i8_be(const uint8_t* data, uint32_t& offset)
{
	return static_cast<int8_t>(data[offset++]);
}

static uint8_t parse_u8_be(const uint8_t* data, uint32_t& offset)
{
	return data[offset++];
}

static int16_t parse_i16_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = static_cast<int16_t>(data[offset] << 8) |
					 static_cast<int16_t>(data[offset + 1]);
		offset += 2;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const int16_t*>(data + offset);
		offset += 2;
		return value;
	}
}

static uint16_t parse_u16_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = static_cast<uint16_t>(data[offset] << 8) |
					 static_cast<uint16_t>(data[offset + 1]);
		offset += 2;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const uint16_t*>(data + offset);
		offset += 2;
		return value;
	}
}

static int32_t parse_i32_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = static_cast<int32_t>(data[offset] << 24) |
					 static_cast<int32_t>(data[offset + 1] << 16) |
					 static_cast<int32_t>(data[offset + 2] << 8)  |
					 static_cast<int32_t>(data[offset + 3]);
		offset += 4;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const int32_t*>(data + offset);
		offset += 4;
		return value;
	}
}

static uint32_t parse_u32_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = static_cast<uint32_t>(data[offset] << 24) |
					 static_cast<uint32_t>(data[offset + 1] << 16) |
					 static_cast<uint32_t>(data[offset + 2] << 8)  |
					 static_cast<uint32_t>(data[offset + 3]);
		offset += 4;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const uint32_t*>(data + offset);
		offset += 4;
		return value;
	}
}

static int64_t parse_i64_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = static_cast<int64_t>(data[offset]) << 56 |
					 static_cast<int64_t>(data[offset + 1]) << 48 |
					 static_cast<int64_t>(data[offset + 2]) << 40 |
					 static_cast<int64_t>(data[offset + 3]) << 32 |
					 static_cast<int64_t>(data[offset + 4]) << 24 |
					 static_cast<int64_t>(data[offset + 5]) << 16 |
					 static_cast<int64_t>(data[offset + 6]) << 8  |
					 static_cast<int64_t>(data[offset + 7]);
		offset += 8;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const int64_t*>(data + offset);
		offset += 8;
		return value;
	}
}

static uint64_t parse_u64_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		auto value = static_cast<uint64_t>(data[offset]) << 56 |
					 static_cast<uint64_t>(data[offset + 1]) << 48 |
					 static_cast<uint64_t>(data[offset + 2]) << 40 |
					 static_cast<uint64_t>(data[offset + 3]) << 32 |
					 static_cast<uint64_t>(data[offset + 4]) << 24 |
					 static_cast<uint64_t>(data[offset + 5]) << 16 |
					 static_cast<uint64_t>(data[offset + 6]) << 8  |
					 static_cast<uint64_t>(data[offset + 7]);
		offset += 8;
		return value;
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const uint64_t*>(data + offset);
		offset += 8;
		return value;
	}
}

static float parse_f32_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		uint32_t int_value = parse_u32_be(data, offset);
		return *reinterpret_cast<float*>(&int_value);
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const float*>(data + offset);
		offset += 4;
		return value;
	}
}

static double parse_f64_be(const uint8_t* data, uint32_t& offset)
{
	if constexpr (std::endian::native == std::endian::little) {
		uint64_t int_value = parse_u64_be(data, offset);
		return *reinterpret_cast<double*>(&int_value);
	} else if constexpr (std::endian::native == std::endian::big) {
		auto value = *reinterpret_cast<const double*>(data + offset);
		offset += 8;
		return value;
	}
}

template <class EnumType>
static EnumType parse_enum_be(const uint8_t* data, uint32_t& offset)
{
	using underlying_type = std::underlying_type_t<EnumType>;

	if constexpr (std::is_same_v<underlying_type, int8_t>) {
		return static_cast<EnumType>(parse_i8_be(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint8_t>) {
		return static_cast<EnumType>(parse_u8_be(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, int16_t>) {
		return static_cast<EnumType>(parse_i16_be(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint16_t>) {
		return static_cast<EnumType>(parse_u16_be(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, int32_t>) {
		return static_cast<EnumType>(parse_i32_be(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint32_t>) {
		return static_cast<EnumType>(parse_u32_be(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, int64_t>) {
		return static_cast<EnumType>(parse_i64_be(data, offset));
	} else if constexpr (std::is_same_v<underlying_type, uint64_t>) {
		return static_cast<EnumType>(parse_u64_be(data, offset));
	} else {
		static_assert("unsupported enum underlying type for parse_enum_be");
	}
}

template <class T>
static T* parse_array_be(T* arr, size_t count, const uint8_t* data, uint32_t& offset)
{
	for (size_t i = 0; i < count; i++) {
		if constexpr (std::is_same_v<T, int8_t>) {
			arr[i] = parse_i8_be(data, offset);
		} else if constexpr (std::is_same_v<T, uint8_t>) {
			arr[i] = parse_u8_be(data, offset);
		} else if constexpr (std::is_same_v<T, int16_t>) {
			arr[i] = parse_i16_be(data, offset);
		} else if constexpr (std::is_same_v<T, uint16_t>) {
			arr[i] = parse_u16_be(data, offset);
		} else if constexpr (std::is_same_v<T, int32_t>) {
			arr[i] = parse_i32_be(data, offset);
		} else if constexpr (std::is_same_v<T, uint32_t>) {
			arr[i] = parse_u32_be(data, offset);
		} else if constexpr (std::is_same_v<T, int64_t>) {
			arr[i] = parse_i64_be(data, offset);
		} else if constexpr (std::is_same_v<T, uint64_t>) {
			arr[i] = parse_u64_be(data, offset);
		} else if constexpr (std::is_same_v<T, float>) {
			arr[i] = parse_f32_be(data, offset);
		} else if constexpr (std::is_same_v<T, double>) {
			arr[i] = parse_f64_be(data, offset);
		} else {
			static_assert(sizeof(T) == 0, "unsupported type for parse_array_be");
		}
	}

	return arr;
}

VERA_NAMESPACE_END