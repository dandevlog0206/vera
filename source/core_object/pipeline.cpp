#include "../../include/vera/core/pipeline.h"
#include "../impl/device_impl.h"
#include "../impl/pipeline_impl.h"
#include "../impl/shader_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/resource_layout.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/util/static_vector.h"

#define VERA_MAX_SHADER_COUNT 8

VERA_NAMESPACE_BEGIN

static obj<PipelineLayout> register_pipeline_layout(obj<Device> device, const GraphicsPipelineCreateInfo& info)
{
	static_vector<const_ref<Shader>, VERA_MAX_SHADER_COUNT> shaders;

	VERA_ASSERT_MSG(info.vertexShader, "vertex shader must be specified");
	VERA_ASSERT_MSG(info.fragmentShader, "fragment shader must be specified");
	
	shaders.push_back(info.vertexShader);
	
	if (info.tessellationControlShader)
		shaders.push_back(info.tessellationControlShader);
	if (info.tessellationEvaluationShader)
		shaders.push_back(info.tessellationEvaluationShader);
	if (info.geometryShader)
		shaders.push_back(info.geometryShader);

	shaders.push_back(info.fragmentShader);

	return PipelineLayout::create(device, shaders);
}

static void fill_shader_info(PipelineImpl& impl, static_vector<vk::PipelineShaderStageCreateInfo, 10>& shader_infos, const GraphicsPipelineCreateInfo& info)
{
	vk::PipelineShaderStageCreateInfo shader_info;

	if (info.vertexShader) {
		auto& shader_impl = CoreObject::getImpl(info.vertexShader);

		shader_info.stage               = vk::ShaderStageFlagBits::eVertex;
		shader_info.module              = shader_impl.shader;
		shader_info.pName               = shader_impl.entryPointName.data();
		shader_info.pSpecializationInfo = nullptr;

		shader_infos.push_back(shader_info);
		impl.shaders.push_back(std::make_pair(ShaderStageFlagBits::Vertex, info.vertexShader));
	}

	if (info.geometryShader) {
		auto& shader_impl = CoreObject::getImpl(info.geometryShader);

		shader_info.stage               = vk::ShaderStageFlagBits::eGeometry;
		shader_info.module              = shader_impl.shader;
		shader_info.pName               = shader_impl.entryPointName.data();
		shader_info.pSpecializationInfo = nullptr;

		shader_infos.push_back(shader_info);
		impl.shaders.push_back(std::make_pair(ShaderStageFlagBits::Geometry, info.geometryShader));
	}

	if (info.fragmentShader) {
		auto& shader_impl = CoreObject::getImpl(info.fragmentShader);

		shader_info.stage               = vk::ShaderStageFlagBits::eFragment;
		shader_info.module              = shader_impl.shader;
		shader_info.pName               = shader_impl.entryPointName.data();
		shader_info.pSpecializationInfo = nullptr;

		shader_infos.push_back(shader_info);
		impl.shaders.push_back(std::make_pair(ShaderStageFlagBits::Fragment, info.fragmentShader));
	}
}

