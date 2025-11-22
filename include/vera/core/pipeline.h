#pragma once

#include "pipeline_layout.h"
#include "../graphics/vertex_input.h"
#include "../util/bool32.h"
#include <optional>
#include <vector>
#include <array>

VERA_NAMESPACE_BEGIN

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
	bool              enableRestart = false;
	PrimitiveTopology topology      = PrimitiveTopology::TriangleList;
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
	bool32              blendEnable         = false;
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
	obj<PipelineLayout>              pipelineLayout; // optional

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
	obj<PipelineLayout>              pipelineLayout; // optional

	std::optional<RasterizationInfo> rasterizationInfo;
	std::optional<DepthStencilInfo>  depthStencilInfo;
	std::optional<ColorBlendInfo>    colorBlendInfo;
	std::vector<Format>              colorAttachmentFormats;
	std::vector<DynamicState>        dynamicStates;
};

struct ComputePipelineCreateInfo
{
	obj<Shader>         computeShader;
	obj<PipelineLayout> pipelineLayout; // optional
};

class Pipeline : protected CoreObject
{
	VERA_CORE_OBJECT_INIT(Pipeline)
public:
	static obj<Pipeline> create(obj<Device> device, const GraphicsPipelineCreateInfo& info);
	static obj<Pipeline> create(obj<Device> device, const MeshPipelineCreateInfo& info);
	static obj<Pipeline> create(obj<Device> device, const ComputePipelineCreateInfo& info);
	~Pipeline() VERA_NOEXCEPT override;

	obj<Device> getDevice();
	obj<PipelineLayout> getPipelineLayout();
	std::vector<obj<Shader>> enumerateShaders();
	obj<Shader> getShader(ShaderStageFlagBits stage);
};

VERA_NAMESPACE_END