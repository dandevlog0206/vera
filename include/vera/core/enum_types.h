#pragma once

#include "../util/flag.h"

VERA_NAMESPACE_BEGIN

VERA_VK_ABI_COMPATIBLE enum class MemoryHeapFlagBits VERA_FLAG_BITS
{
	None          = 0,
	DeviceLocal   = 1 << 0,
	MultiInstance = 1 << 1
} VERA_ENUM_FLAGS(MemoryHeapFlagBits, MemoryHeapFlags)

VERA_VK_ABI_COMPATIBLE enum class MemoryPropertyFlagBits VERA_FLAG_BITS
{
	None            = 0,
	DeviceLocal     = 1 << 0,
	HostVisible     = 1 << 1,
	HostCoherent    = 1 << 2,
	HostCached      = 1 << 3,
	LazilyAllocated = 1 << 4,
	Protected       = 1 << 5
} VERA_ENUM_FLAGS(MemoryPropertyFlagBits, MemoryPropertyFlags)

enum class PipelineBindPoint VERA_ENUM
{
	Unknown,
	Graphics,
	Compute
};

VERA_VK_ABI_COMPATIBLE enum class PrimitiveTopology VERA_ENUM
{
	PointList                  = 0,
	LineList                   = 1,
	LineStrip                  = 2,
	TriangleList               = 3,
	TriangleStrip              = 4,
	TriangleFan                = 5,
	LineListWithAdjacency      = 6,
	LineStripWithAdjacency     = 7,
	TriangleListWithAdjacency  = 8,
	TriangleStripWithAdjacency = 9,
	PatchList                  = 10
};

VERA_VK_ABI_COMPATIBLE enum class PolygonMode VERA_ENUM
{
	Fill  = 0,
	Line  = 1,
	Point = 2
};

VERA_VK_ABI_COMPATIBLE enum class CullModeFlagBits VERA_ENUM
{
	None         = 0,
	Front        = 1 << 0,
	Back         = 1 << 1,
	FrontAndBack = Front | Back
} VERA_ENUM_FLAGS(CullModeFlagBits, CullModeFlags);

VERA_VK_ABI_COMPATIBLE enum class FrontFace VERA_ENUM
{
	CounterClockwise = 0,
	Clockwise        = 1
};

VERA_VK_ABI_COMPATIBLE enum class CompareOp VERA_ENUM
{
	Never          = 0,
	Less           = 1,
	Equal          = 2,
	LessOrEqual    = 3,
	Greater        = 4,
	NotEqual       = 5,
	GreaterOrEqual = 6,
	Always         = 7
};

VERA_VK_ABI_COMPATIBLE enum class StencilOp VERA_ENUM
{
	Keep              = 0,
	Zero              = 1,
	Replace           = 2,
	IncrementAndClamp = 3,
	DecrementAndClamp = 4,
	Invert            = 5,
	IncrementAndWrap  = 6,
	DecrementAndWrap  = 7
};

VERA_VK_ABI_COMPATIBLE enum class LogicOp VERA_ENUM
{
	Clear        = 0,
	And          = 1,
	AndReverse   = 2,
	Copy         = 3,
	AndInverted  = 4,
	NoOp         = 5,
	Xor          = 6,
	Or           = 7,
	Nor          = 8,
	Equivalent   = 9,
	Invert       = 10,
	OrReverse    = 11,
	CopyInverted = 12,
	OrInverted   = 13,
	Nand         = 14,
	Set          = 15
};

VERA_VK_ABI_COMPATIBLE enum class BlendFactor VERA_ENUM
{
	Zero                  = 0,
	One                   = 1,
	SrcColor              = 2,
	OneMinusSrcColor      = 3,
	DstColor              = 4,
	OneMinusDstColor      = 5,
	SrcAlpha              = 6,
	OneMinusSrcAlpha      = 7,
	DstAlpha              = 8,
	OneMinusDstAlpha      = 9,
	ConstantColor         = 10,
	OneMinusConstantColor = 11,
	ConstantAlpha         = 12,
	OneMinusConstantAlpha = 13,
	SrcAlphaSaturate      = 14,
	Src1Color             = 15,
	OneMinusSrc1Color     = 16,
	Src1Alpha             = 17,
	OneMinusSrc1Alpha     = 18
};

