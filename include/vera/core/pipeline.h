#pragma once

#include "shader.h"
#include "../graphics/vertex_input.h"
#include <vector>
#include <optional>

VERA_NAMESPACE_BEGIN

class PipelineLayout;

enum class PrimitiveTopology VERA_ENUM
{
	PointList,
	LineList,
	LineStrip,
	TriangleList,
	TriangleStrip,
	TriangleFan,
	LineListWithAdjacency,
	LineStripWithAdjacency,
	TriangleListWithAdjacency,
	TriangleStripWithAdjacency,
	PatchList
};

enum class PolygonMode VERA_ENUM
{
	Fill,
	Line,
	Point,
	FillRectangleNV
};

enum class CullMode VERA_ENUM
{
	None,
	Front,
	Back,
	FrontAndBack
};

enum class FrontFace VERA_ENUM
{
	CounterClockwise,
	Clockwise
};

enum class CompareOp VERA_ENUM
{
	Never,
	Less,
	Equal,
	LessOrEqual,
	Greater,
	NotEqual,
	GreaterOrEqual,
	Always
};

enum class StencilOp VERA_ENUM
{
	Keep,
	Zero,
	Replace,
	IncrementAndClamp,
	DecrementAndClamp,
	Invert,
	IncrementAndWrap,
	DecrementAndWrap
};

enum class LogicOp VERA_ENUM
{
	Clear,
	And,
	AndReverse,
	Copy,
	AndInverted,
	NoOp,
	Xor,
	Or,
	Nor,
	Equivalent,
	Invert,
	OrReverse,
	CopyInverted,
	OrInverted,
	Nand,
	Set
};

enum class PipelineStageFlagBits VERA_FLAG_BITS
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

struct StencilOpState
{
	StencilOp failOp       = StencilOp::Keep;
	StencilOp passOp       = StencilOp::Keep;
	StencilOp depthFailOp  = StencilOp::Keep;
	CompareOp compareOp    = CompareOp::Never;
	uint32_t  compareMask  = 0;
	uint32_t  writeMask    = 0;
	uint32_t  reference    = 0;
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
	bool        depthClampEnable        = false;
	bool        rasterizerDiscardEnable = false;
	PolygonMode polygonMode             = PolygonMode::Fill;
	CullMode    cullMode                = CullMode::None;
	FrontFace   frontFace               = FrontFace::CounterClockwise;
	bool        depthBiasEnable         = false;
	float       depthBiasConstantFactor = 0.f;
	float       depthBiasClamp          = 0.f;
	float       depthBiasSlopeFactor    = 0.f;
	float       lineWidth               = 1.f;
};

struct DepthStencilInfo
{
	bool           depthTestEnable       = false;
	bool           depthWriteEnable      = false;
	CompareOp      depthCompareOp        = CompareOp::Never;
	bool           depthBoundsTestEnable = false;
	bool           stencilTestEnable     = false;
	StencilOpState front                 = {};
	StencilOpState back                  = {};
	float          minDepthBounds        = 0.f;
	float          maxDepthBounds        = 0.f;
};

struct ColorBlendInfo
{
	bool     enableLogicOp;
	LogicOp  logicOp;
	uint32_t attachmentCount;


/*	VULKAN_HPP_NAMESPACE::Bool32                             logicOpEnable_ = {},
	VULKAN_HPP_NAMESPACE::LogicOp logicOp_ = VULKAN_HPP_NAMESPACE::LogicOp::eClear,
	uint32_t                      attachmentCount_ = {},
	const VULKAN_HPP_NAMESPACE::PipelineColorBlendAttachmentState* pAttachments_ = {},
	std::array<float, 4> const& blendConstants_ = {}
*/
};

struct GraphicsPipelineCreateInfo
{
	ref<Shader>                      vertexShader;
	ref<Shader>                      geometryShader;
	ref<Shader>                      fragmentShader;

	std::optional<VertexInputInfo>   vertexInputInfo;
	std::optional<PrimitiveInfo>     primitiveInfo;
	std::optional<RasterizationInfo> rasterizationInfo;
	std::optional<uint32_t>          tesselationPatchControlPoints;
	std::optional<DepthStencilInfo>  depthStencilInfo;
	std::optional<ColorBlendInfo>    colorBlendInfo;
};

struct ComputePipelineCreateInfo
{

};

class Pipeline : protected CoreObject
{
	VERA_CORE_OBJECT(Pipeline)
public:
	static ref<Pipeline> create(ref<Device> device, const GraphicsPipelineCreateInfo& info);
	~Pipeline();

	ref<PipelineLayout> getPipelineLayout();

	std::vector<ref<Shader>> enumerateShaders();
	ref<Shader> getShader(ShaderStageFlagBits stage);
};

VERA_NAMESPACE_END