static vk::Format get_vertex_format(VertexFormat format, uint32_t& attribute_count)
{
	switch (format) {
	case VertexFormat::Char:      attribute_count = 1; return vk::Format::eR8Sint;
	case VertexFormat::Char2:     attribute_count = 1; return vk::Format::eR8G8Sint;
	case VertexFormat::Char3:     attribute_count = 1; return vk::Format::eR8G8B8Sint;
	case VertexFormat::Char4:     attribute_count = 1; return vk::Format::eR8G8B8A8Sint;
	case VertexFormat::UChar:     attribute_count = 1; return vk::Format::eR8Uint;
	case VertexFormat::UChar2:    attribute_count = 1; return vk::Format::eR8G8Uint;
	case VertexFormat::UChar3:    attribute_count = 1; return vk::Format::eR8G8B8Uint;
	case VertexFormat::UChar4:    attribute_count = 1; return vk::Format::eR8G8B8A8Uint;
	case VertexFormat::Short:     attribute_count = 1; return vk::Format::eR16Sint;
	case VertexFormat::Short2:    attribute_count = 1; return vk::Format::eR16G16Sint;
	case VertexFormat::Short3:    attribute_count = 1; return vk::Format::eR16G16B16Sint;
	case VertexFormat::Short4:    attribute_count = 1; return vk::Format::eR16G16B16A16Sint;
	case VertexFormat::UShort:    attribute_count = 1; return vk::Format::eR16Uint;
	case VertexFormat::UShort2:   attribute_count = 1; return vk::Format::eR16G16Uint;
	case VertexFormat::UShort3:   attribute_count = 1; return vk::Format::eR16G16B16Uint;
	case VertexFormat::UShort4:   attribute_count = 1; return vk::Format::eR16G16B16A16Uint;
	case VertexFormat::Int:       attribute_count = 1; return vk::Format::eR32Sint;
	case VertexFormat::Int2:      attribute_count = 1; return vk::Format::eR32G32Sint;
	case VertexFormat::Int3:      attribute_count = 1; return vk::Format::eR32G32B32Sint;
	case VertexFormat::Int4:      attribute_count = 1; return vk::Format::eR32G32B32A32Sint;
	case VertexFormat::UInt:      attribute_count = 1; return vk::Format::eR32Uint;
	case VertexFormat::UInt2:     attribute_count = 1; return vk::Format::eR32G32Uint;
	case VertexFormat::UInt3:     attribute_count = 1; return vk::Format::eR32G32B32Uint;
	case VertexFormat::UInt4:     attribute_count = 1; return vk::Format::eR32G32B32A32Uint;
	case VertexFormat::Long:      attribute_count = 1; return vk::Format::eR64Sint;
	case VertexFormat::Long2:     attribute_count = 1; return vk::Format::eR64G64Sint;
	case VertexFormat::Long3:     attribute_count = 1; return vk::Format::eR64G64B64Sint;
	case VertexFormat::Long4:     attribute_count = 1; return vk::Format::eR64G64B64A64Sint;
	case VertexFormat::ULong:     attribute_count = 1; return vk::Format::eR64Uint;
	case VertexFormat::ULong2:    attribute_count = 1; return vk::Format::eR64G64Uint;
	case VertexFormat::ULong3:    attribute_count = 1; return vk::Format::eR64G64B64Uint;
	case VertexFormat::ULong4:    attribute_count = 1; return vk::Format::eR64G64B64A64Uint;
	case VertexFormat::Float:     attribute_count = 1; return vk::Format::eR32Sfloat;
	case VertexFormat::Float2:    attribute_count = 1; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float3:    attribute_count = 1; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float4:    attribute_count = 1; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Float2x2:  attribute_count = 2; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float2x3:  attribute_count = 2; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float2x4:  attribute_count = 2; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Float3x2:  attribute_count = 3; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float3x3:  attribute_count = 3; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float3x4:  attribute_count = 3; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Float4x2:  attribute_count = 4; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float4x3:  attribute_count = 4; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float4x4:  attribute_count = 4; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Double:    attribute_count = 1; return vk::Format::eR64Sfloat;
	case VertexFormat::Double2:   attribute_count = 1; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double3:   attribute_count = 1; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double4:   attribute_count = 1; return vk::Format::eR64G64B64A64Sfloat;
	case VertexFormat::Double2x2: attribute_count = 2; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double2x3: attribute_count = 2; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double2x4: attribute_count = 2; return vk::Format::eR64G64B64A64Sfloat;
	case VertexFormat::Double3x2: attribute_count = 3; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double3x3: attribute_count = 3; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double3x4: attribute_count = 3; return vk::Format::eR64G64B64A64Sfloat;
	case VertexFormat::Double4x2: attribute_count = 4; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double4x3: attribute_count = 4; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double4x4: attribute_count = 4; return vk::Format::eR64G64B64A64Sfloat;
	}

	VERA_ASSERT_MSG(false, "invalid vertex format");
	return {};
}

