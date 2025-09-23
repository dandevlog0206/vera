#include "../../include/vera/core/pipeline.h"
#include "../impl/device_impl.h"
#include "../impl/pipeline_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/resource_layout.h"
#include "../../include/vera/core/shader.h"
#include "../../include/vera/util/static_vector.h"
#include "../../include/vera/util/hash.h"

VERA_NAMESPACE_BEGIN

static void append_shader_layout_info(PipelineLayoutCreateInfo& layout_info, ref<Shader> shader)
{
	if (!shader) return;

	auto& shader_impl = CoreObject::getImpl(shader);

	for (const auto& layout : shader_impl.resourceLayouts)
		layout_info.resourceLayouts.push_back(layout);
	for (const auto& pc : shader_impl.pushConstantRanges)
		layout_info.pushConstantRanges.push_back(pc);
}

static ref<PipelineLayout> register_pipeline_layout(ref<Device> device, const GraphicsPipelineCreateInfo& info)
{
	PipelineLayoutCreateInfo layout_info;

	append_shader_layout_info(layout_info, info.vertexShader);
	append_shader_layout_info(layout_info, info.geometryShader);
	append_shader_layout_info(layout_info, info.fragmentShader);

	return PipelineLayout::create(device, layout_info);
}

static void fill_shader_info(PipelineImpl& impl, static_vector<vk::PipelineShaderStageCreateInfo, 10>& shader_infos, const GraphicsPipelineCreateInfo& info)
{
	vk::PipelineShaderStageCreateInfo shader_info;

	if (info.vertexShader) {
		auto& shader_impl = CoreObject::getImpl(info.vertexShader);

		shader_info.stage               = vk::ShaderStageFlagBits::eVertex;
		shader_info.module              = shader_impl.shader;
		shader_info.pName               = shader_impl.entryPointName.c_str();
		shader_info.pSpecializationInfo = nullptr;

		shader_infos.push_back(shader_info);
		impl.shaders.push_back(std::make_pair(ShaderStageFlagBits::Vertex, info.vertexShader));
	}

	if (info.geometryShader) {
		auto& shader_impl = CoreObject::getImpl(info.geometryShader);

		shader_info.stage               = vk::ShaderStageFlagBits::eGeometry;
		shader_info.module              = shader_impl.shader;
		shader_info.pName               = shader_impl.entryPointName.c_str();
		shader_info.pSpecializationInfo = nullptr;

		shader_infos.push_back(shader_info);
		impl.shaders.push_back(std::make_pair(ShaderStageFlagBits::Geometry, info.geometryShader));
	}

	if (info.fragmentShader) {
		auto& shader_impl = CoreObject::getImpl(info.fragmentShader);

		shader_info.stage               = vk::ShaderStageFlagBits::eFragment;
		shader_info.module              = shader_impl.shader;
		shader_info.pName               = shader_impl.entryPointName.c_str();
		shader_info.pSpecializationInfo = nullptr;

		shader_infos.push_back(shader_info);
		impl.shaders.push_back(std::make_pair(ShaderStageFlagBits::Fragment, info.fragmentShader));
	}
}

static void fill_dynamic_states(static_vector<vk::DynamicState, 64>& states, const GraphicsPipelineCreateInfo& info)
{
	 states = {
		// viewport
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,

		// color blend
		//vk::DynamicState::eLogicOpEnableEXT,
		//vk::DynamicState::eLogicOpEXT,
		//vk::DynamicState::eColorBlendEnableEXT,
		//vk::DynamicState::eColorBlendEquationEXT,
		//vk::DynamicState::eColorWriteMaskEXT,
		//vk::DynamicState::eBlendConstants
	};

	if (!info.primitiveInfo) {
		states.push_back(vk::DynamicState::ePrimitiveRestartEnable);
		states.push_back(vk::DynamicState::ePrimitiveTopology);
	}

	if (!info.rasterizationInfo) {
		states.push_back(vk::DynamicState::eDepthClampEnableEXT);
		states.push_back(vk::DynamicState::eRasterizerDiscardEnable);
		states.push_back(vk::DynamicState::ePolygonModeEXT);
		states.push_back(vk::DynamicState::eCullMode);
		states.push_back(vk::DynamicState::eFrontFace);
		states.push_back(vk::DynamicState::eDepthBiasEnable);
		states.push_back(vk::DynamicState::eDepthBias);
		states.push_back(vk::DynamicState::eLineWidth);
	}

	if (!info.tesselationPatchControlPoints)
		states.push_back(vk::DynamicState::ePatchControlPointsEXT);

	if (!info.depthStencilInfo) {
		states.push_back(vk::DynamicState::eLogicOpEnableEXT);
		states.push_back(vk::DynamicState::eLogicOpEXT);
		states.push_back(vk::DynamicState::eColorBlendEnableEXT);
		states.push_back(vk::DynamicState::eColorBlendEquationEXT);
		states.push_back(vk::DynamicState::eColorWriteMaskEXT);
		states.push_back(vk::DynamicState::eBlendConstants);
	}
}

