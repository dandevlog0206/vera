#include "../../include/vera/core/pipeline.h"
#include "../impl/device_impl.h"
#include "../impl/pipeline_impl.h"
#include "../impl/shader_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/util/static_vector.h"

#define MAX_SHADER_STAGE_COUNT 16

VERA_NAMESPACE_BEGIN

static const vk::PipelineViewportStateCreateInfo* get_default_viewport_state_info()
{
	// numbers here are arbitrary since we always use viewport and scissor dynamic states
	static const vk::Viewport temp_viewport{
		0.f,    // x
		0.f,    // y
		1080.f, // width
		-720.f, // height
		0.f,    // minDepth
		1.f     // maxDepth
	};

	static const vk::Rect2D temp_scissor{
		{ 0, 0 },     // offset
		{ 1080, 720 } // extent
	};

	static const vk::PipelineViewportStateCreateInfo info{
		{},             // flags
		1,              // viewportCount
		&temp_viewport, // pViewports
		1,              // scissorCount
		&temp_scissor   // pScissors
	};

	return &info;
}

static void check_shader_stage(const obj<Shader>& shader, ShaderStageFlagBits stage)
{	
	if (shader->getStageFlags() != stage)
		throw Exception("shader stage mismatch");
}

static obj<PipelineLayout> register_pipeline_layout(obj<Device> device, const GraphicsPipelineCreateInfo& info)
{
	if (info.pipelineLayout)
		return info.pipelineLayout;

	static_vector<const_ref<Shader>, MAX_SHADER_STAGE_COUNT> shaders;
	
	check_shader_stage(info.vertexShader, ShaderStageFlagBits::Vertex);
	check_shader_stage(info.fragmentShader, ShaderStageFlagBits::Fragment);

	shaders.push_back(info.vertexShader);
	shaders.push_back(info.fragmentShader);

	if (info.tessellationControlShader) {
		check_shader_stage(info.tessellationControlShader, ShaderStageFlagBits::TessellationControl);
		shaders.push_back(info.tessellationControlShader);
	}
	if (info.tessellationEvaluationShader) {
		check_shader_stage(info.tessellationEvaluationShader, ShaderStageFlagBits::TessellationEvaluation);
		shaders.push_back(info.tessellationEvaluationShader);
	}
	if (info.geometryShader) {
		check_shader_stage(info.geometryShader, ShaderStageFlagBits::Geometry);
		shaders.push_back(info.geometryShader);
	}

	return PipelineLayout::create(device, shaders);
}

static obj<PipelineLayout> register_pipeline_layout(obj<Device> device, const MeshPipelineCreateInfo& info)
{
	if (info.pipelineLayout)
		return info.pipelineLayout;

	static_vector<const_ref<Shader>, MAX_SHADER_STAGE_COUNT> shaders;

	check_shader_stage(info.meshShader, ShaderStageFlagBits::Mesh);
	check_shader_stage(info.fragmentShader, ShaderStageFlagBits::Fragment);

	shaders.push_back(info.meshShader);
	shaders.push_back(info.fragmentShader);

	if (info.taskShader) {
		check_shader_stage(info.taskShader, ShaderStageFlagBits::Task);
		shaders.push_back(info.taskShader);
	}

	return PipelineLayout::create(device, shaders);
}

static obj<PipelineLayout> register_pipeline_layout(obj<Device> device, const ComputePipelineCreateInfo& info)
{
	if (info.pipelineLayout)
		return info.pipelineLayout;

	check_shader_stage(info.computeShader, ShaderStageFlagBits::Compute);

	return PipelineLayout::create(device, info.computeShader.cref());
}

static void add_shader_stage(
	PipelineImpl&                                                             impl,
	static_vector<vk::PipelineShaderStageCreateInfo, MAX_SHADER_STAGE_COUNT>& shader_infos,
	const obj<Shader>&                                                        shader,
	ShaderStageFlagBits                                                       stage
) {
	VERA_ASSERT_MSG(shader, "shader is null");

	auto& shader_impl = const_cast<ShaderImpl&>(CoreObject::getImpl(shader));

	vk::PipelineShaderStageCreateInfo shader_info;
	shader_info.stage               = to_vk_shader_stage_flag_bits(stage);
	shader_info.module              = shader_impl.vkShaderModule;
	shader_info.pName               = shader_impl.entryPointName.data();
	shader_info.pSpecializationInfo = nullptr;

	shader_infos.push_back(shader_info);
	impl.shaders.push_back(std::make_pair(stage, shader));
}

