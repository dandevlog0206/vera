#pragma once

#include "../../impl/object_impl.h"

VERA_NAMESPACE_BEGIN

enum class ResourceType VERA_ENUM;

struct ResourceBindingNode;
struct ResourceBindingDesc;

typedef ResourceBindingNode* ResourceBindingNodePtr;

struct ResourceBindingLink
{
	ResourceBindingNodePtr next;
};

struct ResourceBindingNode : ResourceBindingLink
{
	ResourceBindingNodePtr prev;
	ResourceBindingDesc*   descPtr;
};

struct ResourceBindingDesc
{
	ref<ResourcePool>   resourcePool;

	ref<ResourceLayout> resourceLayout;
	ref<Sampler>        sampler;
	ref<TextureView>    textureView;
	ref<Buffer>         buffer;

	vk::DescriptorSet   descriptorSet;

	size_t              hashValue;
	ResourceType        type;
	uint32_t            binding;
	uint32_t            arrayElement;
	bool                needUpdate;
	bool                needDestroy;

	ResourceBindingLink rootLink;
};

VERA_NAMESPACE_END
