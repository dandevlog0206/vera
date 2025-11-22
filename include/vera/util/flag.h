#pragma once

#include "../core/assertion.h"
#include <type_traits>
#include <bit>

#define VERA_ENUM_FLAGS(bit_type, flag_type) ;                                           \
	typedef Flags<bit_type> flag_type;                                                   \
	VERA_CONSTEXPR Flags<bit_type> operator&(bit_type bit1, bit_type bit2) VERA_NOEXCEPT \
	{                                                                                    \
		return Flags<bit_type>(bit1) & bit2;                                             \
	}                                                                                    \
	VERA_CONSTEXPR Flags<bit_type> operator|(bit_type bit1, bit_type bit2) VERA_NOEXCEPT \
	{                                                                                    \
		return Flags<bit_type>(bit1) | bit2;                                             \
	}                                                                                    \
	VERA_CONSTEXPR Flags<bit_type> operator^(bit_type bit1, bit_type bit2) VERA_NOEXCEPT \
	{                                                                                    \
		return Flags<bit_type>(bit1) ^ bit2;                                             \
	}                                                                                    \

VERA_NAMESPACE_BEGIN

template <class BitType>
class Flags
{
public:
	using mask_type       = typename std::underlying_type<BitType>::type;
	using underlying_type = std::underlying_type<BitType>::type;

	VERA_CONSTEXPR Flags() VERA_NOEXCEPT : m_mask(0) {}

	VERA_CONSTEXPR Flags(BitType bit) VERA_NOEXCEPT : m_mask(static_cast<mask_type>(bit)) {}

	VERA_CONSTEXPR Flags(const Flags<BitType>& rhs) VERA_NOEXCEPT = default;

	VERA_CONSTEXPR explicit Flags(mask_type flags) VERA_NOEXCEPT : m_mask(flags) {}

	VERA_NODISCARD VERA_CONSTEXPR BitType flag_bit() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(count() == 1, "flag_bit() called on Flags with multiple or no bits set");
		return static_cast<BitType>(m_mask);
	}

	VERA_NODISCARD VERA_CONSTEXPR mask_type mask() const VERA_NOEXCEPT
	{
		return m_mask;
	}

	VERA_CONSTEXPR void set(const Flags<BitType>& flags, bool value) VERA_NOEXCEPT
	{
		if (value)
			m_mask |= flags.m_mask;
		else
			m_mask &= ~flags.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool has(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return (m_mask & rhs.m_mask) == rhs.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR uint32_t count() const VERA_NOEXCEPT
	{
		return static_cast<uint32_t>(std::popcount(m_mask));
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return !m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR auto operator<=>(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return m_mask <=> rhs.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return m_mask < rhs.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator<=(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return m_mask <= rhs.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return m_mask > rhs.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator>=(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return m_mask >= rhs.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator==(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return m_mask == rhs.m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool operator!=(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return m_mask != rhs.m_mask;
	}

	// logical operator
	VERA_NODISCARD VERA_CONSTEXPR bool operator!() const VERA_NOEXCEPT
	{
		return !m_mask;
	}

	// bitwise operators
	VERA_NODISCARD VERA_CONSTEXPR Flags<BitType> operator&(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return Flags<BitType>(m_mask & rhs.m_mask);
	}

	VERA_NODISCARD VERA_CONSTEXPR Flags<BitType> operator|(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return Flags<BitType>(m_mask | rhs.m_mask);
	}

	VERA_NODISCARD VERA_CONSTEXPR Flags<BitType> operator^(const Flags<BitType>& rhs) const VERA_NOEXCEPT
	{
		return Flags<BitType>(m_mask ^ rhs.m_mask);
	}

	// assignment operators
	VERA_CONSTEXPR Flags<BitType>& operator=(const Flags<BitType>& rhs) VERA_NOEXCEPT = default;
	
	VERA_CONSTEXPR Flags<BitType>& operator+=(const Flags<BitType>& rhs) VERA_NOEXCEPT
	{
		m_mask |= rhs.m_mask;
		return *this;
	}

	VERA_CONSTEXPR Flags<BitType>& operator-=(const Flags<BitType>& rhs) VERA_NOEXCEPT
	{
		m_mask &= ~rhs.m_mask;
		return *this;
	}

	VERA_CONSTEXPR Flags<BitType>& operator|=(const Flags<BitType>& rhs) VERA_NOEXCEPT
	{
		m_mask |= rhs.m_mask;
		return *this;
	}

	VERA_CONSTEXPR Flags<BitType>& operator&=(const Flags<BitType>& rhs) VERA_NOEXCEPT
	{
		m_mask &= rhs.m_mask;
		return *this;
	}

	VERA_CONSTEXPR Flags<BitType>& operator^=(const Flags<BitType>& rhs) VERA_NOEXCEPT
	{
		m_mask ^= rhs.m_mask;
		return *this;
	}

	// cast operators
	VERA_NODISCARD VERA_CONSTEXPR operator bool() const VERA_NOEXCEPT
	{
		return !!m_mask;
	}

	VERA_NODISCARD VERA_CONSTEXPR BitType operator()() const VERA_NOEXCEPT
	{
		return static_cast<BitType>(m_mask);
	}

	VERA_NODISCARD explicit VERA_CONSTEXPR operator mask_type() const VERA_NOEXCEPT
	{
		return m_mask;
	}

private:
	mask_type m_mask;
};

template <class BitType>
VERA_NODISCARD VERA_CONSTEXPR Flags<BitType> operator&(BitType bit, const Flags<BitType>& flags) VERA_NOEXCEPT
{
	return flags.operator&(bit);
}

template <class BitType>
VERA_NODISCARD VERA_CONSTEXPR Flags<BitType> operator|(BitType bit, const Flags<BitType>& flags) VERA_NOEXCEPT
{
	return flags.operator|(bit);
}

template <class BitType>
VERA_NODISCARD VERA_CONSTEXPR Flags<BitType> operator^(BitType bit, const Flags<BitType>& flags) VERA_NOEXCEPT
{
	return flags.operator^(bit);
}

VERA_NAMESPACE_END