static void fill_shader_info(
	PipelineImpl&                                                             impl,
	static_vector<vk::PipelineShaderStageCreateInfo, MAX_SHADER_STAGE_COUNT>& shader_infos,
	const GraphicsPipelineCreateInfo&                                         info
) {
	add_shader_stage(impl, shader_infos, info.vertexShader, ShaderStageFlagBits::Vertex);
	add_shader_stage(impl, shader_infos, info.fragmentShader, ShaderStageFlagBits::Fragment);

	if (info.tessellationControlShader)
		add_shader_stage(impl, shader_infos, info.tessellationControlShader,
			ShaderStageFlagBits::TessellationControl);

	if (info.tessellationEvaluationShader)
		add_shader_stage(impl, shader_infos, info.tessellationEvaluationShader,
			ShaderStageFlagBits::TessellationEvaluation);

	if (info.geometryShader)
		add_shader_stage(impl, shader_infos, info.geometryShader, ShaderStageFlagBits::Geometry);
}

static void fill_shader_info(
	PipelineImpl&                                                             impl,
	static_vector<vk::PipelineShaderStageCreateInfo, MAX_SHADER_STAGE_COUNT>& shader_infos,
	const MeshPipelineCreateInfo&                                             info
) {
	if (info.taskShader)
		add_shader_stage(impl, shader_infos, info.taskShader, ShaderStageFlagBits::Task);

	add_shader_stage(impl, shader_infos, info.meshShader, ShaderStageFlagBits::Mesh);
	add_shader_stage(impl, shader_infos, info.fragmentShader, ShaderStageFlagBits::Fragment);
}