static size_t hash_graphics_pipeline(const GraphicsPipelineCreateInfo& info)
{
	size_t seed = 0;

	hash_combine(seed, info.vertexShader->hash());
	hash_combine(seed, info.geometryShader ? info.geometryShader->hash() : 0);
	hash_combine(seed, info.fragmentShader->hash());

	return seed;
}

ref<Pipeline> Pipeline::create(ref<Device> device, const GraphicsPipelineCreateInfo& info)
{
	VERA_ASSERT_MSG(info.vertexShader && info.fragmentShader,
		"vertex shader and fragment shader are required");

	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_graphics_pipeline(info);

	if (auto it = device_impl.pipelineMap.find(hash_value);
		it != device_impl.pipelineMap.end()) {
		return it->second;
	}

	auto  obj  = createNewObject<Pipeline>();
	auto& impl = getImpl(obj);

	impl.pipelineLayout = register_pipeline_layout(device, info);

	static_vector<vk::PipelineShaderStageCreateInfo, 10> shader_infos;
	fill_shader_info(impl, shader_infos, info);

	vk::PipelineVertexInputStateCreateInfo vi_info;

	vk::PipelineInputAssemblyStateCreateInfo ia_info;
	if (info.primitiveInfo) {
		ia_info.primitiveRestartEnable = info.primitiveInfo->enableRestart;
		ia_info.topology               = to_vk_primitive_topology(info.primitiveInfo->topology);
	}

	vk::PipelineTessellationStateCreateInfo ts_info;
	if (info.tesselationPatchControlPoints)
		ts_info.patchControlPoints = *info.tesselationPatchControlPoints;

	vk::Viewport temp_viewport;
	temp_viewport.width  = 1080.f;
	temp_viewport.height = 720.f;
	
	vk::Rect2D temp_scissor;
	temp_scissor.extent = vk::Extent2D{ 1080, 720 };

	vk::PipelineViewportStateCreateInfo vp_info;
	vp_info.viewportCount = 1;
	vp_info.pViewports    = &temp_viewport;
	vp_info.scissorCount  = 1;
	vp_info.pScissors     = &temp_scissor;

	vk::PipelineRasterizationStateCreateInfo rs_info;
	if (info.rasterizationInfo) {
		rs_info.depthClampEnable        = info.rasterizationInfo->depthClampEnable;
		rs_info.rasterizerDiscardEnable = info.rasterizationInfo->rasterizerDiscardEnable;
		rs_info.polygonMode             = to_vk_polygon_mode(info.rasterizationInfo->polygonMode);
		rs_info.cullMode                = to_vk_cull_mode(info.rasterizationInfo->cullMode);
		rs_info.frontFace               = to_vk_front_face(info.rasterizationInfo->frontFace);
		rs_info.depthBiasEnable         = info.rasterizationInfo->depthBiasEnable;
		rs_info.depthBiasConstantFactor = info.rasterizationInfo->depthBiasConstantFactor;
		rs_info.depthBiasClamp          = info.rasterizationInfo->depthBiasClamp;
		rs_info.depthBiasSlopeFactor    = info.rasterizationInfo->depthBiasConstantFactor;
		rs_info.lineWidth               = info.rasterizationInfo->lineWidth;
	}

	vk::PipelineMultisampleStateCreateInfo ms_info;
	ms_info.rasterizationSamples  = vk::SampleCountFlagBits::e1;
	ms_info.sampleShadingEnable   = false;
	ms_info.minSampleShading      = 1.f;
	ms_info.pSampleMask           = nullptr;
	ms_info.alphaToCoverageEnable = false;
	ms_info.alphaToOneEnable      = false;

	vk::PipelineDepthStencilStateCreateInfo ds_info;
	if (info.depthStencilInfo) {
		ds_info.depthTestEnable       = info.depthStencilInfo->depthTestEnable;
		ds_info.depthWriteEnable      = info.depthStencilInfo->depthWriteEnable;
		ds_info.depthCompareOp        = to_vk_compare_op(info.depthStencilInfo->depthCompareOp);
		ds_info.depthBoundsTestEnable = info.depthStencilInfo->depthBoundsTestEnable;
		ds_info.stencilTestEnable     = info.depthStencilInfo->stencilTestEnable;
		ds_info.front                 = to_vk_stencil_op_state(info.depthStencilInfo->front);
		ds_info.back                  = to_vk_stencil_op_state(info.depthStencilInfo->back);
		ds_info.minDepthBounds        = info.depthStencilInfo->minDepthBounds;
		ds_info.maxDepthBounds        = info.depthStencilInfo->maxDepthBounds;
	}

	vk::PipelineColorBlendAttachmentState blend_attachemnt;
	blend_attachemnt.blendEnable         = false;
	blend_attachemnt.srcColorBlendFactor = vk::BlendFactor::eOne;
	blend_attachemnt.dstColorBlendFactor = vk::BlendFactor::eZero;
	blend_attachemnt.colorBlendOp        = vk::BlendOp::eAdd;
	blend_attachemnt.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	blend_attachemnt.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	blend_attachemnt.alphaBlendOp        = vk::BlendOp::eAdd;
	blend_attachemnt.colorWriteMask      = 
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;

	vk::PipelineColorBlendStateCreateInfo cb_info;
	cb_info.logicOpEnable     = false;
	cb_info.logicOp           = vk::LogicOp::eCopy;
	cb_info.attachmentCount   = 1;
	cb_info.pAttachments      = &blend_attachemnt;
	cb_info.blendConstants[0] = 0.f;
	cb_info.blendConstants[1] = 0.f;
	cb_info.blendConstants[2] = 0.f;
	cb_info.blendConstants[3] = 0.f;

	static_vector<vk::DynamicState, 64> dynamic_states;
	fill_dynamic_states(dynamic_states, info);

	vk::PipelineDynamicStateCreateInfo dynamic_info;
	dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_info.pDynamicStates    = dynamic_states.data();

	auto color_format = to_vk_format(device_impl.colorFormat);
	vk::PipelineRenderingCreateInfoKHR rendering_info;
	rendering_info.colorAttachmentCount    = 1;
	rendering_info.pColorAttachmentFormats = &color_format;
	if (info.depthStencilInfo && info.depthStencilInfo->depthTestEnable)
		rendering_info.depthAttachmentFormat = to_vk_format(device_impl.depthFormat);
	if (info.depthStencilInfo && info.depthStencilInfo->stencilTestEnable)
		rendering_info.stencilAttachmentFormat = to_vk_format(device_impl.depthFormat);

	vk::GraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.stageCount          = static_cast<uint32_t>(shader_infos.size());
	pipeline_info.pStages             = shader_infos.data();
	pipeline_info.pVertexInputState   = &vi_info;
	pipeline_info.pInputAssemblyState = info.primitiveInfo ? &ia_info : nullptr;
	pipeline_info.pTessellationState  = info.tesselationPatchControlPoints ? &ts_info : nullptr;
	pipeline_info.pViewportState      = &vp_info;
	pipeline_info.pRasterizationState = info.rasterizationInfo ? &rs_info : nullptr;
	pipeline_info.pMultisampleState   = &ms_info;
	pipeline_info.pDepthStencilState  = info.depthStencilInfo ? &ds_info : nullptr;
	pipeline_info.pColorBlendState    = &cb_info;
	pipeline_info.pDynamicState       = &dynamic_info;
	pipeline_info.layout              = get_vk_pipeline_layout(impl.pipelineLayout);
	pipeline_info.renderPass          = nullptr;
	pipeline_info.subpass             = 0;
	pipeline_info.basePipelineHandle  = nullptr;
	pipeline_info.basePipelineIndex   = 0;
	pipeline_info.pNext               = &rendering_info;

	auto result = device_impl.device.createGraphicsPipeline(device_impl.pipelineCache, pipeline_info);

	if (result.result != vk::Result::eSuccess)
		throw Exception("failed to create graphics pipeline");

	impl.device            = std::move(device);
	impl.pipeline          = result.value;
	impl.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

	return obj;
}

Pipeline::~Pipeline()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.device.destroy(impl.pipeline);

	destroyObjectImpl(this);
}

ref<PipelineLayout> Pipeline::getPipelineLayout()
{
	return getImpl(this).pipelineLayout;
}

std::vector<ref<Shader>> Pipeline::enumerateShaders()
{
	std::vector<ref<Shader>> result;

	for (auto [flag, shader] : getImpl(this).shaders)
		result.push_back(std::move(shader));

	return result;
}

ref<Shader> Pipeline::getShader(ShaderStageFlagBits stage)
{
	auto& impl = getImpl(this);

	for (auto& [flag, shader] : impl.shaders)
		if (flag == stage) return shader;

	return ref<Shader>();
}

VERA_NAMESPACE_END