VERA_VK_ABI_COMPATIBLE enum class BlendOp VERA_ENUM
{
	Add             = 0,
	Subtract        = 1,
	ReverseSubtract = 2,
	Min             = 3,
	Max             = 4
};

VERA_VK_ABI_COMPATIBLE enum class ColorComponentFlagBits VERA_FLAG_BITS
{
	None = 0,
	R    = 1 << 0,
	G    = 1 << 1,
	B    = 1 << 2,
	A    = 1 << 3,
	RGB  = R | G | B,
	RGBA = R | G | B | A,
} VERA_ENUM_FLAGS(ColorComponentFlagBits, ColorComponentFlags)

VERA_VK_ABI_COMPATIBLE enum class PipelineStageFlagBits VERA_FLAG_BITS
{
	None                         = 0,
	TopOfPipe                    = 1 << 0,
	DrawIndirect                 = 1 << 1,
	VertexInput                  = 1 << 2,
	VertexShader                 = 1 << 3,
	TessellationControlShader    = 1 << 4,
	TessellationEvaluationShader = 1 << 5,
	GeometryShader               = 1 << 6,
	FragmentShader               = 1 << 7,
	EarlyFragmentTests           = 1 << 8,
	LateFragmentTests            = 1 << 9,
	ColorAttachmentOutput        = 1 << 10,
	ComputeShader                = 1 << 11,
	Transfer                     = 1 << 12,
	BottomOfPipe                 = 1 << 13,
	Host                         = 1 << 14,
	AllGraphics                  = 1 << 15
} VERA_ENUM_FLAGS(PipelineStageFlagBits, PipelineStageFlags)

VERA_VK_ABI_COMPATIBLE enum class DynamicState VERA_ENUM
{
	LineWidth                           = 2,
	DepthBias                           = 3,
	BlendConstants                      = 4,
	DepthBounds                         = 5,
	StencilCompareMask                  = 6,
	StencilWriteMask                    = 7,
	StencilReference                    = 8,
	CullMode                            = 1000267000,
	FrontFace                           = 1000267001,
	PrimitiveTopology                   = 1000267002,
	ViewportWithCount                   = 1000267003,
	ScissorWithCount                    = 1000267004,
	VertexInputBindingStride            = 1000267005,
	DepthTestEnable                     = 1000267006,
	DepthWriteEnable                    = 1000267007,
	DepthCompareOp                      = 1000267008,
	DepthBoundsTestEnable               = 1000267009,
	StencilTestEnable                   = 1000267010,
	StencilOp                           = 1000267011,
	RasterizerDiscardEnable             = 1000377001,
	DepthBiasEnable                     = 1000377002,
	PrimitiveRestartEnable              = 1000377004,
	PatchControlPointsExt               = 1000377000,
	LogicOpExt                          = 1000377003,
	LineStipple                         = 1000259000,
	ViewportWScalingNV                  = 1000087000,
	ViewportWSCalingEnableNV            = 1000455023,
	ViewportSwizzleNV                   = 1000455024,
	DiscardRectangleExt                 = 1000099000,
	DiscardRectangleEnableExt           = 1000099001,
	DiscardRectangleModeExt             = 1000099002,
	SampleLocationsExt                  = 1000143000,
	SampleLocationsEnableExt            = 1000455017,
	RayTracingPipelineStackSizeKHR      = 1000347000,
	ViewportShadingRatePaletteNV        = 1000164004,
	ViewportCoarseSampleOrderNV         = 1000164006,
	FragmentShadingRateKHR              = 1000226000,
	ExclusiveScissorEnableNV            = 1000205000,
	ExclusiveScissorNV                  = 1000205001,
	VertexInputExt                      = 1000352000,
	ColorWriteEnableExt                 = 1000381000,
	DepthClampEnableExt                 = 1000455003,
	PolygonModeExt                      = 1000455004,
	RasterizationSamplesExt             = 1000455005,
	SampleMaskExt                       = 1000455006,
	AlphaToCoverageEnableExt            = 1000455007,
	AlphaToOneEnableExt                 = 1000455008,
	LogicOpEnableExt                    = 1000455009,
	ColorBlendEnableExt                 = 1000455010,
	ColorBlendEquationExt               = 1000455011,
	ColorWriteMaskExt                   = 1000455012,
	TessellationDomainOriginExt         = 1000455002,
	RasterizationStreamExt              = 1000455013,
	ConservativeRasterizationModeExt    = 1000455014,
	ExtraPrimitiveOverestimationSizeExt = 1000455015,
	DepthClipEnableExt                  = 1000455016,
	ColorBlendAdvancedExt               = 1000455018,
	ProvokingVertexModeExt              = 1000455019,
	LineRasterizationModeExt            = 1000455020,
	LineStippleEnableExt                = 1000455021,
	DepthClipNegativeOneToOneExt        = 1000455022,
	CoverageToColorEnableNV             = 1000455025,
	CoverageToColorLocationNV           = 1000455026,
	CoverageModulationModeNV            = 1000455027,
	CoverageModulationTableEnableNV     = 1000455028,
	CoverageModulationTableNV           = 1000455029,
	ShadingRateImageEnableNV            = 1000455030,
	RepresentativeFragmentTestEnableNV  = 1000455031,
	CoverageReductionModeNV             = 1000455032,
	AttachmentFeedbackLoopEnableExt     = 1000524000,
	DepthClampRangeExt                  = 1000582000,
};