static vk::Format get_vertex_format(VertexFormat format, uint32_t& attr_count)
{
	switch (format) {
	case VertexFormat::Char:      attr_count = 1; return vk::Format::eR8Sint;
	case VertexFormat::Char2:     attr_count = 1; return vk::Format::eR8G8Sint;
	case VertexFormat::Char3:     attr_count = 1; return vk::Format::eR8G8B8Sint;
	case VertexFormat::Char4:     attr_count = 1; return vk::Format::eR8G8B8A8Sint;
	case VertexFormat::UChar:     attr_count = 1; return vk::Format::eR8Uint;
	case VertexFormat::UChar2:    attr_count = 1; return vk::Format::eR8G8Uint;
	case VertexFormat::UChar3:    attr_count = 1; return vk::Format::eR8G8B8Uint;
	case VertexFormat::UChar4:    attr_count = 1; return vk::Format::eR8G8B8A8Uint;
	case VertexFormat::Short:     attr_count = 1; return vk::Format::eR16Sint;
	case VertexFormat::Short2:    attr_count = 1; return vk::Format::eR16G16Sint;
	case VertexFormat::Short3:    attr_count = 1; return vk::Format::eR16G16B16Sint;
	case VertexFormat::Short4:    attr_count = 1; return vk::Format::eR16G16B16A16Sint;
	case VertexFormat::UShort:    attr_count = 1; return vk::Format::eR16Uint;
	case VertexFormat::UShort2:   attr_count = 1; return vk::Format::eR16G16Uint;
	case VertexFormat::UShort3:   attr_count = 1; return vk::Format::eR16G16B16Uint;
	case VertexFormat::UShort4:   attr_count = 1; return vk::Format::eR16G16B16A16Uint;
	case VertexFormat::Int:       attr_count = 1; return vk::Format::eR32Sint;
	case VertexFormat::Int2:      attr_count = 1; return vk::Format::eR32G32Sint;
	case VertexFormat::Int3:      attr_count = 1; return vk::Format::eR32G32B32Sint;
	case VertexFormat::Int4:      attr_count = 1; return vk::Format::eR32G32B32A32Sint;
	case VertexFormat::UInt:      attr_count = 1; return vk::Format::eR32Uint;
	case VertexFormat::UInt2:     attr_count = 1; return vk::Format::eR32G32Uint;
	case VertexFormat::UInt3:     attr_count = 1; return vk::Format::eR32G32B32Uint;
	case VertexFormat::UInt4:     attr_count = 1; return vk::Format::eR32G32B32A32Uint;
	case VertexFormat::Long:      attr_count = 1; return vk::Format::eR64Sint;
	case VertexFormat::Long2:     attr_count = 1; return vk::Format::eR64G64Sint;
	case VertexFormat::Long3:     attr_count = 1; return vk::Format::eR64G64B64Sint;
	case VertexFormat::Long4:     attr_count = 1; return vk::Format::eR64G64B64A64Sint;
	case VertexFormat::ULong:     attr_count = 1; return vk::Format::eR64Uint;
	case VertexFormat::ULong2:    attr_count = 1; return vk::Format::eR64G64Uint;
	case VertexFormat::ULong3:    attr_count = 1; return vk::Format::eR64G64B64Uint;
	case VertexFormat::ULong4:    attr_count = 1; return vk::Format::eR64G64B64A64Uint;
	case VertexFormat::Float:     attr_count = 1; return vk::Format::eR32Sfloat;
	case VertexFormat::Float2:    attr_count = 1; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float3:    attr_count = 1; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float4:    attr_count = 1; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Float2x2:  attr_count = 2; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float2x3:  attr_count = 2; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float2x4:  attr_count = 2; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Float3x2:  attr_count = 3; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float3x3:  attr_count = 3; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float3x4:  attr_count = 3; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Float4x2:  attr_count = 4; return vk::Format::eR32G32Sfloat;
	case VertexFormat::Float4x3:  attr_count = 4; return vk::Format::eR32G32B32Sfloat;
	case VertexFormat::Float4x4:  attr_count = 4; return vk::Format::eR32G32B32A32Sfloat;
	case VertexFormat::Double:    attr_count = 1; return vk::Format::eR64Sfloat;
	case VertexFormat::Double2:   attr_count = 1; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double3:   attr_count = 1; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double4:   attr_count = 1; return vk::Format::eR64G64B64A64Sfloat;
	case VertexFormat::Double2x2: attr_count = 2; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double2x3: attr_count = 2; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double2x4: attr_count = 2; return vk::Format::eR64G64B64A64Sfloat;
	case VertexFormat::Double3x2: attr_count = 3; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double3x3: attr_count = 3; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double3x4: attr_count = 3; return vk::Format::eR64G64B64A64Sfloat;
	case VertexFormat::Double4x2: attr_count = 4; return vk::Format::eR64G64Sfloat;
	case VertexFormat::Double4x3: attr_count = 4; return vk::Format::eR64G64B64Sfloat;
	case VertexFormat::Double4x4: attr_count = 4; return vk::Format::eR64G64B64A64Sfloat;
	}

	VERA_ERROR_MSG("invalid vertex format");
}

static void fill_vertex_input_attributes(
	std::vector<vk::VertexInputAttributeDescription>& attributes,
	uint32_t                                          binding,
	uint32_t                                          input_attr_count,
	const VertexInputAttribute*                       input_attrs,
	uint32_t&                                         location
) {
	uint32_t attr_count;

	for (uint32_t i = 0; i < input_attr_count; ++i) {
		auto& input_attribute = input_attrs[i];
		auto  vk_format       = get_vertex_format(input_attribute.format, attr_count);

		for (uint32_t i = 0; i < attr_count; ++i) {
			auto& attribute = attributes.emplace_back();
			attribute.location = location++;
			attribute.binding  = binding;
			attribute.format   = vk_format;
			attribute.offset   = input_attribute.offset;
		}
	}
}

static void fill_primitive_state_info(
	vk::PipelineInputAssemblyStateCreateInfo& ps_info,
	const PrimitiveInfo&                      info
) {
	ps_info.topology               = to_vk_primitive_topology(info.topology);
	ps_info.primitiveRestartEnable = info.enableRestart;
}

static void fill_tessellation_state_info(
	vk::PipelineTessellationStateCreateInfo& ts_info,
	const uint32_t                           patch_control_points
) {
	ts_info.patchControlPoints = patch_control_points;
}

