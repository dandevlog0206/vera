#pragma once

#include "../core/coredefs.h"

VERA_NAMESPACE_BEGIN

/*
Unorm   : 0~1
Snorm   : -1~1
Uscaled : 0 ~ 2^n
Sscaled : -2^(n-1) ~ 2^(n-1) - 1
Uint    : 0 ~ 2^n
Sint    : -2^(n-1) ~ 2^(n-1) - 1
Float  : fp16, float, double
*/
enum class Format
{
	Unknown,

	// 1 channel
	R8Unorm,
	R8Snorm,
	R8Uscaled,
	R8Sscaled,
	R8Uint,
	R8Sint,
	R8Srgb,
	R16Unorm,
	R16Snorm,
	R16Uscaled,
	R16Sscaled,
	R16Uint,
	R16Sint,
	R16Float,
	R32Uint,
	R32Sint,
	R32Float,
	R64Uint,
	R64Sint,
	R64Float,

	// 2 channel
	RG8Unorm,
	RG8Snorm,
	RG8Uscaled,
	RG8Sscaled,
	RG8Uint,
	RG8Sint,
	RG8Srgb,
	RG16Unorm,
	RG16Snorm,
	RG16Uscaled,
	RG16Sscaled,
	RG16Uint,
	RG16Sint,
	RG16Float,
	RG32Uint,
	RG32Sint,
	RG32Float,
	RG64Uint,
	RG64Sint,
	RG64Float,

	// 3 channel
	RGB8Unorm,
	RGB8Snorm,
	RGB8Uscaled,
	RGB8Sscaled,
	RGB8Uint,
	RGB8Sint,
	RGB8Srgb,
	BGR8Unorm,
	BGR8Snorm,
	BGR8Uscaled,
	BGR8Sscaled,
	BGR8Uint,
	BGR8Sint,
	BGR8Srgb,
	RGB16Unorm,
	RGB16Snorm,
	RGB16Uscaled,
	RGB16Sscaled,
	RGB16Uint,
	RGB16Sint,
	RGB16Float,
	RGB32Uint,
	RGB32Sint,
	RGB32Float,
	RGB64Uint,
	RGB64Sint,
	RGB64Float,

	// 4 channel
	RGBA8Unorm,
	RGBA8Snorm,
	RGBA8Uscaled,
	RGBA8Sscaled,
	RGBA8Uint,
	RGBA8Sint,
	RGBA8Srgb,
	BGRA8Unorm,
	BGRA8Snorm,
	BGRA8Uscaled,
	BGRA8Sscaled,
	BGRA8Uint,
	BGRA8Sint,
	BGRA8Srgb,
	RGBA16Unorm,
	RGBA16Snorm,
	RGBA16Uscaled,
	RGBA16Sscaled,
	RGBA16Uint,
	RGBA16Sint,
	RGBA16Float,
	RGBA32Uint,
	RGBA32Sint,
	RGBA32Float,
	RGBA64Uint,
	RGBA64Sint,
	RGBA64Float,

	// Pack 8bit
	RG4UnormPack8,

	// Pack 16bit
	RGBA4UnormPack16,
	BGRA4UnormPack16,
	R5G6B5UnormPack16,
	B5G6R5UnormPack16,
	R5G5B5A1UnormPack16,
	B5G5R5A1UnormPack16,
	A1R5G5B5UnormPack16,
	A4R4G4B4UnormPack16,
	A4B4G4R4UnormPack16,
	A1B5G5R5UnormPack16,

	// Pack 32bit
	ABGR8UnormPack32,
	ABGR8SnormPack32,
	ABGR8UscaledPack32,
	ABGR8SscaledPack32,
	ABGR8UintPack32,
	ABGR8SintPack32,
	ABGR8SrgbPack32,
	A2RGB10UnormPack32,
	A2RGB10SnormPack32,
	A2RGB10UscaledPack32,
	A2RGB10SscaledPack32,
	A2RGB10UintPack32,
	A2RGB10SintPack32,
	A2BGR10UnormPack32,
	A2BGR10SnormPack32,
	A2BGR10UscaledPack32,
	A2BGR10SscaledPack32,
	A2BGR10UintPack32,
	A2BGR10SintPack32,

	// Depth
	D16Unorm,
	X8D24Unorm,
	D32Float,

	// Stencil 
	S8Uint,

	// Depth Stencil
	D16UnormS8Uint,
	D24UnormS8Uint,
	D32FloatS8Uint,

	// Alpha
	A8Unorm
};

enum class VectorFormat
{
	Char   = Format::R8Sint,
	UChar  = Format::R8Uint,
	Short  = Format::R16Sint,
	UShort = Format::R16Uint,
	Int    = Format::R32Sint,
	UInt   = Format::R32Uint,
	Long   = Format::R64Sint,
	ULong  = Format::R64Uint,
	Float  = Format::R32Float,
	Double = Format::R64Float,

	Char2   = Format::RG8Sint,
	UChar2  = Format::RG8Uint,
	Short2  = Format::RG16Sint,
	UShort2 = Format::RG16Uint,
	Int2    = Format::RG32Sint,
	UInt2   = Format::RG32Uint,
	Long2   = Format::RG64Sint,
	ULong2  = Format::RG64Uint,
	Float2  = Format::RG32Float,
	Double2 = Format::RG64Float,

	Char3   = Format::RGB8Sint,
	UChar3  = Format::RGB8Uint,
	Short3  = Format::RGB16Sint,
	UShort3 = Format::RGB16Uint,
	Int3    = Format::RGB32Sint,
	UInt3   = Format::RGB32Uint,
	Long3   = Format::RGB64Sint,
	ULong3  = Format::RGB64Uint,
	Float3  = Format::RGB32Float,
	Double3 = Format::RGB64Float,
	
	Char4   = Format::RGBA8Sint,
	UChar4  = Format::RGBA8Uint,
	Short4  = Format::RGBA16Sint,
	UShort4 = Format::RGBA16Uint,
	Int4    = Format::RGBA32Sint,
	UInt4   = Format::RGBA32Uint,
	Long4   = Format::RGBA64Sint,
	ULong4  = Format::RGBA64Uint,
	Float4  = Format::RGBA32Float,
	Double4 = Format::RGBA64Float,
};

enum class ColorFormat
{

};

enum class DepthFormat
{
	D16Unorm   = Format::D16Unorm,
	X8D24Unorm = Format::X8D24Unorm,
	D32Float   = Format::D32Float
};

enum class StencilFormat
{
	S8Uint = Format::S8Uint
};

enum class DepthStencilFormat
{
	D16UnormS8Uint = Format::D16UnormS8Uint,
	D24UnormS8Uint = Format::D24UnormS8Uint,
	D32FloatS8Uint = Format::D32FloatS8Uint
};

enum class AlphaFormat
{
	A8Unorm = Format::A8Unorm
};

VERA_NAMESPACE_END