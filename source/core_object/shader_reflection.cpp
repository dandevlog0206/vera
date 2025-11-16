#include "../../include/vera/core/shader_reflection.h"
#include "../impl/shader_impl.h"
#include "../impl/shader_reflection_impl.h"

#include "../../include/vera/core/shader.h"
#include "../../include/vera/util/static_vector.h"

#define MAX_SHADER_STAGE_COUNT 16

VERA_NAMESPACE_BEGIN

ReflectionVariable::ReflectionVariable(
	const ReflectionNode* node,
	ShaderStageFlags      stage_mask,
	uint32_t              offset
) VERA_NOEXCEPT :
	m_node(node),
	m_stage_mask(stage_mask) {}

ReflectionVariable::ReflectionVariable() VERA_NOEXCEPT :
	m_node(nullptr),
	m_stage_mask(ShaderStageFlagBits::None) {}

ReflectionVariable ReflectionVariable::operator[](uint32_t idx) const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_node->type == ReflectionType::DescriptorArray || m_node->type == ReflectionType::Array,
		"variable is not subscriptable");
	VERA_ASSERT_MSG(idx < m_node->getElementCount(), "array index out of bounds");

	uint32_t offset = 0;

	if (m_node->type == ReflectionType::Array)
		offset = m_offset + m_node->getStride() * idx;

	return ReflectionVariable(m_node->getElementNode(), m_stage_mask, offset);
}

ReflectionVariable ReflectionVariable::operator[](std::string_view name) const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(
		m_node->type == ReflectionType::DescriptorBlock ||
		m_node->type == ReflectionType::PushConstant ||
		m_node->type == ReflectionType::Struct, "variable is not subscriptable");

	return ReflectionVariable();
}

ShaderStageFlags ReflectionVariable::getShaderStageFlags() const VERA_NOEXCEPT
{
	return m_stage_mask;
}

bool ReflectionVariable::isSubscriptible() const VERA_NOEXCEPT
{
	return m_node->hasProperty(ReflectionPropertyFlagBits::ElementCount);
}

uint32_t ReflectionVariable::getElementCount() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(isSubscriptible(), "variable is not subscriptable");
	return m_node->getElementCount();
}

bool ReflectionVariable::empty() const VERA_NOEXCEPT
{
	return m_node == nullptr;
}

obj<ShaderReflection> ShaderReflection::create(array_view<obj<Shader>> shaders)
{
	if (shaders.size() == 1) {
		auto& shader_impl = const_cast<ShaderImpl&>(getImpl(shaders.front()));
		return shader_impl.shaderReflection;
	}

	auto  obj  = createNewCoreObject<ShaderReflection>();
	auto& impl = getImpl(obj);

	impl.shaderReflections.reserve(shaders.size());
	for (auto shader : shaders) {
		auto& shader_impl = const_cast<ShaderImpl&>(getImpl(shader));
		impl.shaderReflections.push_back(shader_impl.shaderReflection);
	}

	impl.targetFlags = ReflectionTargetFlagBits::PipelineLayout;

	return obj;
}

ShaderReflection::~ShaderReflection() VERA_NOEXCEPT
{
	destroyObjectImpl(this);
}

ReflectionTargetFlags ShaderReflection::getTargetFlags() const VERA_NOEXCEPT
{
	return getImpl(this).targetFlags;
}

ShaderStageFlags ShaderReflection::getShaderStageFlags() const
{
	return getImpl(this).requestMinimalReflectionRootNode()->getShaderStageFlags();
}

uint3 ShaderReflection::getLocalSize() const VERA_NOEXCEPT
{
	auto* root_node = getImpl(this).requestMinimalReflectionRootNode();

	return uint3(
		root_node->localSizeX,
		root_node->localSizeY,
		root_node->localSizeZ);
}

const ReflectionRootNode* ShaderReflectionImpl::requestMinimalReflectionRootNode() const
{
	if (auto* root_node = reflectionDesc.getRootNode())
		return root_node;

	if (targetFlags == ReflectionTargetFlagBits::Shader) {
		reflectionDesc.parse(
			spirvCode.data(),
			spirvCode.size(),
			ReflectionParseMode::Minimal);
	} else if (targetFlags == ReflectionTargetFlagBits::PipelineLayout) {
		static_vector<const ReflectionRootNode*, MAX_SHADER_STAGE_COUNT> reflections;

		for (auto& shader_refl : shaderReflections) {
			auto& refl_impl = CoreObject::getImpl(shader_refl);
			reflections.push_back(refl_impl.requestMinimalReflectionRootNode());
		}

		reflectionDesc.merge(reflections);
	}

	return reflectionDesc.getRootNode();
}

const ReflectionRootNode* ShaderReflectionImpl::requestFullReflectionRootNode() const
{
	auto* root_node = reflectionDesc.getRootNode();

	if (root_node && root_node->parseMode == ReflectionParseMode::Full)
		return root_node;

	if (targetFlags == ReflectionTargetFlagBits::Shader) {
		reflectionDesc.parse(
			spirvCode.data(),
			spirvCode.size(),
			ReflectionParseMode::Full);

		spirvCode.clear();
	} else if (targetFlags == ReflectionTargetFlagBits::PipelineLayout) {
		static_vector<const ReflectionRootNode*, MAX_SHADER_STAGE_COUNT> reflections;

		for (auto& shader_refl : shaderReflections) {
			auto& refl_impl = CoreObject::getImpl(shader_refl);
			reflections.push_back(refl_impl.requestFullReflectionRootNode());
		}

		reflectionDesc.merge(reflections);
	}

	return reflectionDesc.getRootNode();
}

VERA_NAMESPACE_END