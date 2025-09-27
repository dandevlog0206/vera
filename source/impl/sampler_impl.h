#pragma once

#include "object_impl.h"

#include "../../include/vera/core/sampler.h"

VERA_NAMESPACE_BEGIN

struct SamplerImpl
{
	obj<Device>       device;

	vk::Sampler       sampler;

	size_t            hashValue;
	SamplerCreateInfo info;
};

static vk::Filter to_vk_filter(SamplerFilter filter)
{
	switch (filter) {
	case SamplerFilter::Nearest: return vk::Filter::eNearest;
	case SamplerFilter::Linear:  return vk::Filter::eLinear;
	}

	VERA_ASSERT_MSG(false, "invalid sampler filter");
	return {};
}

static vk::SamplerMipmapMode to_vk_sampler_mipmap_mode(SamplerMipmapMode filter)
{
	switch (filter) {
	case SamplerMipmapMode::Nearest: return vk::SamplerMipmapMode::eNearest;
	case SamplerMipmapMode::Linear:  return vk::SamplerMipmapMode::eLinear;
	}

	VERA_ASSERT_MSG(false, "invalid sampler filter");
	return {};
}

static vk::SamplerAddressMode to_vk_sampler_address_mode(SamplerAddressMode mode)
{
	switch (mode) {
	case SamplerAddressMode::Repeat:          return vk::SamplerAddressMode::eRepeat;
	case SamplerAddressMode::MirroredRepeat:  return vk::SamplerAddressMode::eMirroredRepeat;
	case SamplerAddressMode::ClampToEdge:     return vk::SamplerAddressMode::eClampToEdge;
	case SamplerAddressMode::ClampToBorder:   return vk::SamplerAddressMode::eClampToBorder;
	case SamplerAddressMode::MirrorClampToEdge:return vk::SamplerAddressMode::eMirrorClampToEdge;
	}

	VERA_ASSERT_MSG(false, "invalid sampler address mode");
	return {};
}

static vk::CompareOp to_vk_compare_op(SamplerCompareOp op)
{
	switch (op) {
	case SamplerCompareOp::Never:          return vk::CompareOp::eNever;
	case SamplerCompareOp::Less:           return vk::CompareOp::eLess;
	case SamplerCompareOp::Equal:          return vk::CompareOp::eEqual;
	case SamplerCompareOp::LessOrEqual:    return vk::CompareOp::eLessOrEqual;
	case SamplerCompareOp::Greater:        return vk::CompareOp::eGreater;
	case SamplerCompareOp::NotEqual:       return vk::CompareOp::eNotEqual;
	case SamplerCompareOp::GreaterOrEqual: return vk::CompareOp::eGreaterOrEqual;
	case SamplerCompareOp::Always:         return vk::CompareOp::eAlways;
	}

	VERA_ASSERT_MSG(false, "invalid compare op");
	return {};
}

VERA_NAMESPACE_END