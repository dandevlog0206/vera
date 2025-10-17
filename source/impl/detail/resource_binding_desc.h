#pragma once

#include "../../impl/object_impl.h"

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;

struct DescriptorBindingNode;
struct DescriptorBindingDesc;

typedef DescriptorBindingNode* DescriptorBindingNodePtr;

struct DescriptorBindingLink
{
	DescriptorBindingNodePtr next;
};

struct DescriptorBindingNode : DescriptorBindingLink
{
	DescriptorBindingNodePtr prev;
	DescriptorBindingDesc*   descPtr;
};

struct DescriptorBindingDesc
{
	ref<DescriptorPool>      descriptorPool;

	ref<DescriptorSetLayout> descriptorSetLayout;
	ref<Sampler>             sampler;
	ref<TextureView>         textureView;
	ref<Buffer>              buffer;

	vk::DescriptorSet        descriptorSet;

	size_t                   hashValue;
	DescriptorType           type;
	uint32_t                 binding;
	uint32_t                 arrayElement;
	bool                     needUpdate;
	bool                     needDestroy;

	DescriptorBindingLink    rootLink;
};

VERA_NAMESPACE_END