static void fill_rasterizer_state_info(
	vk::PipelineRasterizationStateCreateInfo& rs_info,
	const RasterizationInfo&                  info
) {
	rs_info.depthClampEnable        = info.depthClampEnable;
	rs_info.rasterizerDiscardEnable = info.rasterizerDiscardEnable;
	rs_info.polygonMode             = to_vk_polygon_mode(info.polygonMode);
	rs_info.cullMode                = to_vk_cull_mode_flags(info.cullMode);
	rs_info.frontFace               = to_vk_front_face(info.frontFace);
	rs_info.depthBiasEnable         = info.depthBiasEnable;
	rs_info.depthBiasConstantFactor = info.depthBiasConstantFactor;
	rs_info.depthBiasClamp          = info.depthBiasClamp;
	rs_info.depthBiasSlopeFactor    = info.depthBiasSlopeFactor;
	rs_info.lineWidth               = info.lineWidth;
}

//static void fill_multi_sample_state_info(
//	vk::PipelineMultisampleStateCreateInfo& multi_sample_info,
//	const MultiSampleStateCreateInfo&      info
//) {
//	multi_sample_info.rasterizationSamples = static_cast<vk::SampleCountFlagBits>(info.rasterizationSamples);
//	multi_sample_info.sampleShadingEnable   = info.sampleShadingEnable;
//	multi_sample_info.minSampleShading      = info.minSampleShading;
//	multi_sample_info.pSampleMask           = info.sampleMask.data();
//	multi_sample_info.alphaToCoverageEnable = info.alphaToCoverageEnable;
//	multi_sample_info.alphaToOneEnable     = info.alphaToOneEnable;
//}

static void fill_depth_stencil_state_info(
	vk::PipelineDepthStencilStateCreateInfo& ds_info,
	const DepthStencilInfo&                  info
) {
	ds_info.depthTestEnable       = info.depthFormat != DepthFormat::Unknown;
	ds_info.depthWriteEnable      = info.depthWriteEnable;
	ds_info.depthCompareOp        = to_vk_compare_op(info.depthCompareOp);
	ds_info.depthBoundsTestEnable = info.depthBoundsTestEnable;
	ds_info.stencilTestEnable     = info.stencilFormat != StencilFormat::Unknown;
	ds_info.front                 = to_vk_stencil_op_state(info.front);
	ds_info.back                  = to_vk_stencil_op_state(info.back);
	ds_info.minDepthBounds        = info.minDepthBounds;
	ds_info.maxDepthBounds        = info.maxDepthBounds;
}

static void fill_color_blend_state_info(
	vk::PipelineColorBlendStateCreateInfo& cb_info,
	const ColorBlendInfo&                  info
) {
	// VERA_VK_ABI_COMPATIBLE
	const auto* p_attachments = reinterpret_cast<
		const vk::PipelineColorBlendAttachmentState*>(info.attachments.data());

	cb_info.logicOpEnable     = info.enableLogicOp;
	cb_info.logicOp           = static_cast<vk::LogicOp>(info.logicOp);
	cb_info.attachmentCount   = static_cast<uint32_t>(info.attachments.size());
	cb_info.pAttachments      = p_attachments;
	cb_info.blendConstants[0] = info.blendConstants[0];
	cb_info.blendConstants[1] = info.blendConstants[1];
	cb_info.blendConstants[2] = info.blendConstants[2];
	cb_info.blendConstants[3] = info.blendConstants[3];
}