static void fill_vertex_input_attributes(
	std::vector<vk::VertexInputAttributeDescription>& attributes,
	uint32_t                                          binding,
	uint32_t                                          input_attribute_count,
	const VertexInputAttribute*                       input_attributes,
	uint32_t&                                         location
) {
	uint32_t attribute_count;

	for (uint32_t i = 0; i < input_attribute_count; ++i) {
		auto& input_attribute = input_attributes[i];
		auto  vk_format       = get_vertex_format(input_attribute.format, attribute_count);

		for (uint32_t i = 0; i < attribute_count; ++i) {
			auto& attribute = attributes.emplace_back();
			attribute.location = location++;
			attribute.binding  = binding;
			attribute.format   = vk_format;
			attribute.offset   = input_attribute.offset;
		}
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

	if (!info.tessellationPatchControlPoints)
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

obj<Pipeline> Pipeline::create(obj<Device> device, const GraphicsPipelineCreateInfo& info)
{
	VERA_ASSERT_MSG(info.vertexShader && info.fragmentShader,
		"vertex shader and fragment shader are required");

	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_graphics_pipeline(info);

	if (auto it = device_impl.pipelineCacheMap.find(hash_value);
		it != device_impl.pipelineCacheMap.end()) {
		return obj<Pipeline>(it->second.get());
	}

	auto  obj  = createNewCoreObject<Pipeline>();
	auto& impl = getImpl(obj);

	impl.pipelineLayout = register_pipeline_layout(device, info);

	static_vector<vk::PipelineShaderStageCreateInfo, 10> shader_infos;
	fill_shader_info(impl, shader_infos, info);

	std::vector<vk::VertexInputAttributeDescription> vertex_attributes;
	vk::VertexInputBindingDescription                vertex_binding_descs[2];
	uint32_t                                         vertex_binding_desc_count = 1;
	uint32_t                                         vertex_location           = 0;

	if (info.vertexInputInfo.has_value()) {
		auto& vertex_info = info.vertexInputInfo.value();

		if (!vertex_info.vertexInputDescriptor.empty()) {
			auto& desc    = vertex_info.vertexInputDescriptor;
			auto& binding = vertex_binding_descs[0];
			
			fill_vertex_input_attributes(
				vertex_attributes,
				0,
				desc.attributeSize(),
				desc.attributeData(),
				vertex_location);

			binding.binding   = 0;
			binding.stride    = desc.vertexSize();
			binding.inputRate = vk::VertexInputRate::eVertex;
		} else {
			throw Exception("vertex input attribute cannot be empty");
		}

		if (!vertex_info.instanceInputDescriptor.empty()) {
			auto& desc    = vertex_info.vertexInputDescriptor;
			auto& binding = vertex_binding_descs[vertex_binding_desc_count++];

			fill_vertex_input_attributes(
				vertex_attributes,
				1,
				desc.attributeSize(),
				desc.attributeData(),
				vertex_location);

			binding.binding   = 1;
			binding.stride    = desc.vertexSize();
			binding.inputRate = vk::VertexInputRate::eInstance;
		}
	}

	vk::PipelineVertexInputStateCreateInfo vi_info;
	vi_info.vertexBindingDescriptionCount   = vertex_binding_desc_count;
	vi_info.pVertexBindingDescriptions      = vertex_binding_descs;
	vi_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes.size());
	vi_info.pVertexAttributeDescriptions    = vertex_attributes.data();

	vk::PipelineInputAssemblyStateCreateInfo ia_info;
	if (info.primitiveInfo) {
		ia_info.primitiveRestartEnable = info.primitiveInfo->enableRestart;
		ia_info.topology               = to_vk_primitive_topology(info.primitiveInfo->topology);
	}

	vk::PipelineTessellationStateCreateInfo ts_info;
	if (info.tessellationPatchControlPoints)
		ts_info.patchControlPoints = *info.tessellationPatchControlPoints;

	vk::Viewport temp_viewport;
	temp_viewport.width  = 1080.f;
	temp_viewport.height = -720.f;
	
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
		ds_info.depthTestEnable       = info.depthStencilInfo->depthFormat != DepthFormat::Unknown;
		ds_info.depthWriteEnable      = info.depthStencilInfo->depthWriteEnable;
		ds_info.depthCompareOp        = to_vk_compare_op(info.depthStencilInfo->depthCompareOp);
		ds_info.depthBoundsTestEnable = info.depthStencilInfo->depthBoundsTestEnable;
		ds_info.stencilTestEnable     = info.depthStencilInfo->stencilFormat != StencilFormat::Unknown;
		ds_info.front                 = to_vk_stencil_op_state(info.depthStencilInfo->front);
		ds_info.back                  = to_vk_stencil_op_state(info.depthStencilInfo->back);
		ds_info.minDepthBounds        = info.depthStencilInfo->minDepthBounds;
		ds_info.maxDepthBounds        = info.depthStencilInfo->maxDepthBounds;
	}

	vk::PipelineColorBlendAttachmentState blend_attachemnt;
	blend_attachemnt.blendEnable         = true;
	blend_attachemnt.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	blend_attachemnt.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	blend_attachemnt.colorBlendOp        = vk::BlendOp::eAdd;
	blend_attachemnt.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
	blend_attachemnt.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
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
	if (info.depthStencilInfo && info.depthStencilInfo->depthFormat != DepthFormat::Unknown) {
		auto format = to_vk_format(static_cast<Format>(info.depthStencilInfo->depthFormat));
		rendering_info.depthAttachmentFormat = format;
	}
	if (info.depthStencilInfo && info.depthStencilInfo->stencilFormat != StencilFormat::Unknown) {
		auto format = to_vk_format(static_cast<Format>(info.depthStencilInfo->stencilFormat));
		rendering_info.stencilAttachmentFormat = format;
	}

	vk::GraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.stageCount          = static_cast<uint32_t>(shader_infos.size());
	pipeline_info.pStages             = shader_infos.data();
	pipeline_info.pVertexInputState   = &vi_info;
	pipeline_info.pInputAssemblyState = info.primitiveInfo ? &ia_info : nullptr;
	pipeline_info.pTessellationState  = info.tessellationPatchControlPoints ? &ts_info : nullptr;
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

	device_impl.pipelineCacheMap.insert({ hash_value, obj });

	return obj;
}

Pipeline::~Pipeline()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.device.destroy(impl.pipeline);

	destroyObjectImpl(this);
}

obj<Device> Pipeline::getDevice()
{
	return getImpl(this).device;
}

obj<PipelineLayout> Pipeline::getPipelineLayout()
{
	return getImpl(this).pipelineLayout;
}

std::vector<obj<Shader>> Pipeline::enumerateShaders()
{
	std::vector<obj<Shader>> result;

	for (auto [flag, shader] : getImpl(this).shaders)
		result.push_back(std::move(shader));

	return result;
}

obj<Shader> Pipeline::getShader(ShaderStageFlagBits stage)
{
	auto& impl = getImpl(this);

	for (auto& [flag, shader] : impl.shaders)
		if (flag == stage) return shader;

	return obj<Shader>();
}

VERA_NAMESPACE_END