VERA_VK_ABI_COMPATIBLE enum class ShaderStageFlagBits VERA_FLAG_BITS
{
	None                   = 0,
	Vertex                 = 1 << 0,
	TessellationControl    = 1 << 1,
	TessellationEvaluation = 1 << 2,
	Geometry               = 1 << 3,
	Fragment               = 1 << 4,
	Compute                = 1 << 5,
	Task                   = 1 << 6,
	Mesh                   = 1 << 7,
	RayGen                 = 1 << 8,
	AnyHit                 = 1 << 9,
	ClosestHit             = 1 << 10,
	Miss                   = 1 << 11,
	Intersection           = 1 << 12,
	Callable               = 1 << 13,
} VERA_ENUM_FLAGS(ShaderStageFlagBits, ShaderStageFlags)

enum class ReflectionPrimitiveType VERA_ENUM
{
	Unknown,

	// Scalar types
	Bool,
	Char,
	UChar,
	Short,
	Int,
	UInt,
	Long,
	ULong,
	Float,
	Double,

	// Vector types
	Bool2,
	Bool3,
	Bool4,

	Char2,
	Char3,
	Char4,

	UChar2,
	UChar3,
	UChar4,

	Short2,
	Short3,
	Short4,

	UShort,
	UShort2,
	UShort3,
	UShort4,

	Int2,
	Int3,
	Int4,

	UInt2,
	UInt3,
	UInt4,

	Long2,
	Long3,
	Long4,

	ULong2,
	ULong3,
	ULong4,

	Float2,
	Float3,
	Float4,

	Double2,
	Double3,
	Double4,

	// Matrix types
	Float2x2,
	Float2x3,
	Float2x4,
	Float3x2,
	Float3x3,
	Float3x4,
	Float4x2,
	Float4x3,
	Float4x4,

	Double2x2,
	Double2x3,
	Double2x4,
	Double3x2,
	Double3x3,
	Double3x4,
	Double4x2,
	Double4x3,
	Double4x4
};

VERA_VK_ABI_COMPATIBLE enum class DescriptorSetLayoutCreateFlagBits VERA_FLAG_BITS
{
	None                = 0,
	PushDescriptor      = 1 << 0,
	UpdateAfterBindPool = 1 << 1
} VERA_ENUM_FLAGS(DescriptorSetLayoutCreateFlagBits, DescriptorSetLayoutCreateFlags)

VERA_VK_ABI_COMPATIBLE enum class DescriptorSetLayoutBindingFlagBits VERA_FLAG_BITS
{
	None                     = 0,
	UpdateAfterBind          = 1 << 0,
	UpdateUnusedWhilePending = 1 << 1,
	PartiallyBound           = 1 << 2,
	VariableDescriptorCount  = 1 << 3
} VERA_ENUM_FLAGS(DescriptorSetLayoutBindingFlagBits, DescriptorSetLayoutBindingFlags)

