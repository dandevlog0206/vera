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
	bool                blendEnable         = true;
	BlendFactor         srcColorBlendFactor = BlendFactor::SrcColor;
	BlendFactor         dstColorBlendFactor = BlendFactor::OneMinusSrcColor;
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
};

struct ComputePipelineCreateInfo
{

};

class Pipeline : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Pipeline)
public:
	static obj<Pipeline> create(obj<Device> device, const GraphicsPipelineCreateInfo& info);
	~Pipeline();

	obj<Device> getDevice();
	obj<PipelineLayout> getPipelineLayout();
	std::vector<obj<Shader>> enumerateShaders();
	obj<Shader> getShader(ShaderStageFlagBits stage);
};

VERA_NAMESPACE_END