static void fill_dynamic_states(static_vector<vk::DynamicState, 64>& states, const GraphicsPipelineCreateInfo& info)
{
	states = {
		// viewport
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	for (const auto ds : info.dynamicStates)
		states.push_back(to_vk_dynamic_state(ds));

	//if (!info.primitiveInfo) {
	//	states.push_back(vk::DynamicState::ePrimitiveRestartEnable);
	//	states.push_back(vk::DynamicState::ePrimitiveTopology);
	//}

	//if (!info.rasterizationInfo) {
	//	states.push_back(vk::DynamicState::eDepthClampEnableEXT);
	//	states.push_back(vk::DynamicState::eRasterizerDiscardEnable);
	//	states.push_back(vk::DynamicState::ePolygonModeEXT);
	//	states.push_back(vk::DynamicState::eCullMode);
	//	states.push_back(vk::DynamicState::eFrontFace);
	//	states.push_back(vk::DynamicState::eDepthBiasEnable);
	//	states.push_back(vk::DynamicState::eDepthBias);
	//	states.push_back(vk::DynamicState::eLineWidth);
	//}

	//if (!info.tessellationPatchControlPoints)
	//	states.push_back(vk::DynamicState::ePatchControlPointsEXT);

	//if (!info.depthStencilInfo) {
	//	states.push_back(vk::DynamicState::eLogicOpEnableEXT);
	//	states.push_back(vk::DynamicState::eLogicOpEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEnableEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEquationEXT);
	//	states.push_back(vk::DynamicState::eColorWriteMaskEXT);
	//	states.push_back(vk::DynamicState::eBlendConstants);
	//}

	//if (!info.colorBlendInfo) {
	//	states.push_back(vk::DynamicState::eLogicOpEnableEXT);
	//	states.push_back(vk::DynamicState::eLogicOpEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEnableEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEquationEXT);
	//	states.push_back(vk::DynamicState::eColorWriteMaskEXT);
	//	states.push_back(vk::DynamicState::eBlendConstants);
	//}
}

static void fill_dynamic_states(static_vector<vk::DynamicState, 64>& states, const MeshPipelineCreateInfo& info)
{
	states = {
		// viewport
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	for (const auto ds : info.dynamicStates)
		states.push_back(to_vk_dynamic_state(ds));

	//if (!info.rasterizationInfo) {
	//	states.push_back(vk::DynamicState::eDepthClampEnableEXT);
	//	states.push_back(vk::DynamicState::eRasterizerDiscardEnable);
	//	states.push_back(vk::DynamicState::ePolygonModeEXT);
	//	states.push_back(vk::DynamicState::eCullMode);
	//	states.push_back(vk::DynamicState::eFrontFace);
	//	states.push_back(vk::DynamicState::eDepthBiasEnable);
	//	states.push_back(vk::DynamicState::eDepthBias);
	//	states.push_back(vk::DynamicState::eLineWidth);
	//}

	//if (!info.depthStencilInfo) {
	//	states.push_back(vk::DynamicState::eLogicOpEnableEXT);
	//	states.push_back(vk::DynamicState::eLogicOpEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEnableEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEquationEXT);
	//	states.push_back(vk::DynamicState::eColorWriteMaskEXT);
	//	states.push_back(vk::DynamicState::eBlendConstants);
	//}

	//if (!info.colorBlendInfo) {
	//	states.push_back(vk::DynamicState::eLogicOpEnableEXT);
	//	states.push_back(vk::DynamicState::eLogicOpEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEnableEXT);
	//	states.push_back(vk::DynamicState::eColorBlendEquationEXT);
	//	states.push_back(vk::DynamicState::eColorWriteMaskEXT);
	//	states.push_back(vk::DynamicState::eBlendConstants);
	//}
}

static hash_t hash_pipeline_info(const GraphicsPipelineCreateInfo& info)
{
	hash_t seed = 0;

	hash_combine(seed, info.vertexShader->hash());
	hash_combine(seed, info.tessellationControlShader ? info.tessellationControlShader->hash() : 0);
	hash_combine(seed, info.tessellationEvaluationShader ? info.tessellationEvaluationShader->hash() : 0);
	hash_combine(seed, info.geometryShader ? info.geometryShader->hash() : 0);
	hash_combine(seed, info.fragmentShader->hash());

	return seed;
}

static hash_t hash_pipeline_info(const MeshPipelineCreateInfo& info)
{
	hash_t seed = 1;

	hash_combine(seed, info.taskShader ? info.taskShader->hash() : 0);
	hash_combine(seed, info.meshShader->hash());
	hash_combine(seed, info.fragmentShader->hash());

	return seed;
}

static hash_t hash_pipeline_info(const ComputePipelineCreateInfo& info)
{
	hash_t seed = 2;
	
	hash_combine(seed, info.computeShader->hash());

	return seed;
}

const vk::Pipeline& get_vk_pipeline(const_ref<Pipeline> pipeline) VERA_NOEXCEPT
{
	return CoreObject::getImpl(pipeline).vkPipeline;
}

vk::Pipeline& get_vk_pipeline(ref<Pipeline> pipeline) VERA_NOEXCEPT
{
	return CoreObject::getImpl(pipeline).vkPipeline;
}

obj<Pipeline> Pipeline::create(obj<Device> device, const GraphicsPipelineCreateInfo& info)
{
	if (!info.vertexShader)
		throw Exception("Graphics pipeline must have a vertex shader");
	if (!info.fragmentShader)
		throw Exception("Graphics pipeline must have a fragment shader");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_pipeline_info(info);

	if (auto cached_obj = device_impl.findCachedObject<Pipeline>(hash_value))
		return cached_obj;

	// create pipline layout first to check shader stages
	auto pipeline_layout = register_pipeline_layout(device, info);

	auto  obj  = createNewCoreObject<Pipeline>();
	auto& impl = getImpl(obj);

	static_vector<vk::PipelineShaderStageCreateInfo, MAX_SHADER_STAGE_COUNT> shader_infos;
	fill_shader_info(impl, shader_infos, info);

	std::vector<vk::VertexInputAttributeDescription> vertex_attributes;
	vk::VertexInputBindingDescription                vertex_binding_descs[2];
	uint32_t                                         vertex_binding_desc_count = 0;
	uint32_t                                         vertex_location           = 0;

	if (info.vertexInputInfo.has_value()) {
		auto& vertex_info = info.vertexInputInfo.value();

		if (!vertex_info.vertexInputDescriptor.empty()) {
			auto& desc    = vertex_info.vertexInputDescriptor;
			auto& binding = vertex_binding_descs[vertex_binding_desc_count++];
			
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
		fill_primitive_state_info(ia_info, *info.primitiveInfo);
	} else {
		fill_primitive_state_info(ia_info, PrimitiveInfo{});
	}

	vk::PipelineTessellationStateCreateInfo ts_info;
	if (info.tessellationPatchControlPoints) {
		fill_tessellation_state_info(ts_info, *info.tessellationPatchControlPoints);
	} else {
		fill_tessellation_state_info(ts_info, 0);
	}

	vk::PipelineRasterizationStateCreateInfo rs_info;
	if (info.rasterizationInfo) {
		fill_rasterizer_state_info(rs_info, *info.rasterizationInfo);
	} else {
		fill_rasterizer_state_info(rs_info, RasterizationInfo{});
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
		fill_depth_stencil_state_info(ds_info, *info.depthStencilInfo);
	} else {
		fill_depth_stencil_state_info(ds_info, DepthStencilInfo{});
	}

	vk::PipelineColorBlendStateCreateInfo cb_info;
	if (info.colorBlendInfo) {
		fill_color_blend_state_info(cb_info, *info.colorBlendInfo);
	} else {
		fill_color_blend_state_info(cb_info, ColorBlendInfo{});
	}

	static_vector<vk::DynamicState, 64> dynamic_states;
	fill_dynamic_states(dynamic_states, info);

	vk::PipelineDynamicStateCreateInfo dynamic_info;
	dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_info.pDynamicStates    = dynamic_states.data();

	static_vector<vk::Format, 32> vk_color_formats;
	for (const auto& format : info.colorAttachmentFormats) {
		vk_color_formats.push_back(format == Format::Unknown ?
								   to_vk_format(device_impl.colorFormat) :
								   to_vk_format(format));
	}

	vk::PipelineRenderingCreateInfoKHR rendering_info;
	rendering_info.colorAttachmentCount    = static_cast<uint32_t>(vk_color_formats.size());
	rendering_info.pColorAttachmentFormats = vk_color_formats.data();

	if (info.depthStencilInfo) {
		rendering_info.depthAttachmentFormat   =
			to_vk_format(static_cast<Format>(info.depthStencilInfo->depthFormat));
		rendering_info.stencilAttachmentFormat =
			to_vk_format(static_cast<Format>(info.depthStencilInfo->stencilFormat));
	} else {
		rendering_info.depthAttachmentFormat   = vk::Format::eUndefined; 
		rendering_info.stencilAttachmentFormat = vk::Format::eUndefined;
	}

	vk::GraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.stageCount          = static_cast<uint32_t>(shader_infos.size());
	pipeline_info.pStages             = shader_infos.data();
	pipeline_info.pVertexInputState   = &vi_info;
	pipeline_info.pInputAssemblyState = &ia_info;
	pipeline_info.pTessellationState  = &ts_info;
	pipeline_info.pViewportState      = get_default_viewport_state_info();
	pipeline_info.pRasterizationState = &rs_info;
	pipeline_info.pMultisampleState   = &ms_info;
	pipeline_info.pDepthStencilState  = &ds_info;
	pipeline_info.pColorBlendState    = &cb_info;
	pipeline_info.pDynamicState       = &dynamic_info;
	pipeline_info.layout              = get_vk_pipeline_layout(pipeline_layout);
	pipeline_info.renderPass          = nullptr;
	pipeline_info.subpass             = 0;
	pipeline_info.basePipelineHandle  = nullptr;
	pipeline_info.basePipelineIndex   = 0;
	pipeline_info.pNext               = &rendering_info;

	auto result = device_impl.vkDevice.createGraphicsPipeline(device_impl.vkPipelineCache, pipeline_info);

	if (result.result != vk::Result::eSuccess)
		throw Exception("failed to create graphics pipeline");

	impl.device            = std::move(device);
	impl.pipelineLayout    = std::move(pipeline_layout);
	impl.vkPipeline        = result.value;
	impl.pipelineBindPoint = PipelineBindPoint::Graphics;
	impl.hashValue         = hash_value;

	device_impl.registerCachedObject<Pipeline>(impl.hashValue, obj);

	return obj;
}

obj<Pipeline> Pipeline::create(obj<Device> device, const MeshPipelineCreateInfo& info)
{
	if (!info.meshShader)
		throw Exception("Mesh pipeline must have a mesh shader");
	if (!info.fragmentShader)
		throw Exception("Mesh pipeline must have a fragment shader");

	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_pipeline_info(info);

	if (auto cached_obj = device_impl.findCachedObject<Pipeline>(hash_value))
		return cached_obj;

	if (info.taskShader && !device_impl.isFeatureEnabled(DeviceFeatureType::TaskShader))
		throw Exception("task shader feature is not enabled on device");
	if (!device_impl.isFeatureEnabled(DeviceFeatureType::MeshShader))
		throw Exception("mesh shader feature is not enabled on device");

	auto pipeline_layout = register_pipeline_layout(device, info);

	auto  obj  = createNewCoreObject<Pipeline>();
	auto& impl = getImpl(obj);

	static_vector<vk::PipelineShaderStageCreateInfo, MAX_SHADER_STAGE_COUNT> shader_infos;
	fill_shader_info(impl, shader_infos, info);

	vk::PipelineRasterizationStateCreateInfo rs_info;
	if (info.rasterizationInfo) {
		fill_rasterizer_state_info(rs_info, *info.rasterizationInfo);
	} else {
		fill_rasterizer_state_info(rs_info, RasterizationInfo{});
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
		fill_depth_stencil_state_info(ds_info, *info.depthStencilInfo);
	} else {
		fill_depth_stencil_state_info(ds_info, DepthStencilInfo{});
	}

	vk::PipelineColorBlendStateCreateInfo cb_info;
	if (info.colorBlendInfo) {
		fill_color_blend_state_info(cb_info, *info.colorBlendInfo);
	} else {
		fill_color_blend_state_info(cb_info, ColorBlendInfo{});
	}

	static_vector<vk::DynamicState, 64> dynamic_states;
	fill_dynamic_states(dynamic_states, info);

	vk::PipelineDynamicStateCreateInfo dynamic_info;
	dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_info.pDynamicStates    = dynamic_states.data();

	static_vector<vk::Format, 32> vk_color_formats;
	for (const auto& format : info.colorAttachmentFormats) {
		vk_color_formats.push_back(format == Format::Unknown ?
								   to_vk_format(device_impl.colorFormat) :
								   to_vk_format(format));
	}

	vk::PipelineRenderingCreateInfoKHR rendering_info;
	rendering_info.colorAttachmentCount    = static_cast<uint32_t>(vk_color_formats.size());
	rendering_info.pColorAttachmentFormats = vk_color_formats.data();

	if (info.depthStencilInfo) {
		rendering_info.depthAttachmentFormat   =
			to_vk_format(static_cast<Format>(info.depthStencilInfo->depthFormat));
		rendering_info.stencilAttachmentFormat =
			to_vk_format(static_cast<Format>(info.depthStencilInfo->stencilFormat));
	} else {
		rendering_info.depthAttachmentFormat   = vk::Format::eUndefined; 
		rendering_info.stencilAttachmentFormat = vk::Format::eUndefined;
	}

	vk::GraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.stageCount          = static_cast<uint32_t>(shader_infos.size());
	pipeline_info.pStages             = shader_infos.data();
	pipeline_info.pViewportState      = get_default_viewport_state_info();
	pipeline_info.pRasterizationState = &rs_info;
	pipeline_info.pMultisampleState   = &ms_info;
	pipeline_info.pDepthStencilState  = &ds_info;
	pipeline_info.pColorBlendState    = &cb_info;
	pipeline_info.pDynamicState       = &dynamic_info;
	pipeline_info.layout              = get_vk_pipeline_layout(pipeline_layout);
	pipeline_info.renderPass          = nullptr;
	pipeline_info.subpass             = 0;
	pipeline_info.basePipelineHandle  = nullptr;
	pipeline_info.basePipelineIndex   = 0;
	pipeline_info.pNext               = &rendering_info;

	auto result = device_impl.vkDevice.createGraphicsPipeline(device_impl.vkPipelineCache, pipeline_info);

	if (result.result != vk::Result::eSuccess)
		throw Exception("failed to create graphics pipeline");

	impl.device            = std::move(device);
	impl.pipelineLayout    = std::move(pipeline_layout);
	impl.vkPipeline        = result.value;
	impl.pipelineBindPoint = PipelineBindPoint::Graphics;
	impl.hashValue         = hash_value;

	device_impl.registerCachedObject<Pipeline>(impl.hashValue, obj);

	return obj;
}

obj<Pipeline> Pipeline::create(obj<Device> device, const ComputePipelineCreateInfo& info)
{
	if (!info.computeShader)
		throw Exception("Compute pipeline must have a compute shader");

	auto&  device_impl = getImpl(device);
	size_t hash_value  = hash_pipeline_info(info);

	if (auto cached_obj = device_impl.findCachedObject<Pipeline>(hash_value))
		return cached_obj;

	// create pipline layout first to check shader stages
	auto pipeline_layout = register_pipeline_layout(device, info);
	
	auto  obj         = createNewCoreObject<Pipeline>();
	auto& impl        = getImpl(obj);
	auto& shader_impl = getImpl(info.computeShader);
	// auto* root_node   = get_reflection_root_node(shader_impl.shaderReflection);

	VERA_NOT_IMPLEMENTED;

	vk::ComputePipelineCreateInfo pipeline_info;
	pipeline_info.stage.stage  = vk::ShaderStageFlagBits::eCompute;
	pipeline_info.stage.module = shader_impl.vkShaderModule;
	pipeline_info.stage.pName  = "main"; //root_node->getEntryPointName(ShaderStageFlagBits::Compute);
	pipeline_info.layout       = get_vk_pipeline_layout(pipeline_layout);

	auto result = device_impl.vkDevice.createComputePipeline(device_impl.vkPipelineCache, pipeline_info);

	if (result.result != vk::Result::eSuccess)
		throw Exception("failed to create compute pipeline");

	impl.device            = std::move(device);
	impl.pipelineLayout    = std::move(pipeline_layout);
	impl.vkPipeline        = result.value;
	impl.shaders           = { std::make_pair(ShaderStageFlagBits::Compute, info.computeShader) };
	impl.pipelineBindPoint = PipelineBindPoint::Compute;
	impl.hashValue         = hash_value;

	device_impl.registerCachedObject<Pipeline>(impl.hashValue, obj);

	return obj;
}

Pipeline::~Pipeline() VERA_NOEXCEPT
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	device_impl.unregisterCachedObject<Pipeline>(impl.hashValue);
	device_impl.vkDevice.destroy(impl.vkPipeline);

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
	
	return {};
}

VERA_NAMESPACE_END