enum class DescriptorType VERA_ENUM
{
	Unknown                            = 0,
	Sampler                            = 1,
	CombinedTextureSampler             = 2,
	SampledTexture                     = 3,
	StorageTexture                     = 4,
	UniformTexelBuffer                 = 5,
	StorageTexelBuffer                 = 6,
	UniformBuffer                      = 7,
	StorageBuffer                      = 8,
	UniformBufferDynamic               = 9,
	StorageBufferDynamic               = 10,
	InputAttachment                    = 11,
	inlineUniformBlock                 = 1000138001,
	AccelerationStructure              = 1000150001,
	AccelerationStructureNV            = 1000165001,
	PartitionedAccelerationStructureNV = 1000570001
};

VERA_VK_ABI_COMPATIBLE enum class DescriptorPoolCreateFlagBits VERA_FLAG_BITS
{
	None              = 0,
	FreeDescriptorSet = 1 << 0,
	UpdateAfterBind   = 1 << 1
} VERA_ENUM_FLAGS(DescriptorPoolCreateFlagBits, DescriptorPoolCreateFlags)

VERA_VK_ABI_COMPATIBLE enum class PresentMode VERA_ENUM
{
	Immediate = 0,
	Mailbox   = 1,
	Fifo      = 2
};

VERA_VK_ABI_COMPATIBLE enum class SamplerFilter VERA_ENUM
{
	Nearest = 0,
	Linear  = 1
};

VERA_VK_ABI_COMPATIBLE enum class SamplerMipmapMode VERA_ENUM
{
	Nearest = 0,
	Linear  = 1
};

VERA_VK_ABI_COMPATIBLE enum class SamplerAddressMode VERA_ENUM
{
	Repeat            = 0,
	MirroredRepeat    = 1,
	ClampToEdge       = 2,
	ClampToBorder     = 3,
	MirrorClampToEdge = 4 
};

VERA_VK_ABI_COMPATIBLE enum class SamplerCompareOp VERA_ENUM
{
	Never          = 0,
	Less           = 1,
	Equal          = 2,
	LessOrEqual    = 3,
	Greater        = 4,
	NotEqual       = 5,
	GreaterOrEqual = 6,
	Always         = 7
};

VERA_VK_ABI_COMPATIBLE enum class ResolveModeFlagBits VERA_ENUM
{
	None       = 0,
	SampleZero = 1 << 0,
	Average    = 1 << 1,
	Min        = 1 << 2,
	Max        = 1 << 3
} VERA_ENUM_FLAGS(ResolveModeFlagBits, ResolveModeFlags);

VERA_VK_ABI_COMPATIBLE enum class LoadOp VERA_ENUM
{
	Load     = 0,
	Clear    = 1,
	DontCare = 2
};

VERA_VK_ABI_COMPATIBLE enum class StoreOp VERA_ENUM
{
	Store    = 0,
	DontCare = 1
};

VERA_VK_ABI_COMPATIBLE enum class AccessFlagBits VERA_FLAG_BITS
{
	None                        = 0,
	IndirectCommandRead         = 1 << 0,
	IndexRead                   = 1 << 1,
	VertexAttributeRead         = 1 << 2,
	UniformRead                 = 1 << 3,
	InputAttachmentRead         = 1 << 4,
	ShaderRead                  = 1 << 5,
	ShaderWrite                 = 1 << 6,
	ColorAttachmentRead         = 1 << 7,
	ColorAttachmentWrite        = 1 << 8,
	DepthStencilAttachmentRead  = 1 << 9,
	DepthStencilAttachmentWrite = 1 << 10,
	TransferRead                = 1 << 11,
	TransferWrite               = 1 << 12,
	HostRead                    = 1 << 13,
	HostWrite                   = 1 << 14,
	MemoryRead                  = 1 << 15,
	MemoryWrite                 = 1 << 16,
} VERA_ENUM_FLAGS(AccessFlagBits, AccessFlags)

