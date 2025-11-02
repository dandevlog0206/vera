#pragma once

#include "shader.h"
#include "../graphics/vertex_input.h"
#include <optional>
#include <vector>
#include <array>

VERA_NAMESPACE_BEGIN

class PipelineLayout;

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
	R    = 1 << 0,
	G    = 1 << 1,
	B    = 1 << 2,
	A    = 1 << 3,
	RGB  = R | G | B,
	RGBA = R | G | B | A,
} VERA_ENUM_FLAGS(ColorComponentFlagBits, ColorComponentFlags)

VERA_VK_ABI_COMPATIBLE enum class PipelineStageFlagBits VERA_FLAG_BITS
{
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

VERA_VK_ABI_COMPATIBLE struct StencilOpState
{
	StencilOp failOp      = StencilOp::Keep;
	StencilOp passOp      = StencilOp::Keep;
	StencilOp depthFailOp = StencilOp::Keep;
	CompareOp compareOp   = CompareOp::Never;
	uint32_t  compareMask = 0;
	uint32_t  writeMask   = 0;
	uint32_t  reference   = 0;
};

struct VertexInputInfo
{
	VertexInputDescriptor vertexInputDescriptor;
	VertexInputDescriptor instanceInputDescriptor;
};

struct PrimitiveInfo
{
	bool              enableRestart;
	PrimitiveTopology topology;
};

struct RasterizationInfo
{
	bool          depthClampEnable        = false;
	bool          rasterizerDiscardEnable = false;
	PolygonMode   polygonMode             = PolygonMode::Fill;
	CullModeFlags cullMode                = CullModeFlagBits::None;
	FrontFace     frontFace               = FrontFace::CounterClockwise;
	bool          depthBiasEnable         = false;
	float         depthBiasConstantFactor = 0.f;
	float         depthBiasClamp          = 0.f;
	float         depthBiasSlopeFactor    = 0.f;
	float         lineWidth               = 1.f;
};

struct DepthStencilInfo
{
	DepthFormat    depthFormat           = DepthFormat::Unknown;
	bool           depthWriteEnable      = false;
	CompareOp      depthCompareOp        = CompareOp::Never;
	bool           depthBoundsTestEnable = false;
	StencilFormat  stencilFormat         = StencilFormat::Unknown;
	StencilOpState front                 = {};
	StencilOpState back                  = {};
	float          minDepthBounds        = 0.f;
	float          maxDepthBounds        = 1.f;
};

VERA_VK_ABI_COMPATIBLE struct ColorBlendAttachmentState
{
	bool                blendEnable         = false;
	BlendFactor         srcColorBlendFactor = BlendFactor::SrcAlpha;
	BlendFactor         dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
	BlendOp             colorBlendOp        = BlendOp::Add;
	BlendFactor         srcAlphaBlendFactor = BlendFactor::SrcAlpha;
	BlendFactor         dstAlphaBlendFactor = BlendFactor::OneMinusSrcAlpha;
	BlendOp             alphaBlendOp        = BlendOp::Add;
	ColorComponentFlags colorWriteMask      = ColorComponentFlagBits::RGBA;
};

struct ColorBlendInfo
{
	bool                                   enableLogicOp  = false;
	LogicOp                                logicOp        = LogicOp::Copy;
	std::vector<ColorBlendAttachmentState> attachments    = {};
	std::array<float, 4>                   blendConstants = {{ 0.f, 0.f, 0.f, 0.f }};
};

struct PipelineAttachmentInfo
{
	std::vector<ColorBlendAttachmentState> blendState;
};

struct GraphicsPipelineCreateInfo
{
	obj<Shader>                      vertexShader;
	obj<Shader>                      tessellationControlShader;
	obj<Shader>                      tessellationEvaluationShader;
	obj<Shader>                      geometryShader;
	obj<Shader>                      fragmentShader;

	std::optional<VertexInputInfo>   vertexInputInfo;
	std::optional<PrimitiveInfo>     primitiveInfo;
	std::optional<RasterizationInfo> rasterizationInfo;
	std::optional<uint32_t>          tessellationPatchControlPoints;
	std::optional<DepthStencilInfo>  depthStencilInfo;
	std::optional<ColorBlendInfo>    colorBlendInfo;
	std::vector<Format>              colorAttachmentFormats;
	std::vector<DynamicState>        dynamicStates;
};

struct MeshPipelineCreateInfo
{
	obj<Shader>                      taskShader;
	obj<Shader>                      meshShader;
	obj<Shader>                      fragmentShader;

	std::optional<RasterizationInfo> rasterizationInfo;
	std::optional<DepthStencilInfo>  depthStencilInfo;
	std::optional<ColorBlendInfo>    colorBlendInfo;
	std::vector<Format>              colorAttachmentFormats;
	std::vector<DynamicState>        dynamicStates;
};

struct ComputePipelineCreateInfo
{
	obj<Shader> computeShader;
};

class Pipeline : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Pipeline)
public:
	static obj<Pipeline> create(obj<Device> device, const GraphicsPipelineCreateInfo& info);
	static obj<Pipeline> create(obj<Device> device, const MeshPipelineCreateInfo& info);
	static obj<Pipeline> create(obj<Device> device, const ComputePipelineCreateInfo& info);
	~Pipeline();

	obj<Device> getDevice();
	obj<PipelineLayout> getPipelineLayout();
	std::vector<obj<Shader>> enumerateShaders();
	obj<Shader> getShader(ShaderStageFlagBits stage);
};

VERA_NAMESPACE_END