VERA_VK_ABI_COMPATIBLE enum class BufferUsageFlagBits VERA_FLAG_BITS
{
	TransferSrc        = 1 << 0,
	TransferDst        = 1 << 1,
	UniformTexelBuffer = 1 << 2,
	StorageTexelBuffer = 1 << 3,
	UniformBuffer      = 1 << 4,
	StorageBuffer      = 1 << 5,
	IndexBuffer        = 1 << 6,
	VertexBuffer       = 1 << 7,
	IndirectBuffer     = 1 << 8
} VERA_ENUM_FLAGS(BufferUsageFlagBits, BufferUsageFlags)

enum class TextureType VERA_ENUM
{
	Texture1D,
	Texture2D,
	Texture3D,
	TextureCube,
	Texture2DMultisample,
};

VERA_VK_ABI_COMPATIBLE enum class TextureLayout VERA_ENUM
{
	Undefined                             = 0,
	General                               = 1,
	ColorAttachmentOptimal                = 2,
	DepthStencilAttachmentOptimal         = 3,
	DepthStencilReadOnlyOptimal           = 4,
	ShaderReadOnlyOptimal                 = 5,
	TransferSrcOptimal                    = 6,
	TransferDstOptimal                    = 7,
	Preinitialized                        = 8,
	DepthReadOnlyStencilAttachmentOptimal = 1000117000,
	DepthAttachmentStencilReadOnlyOptimal = 1000117001,
	DepthAttachmentOptimal                = 1000241000,
	DepthReadOnlyOptimal                  = 1000241001,
	StencilAttachmentOptimal              = 1000241002,
	StencilReadOnlyOptimal                = 1000241003,
	ReadOnlyOptimal                       = 1000314000,
	AttachmentOptimal                     = 1000314001,
	RenderingLocalRead                    = 1000232000,
	PresentSrc                            = 1000001002
};

VERA_VK_ABI_COMPATIBLE enum class TextureAspectFlagBits VERA_FLAG_BITS
{
	Color    = 1 << 0,
	Depth    = 1 << 1,
	Stencil  = 1 << 2,
	Metadata = 1 << 3
} VERA_ENUM_FLAGS(TextureAspectFlagBits, TextureAspectFlags)

enum class TextureUsageFlagBits VERA_FLAG_BITS
{
	TransferSrc            = 1 << 0,
	TransferDst            = 1 << 1,
	Sampled                = 1 << 2,
	Storage                = 1 << 3,
	ColorAttachment        = 1 << 4,
	DepthStencilAttachment = 1 << 5,
	TransientAttachment    = 1 << 6,
	InputAttachment        = 1 << 7,
	HostTransfer           = 1 << 8,
	FrameBuffer            = 1 << 9
} VERA_ENUM_FLAGS(TextureUsageFlagBits, TextureUsageFlags)

VERA_VK_ABI_COMPATIBLE enum class TextureViewType VERA_ENUM
{
	View1D        = 0,
	View2D        = 1,
	View3D        = 2,
	ViewCube      = 3,
	View1DArray   = 4,
	View2DArray   = 5,
	ViewCubeArray = 6
};

VERA_VK_ABI_COMPATIBLE enum class ComponentSwizzle VERA_ENUM
{
	Identity = 0,
	One      = 1,
	Zero     = 2,
	R        = 3,
	G        = 4,
	B        = 5,
	A        = 6
};

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
	Unknown        = Format::Unknown,
	D16Unorm       = Format::D16Unorm,
	X8D24Unorm     = Format::X8D24Unorm,
	D32Float       = Format::D32Float,
	D16UnormS8Uint = Format::D16UnormS8Uint,
	D24UnormS8Uint = Format::D24UnormS8Uint,
	D32FloatS8Uint = Format::D32FloatS8Uint
};

enum class StencilFormat
{
	Unknown        = Format::Unknown,
	S8Uint         = Format::S8Uint,
	D16UnormS8Uint = Format::D16UnormS8Uint,
	D24UnormS8Uint = Format::D24UnormS8Uint,
	D32FloatS8Uint = Format::D32FloatS8Uint
};

enum class AlphaFormat
{
	A8Unorm = Format::A8Unorm
};

enum class IndexType VERA_ENUM
{
	Unknown,
	UInt8,
	UInt16,
	UInt32
};

VERA_NAMESPACE_END