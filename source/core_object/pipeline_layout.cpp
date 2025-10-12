#include "../../include/vera/core/pipeline_layout.h"
#include "../impl/device_impl.h"
#include "../impl/pipeline_layout_impl.h"
#include "../impl/resource_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/util/hash.h"
#include "../../include/vera/util/static_vector.h"

#define VERA_MAX_SHADER_COUNT 8
#define VERA_MAX_SET_COUNT 64
#define VERA_MAX_BINDING_COUNT 128

#define UNSIZED_ARRAY_BINDING_FLAGS \
	ResourceLayoutBindingFlagBits::UpdateAfterBind | \
	ResourceLayoutBindingFlagBits::PartiallyBound | \
	ResourceLayoutBindingFlagBits::VariableBindingCount

VERA_NAMESPACE_BEGIN

typedef static_vector<ReflectionBlockDescPtr, VERA_MAX_SHADER_COUNT> BlockDescArray;
typedef static_vector<const ReflectionResourceDesc*, VERA_MAX_SET_COUNT> SetDescArray;
typedef static_vector<const ReflectionResourceDesc*, VERA_MAX_BINDING_COUNT> BindingReflectionArray;

static uint32_t get_max_resource_count(const PipelineLayoutImpl& impl, ResourceType resource_type)
{
	const auto& indexing_props = CoreObject::getImpl(impl.device).descriptorIndexingProperties;

	switch (resource_type) {
	case ResourceType::Sampler:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers;
	case ResourceType::CombinedImageSampler:
		return std::min(
			indexing_props.maxPerStageDescriptorUpdateAfterBindSamplers,
			indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages);
	case ResourceType::SampledImage:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindSampledImages;
	case ResourceType::StorageImage:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindStorageImages;
	case ResourceType::UniformTexelBuffer:
	case ResourceType::UniformBuffer:
	case ResourceType::UniformBufferDynamic:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
	case ResourceType::StorageTexelBuffer:
	case ResourceType::StorageBuffer:
	case ResourceType::StorageBufferDynamic:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
	case ResourceType::InputAttachment:
		return indexing_props.maxPerStageDescriptorUpdateAfterBindInputAttachments;
	default:
		VERA_ASSERT_MSG(false, "invalid resource type");
	}
}

static bool sort_name_map(const ReflectionNameMap& a, const ReflectionNameMap& b)
{
	int32_t cmp = std::strcmp(a.name, b.name);

	if (cmp != 0)
		return cmp < 0;

	return static_cast<uint32_t>(a.stageFlags) < static_cast<uint32_t>(b.stageFlags);
}

// Checks shader reflection descriptors are identical except shader stage flags and name
static bool check_compatible_reflection(const ReflectionDesc* a, const ReflectionDesc* b)
{
	if (a == b) return true;
	if (a->type != b->type) return false;

	switch (a->type) {
	case ReflectionType::Primitive:{
		const auto& desc_a = static_cast<const ReflectionPrimitiveDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionPrimitiveDesc&>(*b);

		return desc_a.offset == desc_b.offset && desc_a.primitiveType == desc_b.primitiveType;
	}
	case ReflectionType::Array: {
		const auto& desc_a = static_cast<const ReflectionArrayDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionArrayDesc&>(*b);

		return 
			desc_a.offset == desc_b.offset &&
			desc_a.stride == desc_b.stride &&
			desc_a.elementCount == desc_b.elementCount &&
			check_compatible_reflection(desc_a.element, desc_b.element);
	}
	case ReflectionType::Struct: {
		const auto& desc_a = static_cast<const ReflectionStructDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionStructDesc&>(*b);
		
		if (desc_a.offset != desc_b.offset || desc_a.memberCount != desc_b.memberCount)
			return false;

		for (size_t i = 0; i < desc_a.memberCount; ++i)
			if (!check_compatible_reflection(desc_a.members[i], desc_b.members[i]))
				return false;
		
		return true;
	}
	case ReflectionType::Resource: {
		const auto& desc_a = static_cast<const ReflectionResourceDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionResourceDesc&>(*b);

		return
			desc_a.resourceType == desc_b.resourceType &&
			desc_a.set == desc_b.set&&
			desc_a.binding == desc_b.binding;
	}
	case ReflectionType::ResourceBlock: {
		const auto& desc_a = static_cast<const ReflectionResourceBlockDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionResourceBlockDesc&>(*b);
		
		if (desc_a.resourceType != desc_b.resourceType ||
			desc_a.set != desc_b.set ||
			desc_a.binding != desc_b.binding ||
			desc_a.sizeInByte != desc_b.sizeInByte ||
			desc_a.memberCount != desc_b.memberCount)
			return false;

		for (size_t i = 0; i < desc_a.memberCount; ++i)
			if (!check_compatible_reflection(desc_a.members[i], desc_b.members[i]))
				return false;

		return true;
	}
	case ReflectionType::ResourceArray: {
		const auto& desc_a = static_cast<const ReflectionResourceArrayDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionResourceArrayDesc&>(*b);

		return
			desc_a.resourceType == desc_b.resourceType &&
			desc_a.set == desc_b.set &&
			desc_a.binding == desc_b.binding &&
			desc_a.elementCount == desc_b.elementCount &&
			check_compatible_reflection(desc_a.element, desc_b.element);
	}
	case ReflectionType::PushConstant: {
		const auto& desc_a = static_cast<const ReflectionPushConstantDesc&>(*a);
		const auto& desc_b = static_cast<const ReflectionPushConstantDesc&>(*b);

		if (desc_a.sizeInByte != desc_b.sizeInByte ||
			desc_a.memberCount != desc_b.memberCount)
			return false;
		
		for (size_t i = 0; i < desc_a.memberCount; ++i)
			if (!check_compatible_reflection(desc_a.members[i], desc_b.members[i]))
				return false;

		return true;
	}
	default:
		VERA_ASSERT_MSG(false, "invalid reflection type");
	}
}

void fill_struct_next_member(array_view<ReflectionBlockDescPtr> block_descs, BlockDescArray& next_members) {
	next_members.resize(block_descs.size());

	for (size_t i = 0; i < block_descs.size(); ++i) {
		const auto* desc = static_cast<const ReflectionStructDesc*>(block_descs[i]);
		next_members[i] = desc->members[i];
	}
}

template <class RootMemberType>
void fill_root_next_member(array_view<ReflectionRootMemberDescPtr> root_descs, BlockDescArray& next_members) {
	next_members.resize(root_descs.size());

	for (size_t i = 0; i < root_descs.size(); ++i) {
		const auto* desc = static_cast<const RootMemberType*>(root_descs[i]);
		next_members[i] = desc->members[i];
	}
}

static ReflectionBlockDesc* copy_block_reflection(const ReflectionBlockDesc* source)
{
	switch (source->type) {
	case ReflectionType::Primitive: {
		const auto* prim_desc = static_cast<const ReflectionPrimitiveDesc*>(source);
		auto*       new_desc  = new ReflectionPrimitiveDesc;
		
		new_desc->type          = ReflectionType::Primitive;
		new_desc->offset        = prim_desc->offset;
		new_desc->primitiveType = prim_desc->primitiveType;
	
		return new_desc;
	}
	case ReflectionType::Array: {
		const auto* array_desc  = static_cast<const ReflectionArrayDesc*>(source);
		auto*       new_desc    = new ReflectionArrayDesc;

		new_desc->type         = ReflectionType::Array;
		new_desc->offset       = array_desc->offset;
		new_desc->stride       = array_desc->stride;
		new_desc->elementCount = array_desc->elementCount;
		new_desc->element      = copy_block_reflection(array_desc->element);
		
		return new_desc;
	}
	case ReflectionType::Struct: {
		const auto* struct_desc = static_cast<const ReflectionStructDesc*>(source);
		auto*       new_desc    = new ReflectionStructDesc;
		
		new_desc->type         = ReflectionType::Struct;
		new_desc->offset       = struct_desc->offset;
		new_desc->memberCount  = struct_desc->memberCount;
		new_desc->members      = new ReflectionBlockDesc*[struct_desc->memberCount];
		new_desc->nameMapCount = struct_desc->nameMapCount;
		new_desc->nameMaps     = new ReflectionNameMap[struct_desc->nameMapCount];
		
		for (uint32_t i = 0; i < struct_desc->memberCount; ++i)
			new_desc->members[i] = copy_block_reflection(struct_desc->members[i]);
		
		std::copy_n(struct_desc->nameMaps, struct_desc->nameMapCount, new_desc->nameMaps);
		
		return new_desc;
	}
	default:
		VERA_ASSERT_MSG(false, "invalid block reflection type");
	}
}

static ReflectionRootMemberDesc* copy_root_reflection(const ReflectionRootMemberDesc* source)
{
	switch (source->type) {
	case ReflectionType::Resource: {
		const auto* res_desc = static_cast<const ReflectionResourceDesc*>(source);
		auto*       new_desc = new ReflectionResourceDesc;
		
		new_desc->type           = ReflectionType::Resource;
		new_desc->stageFlags     = res_desc->stageFlags;
		new_desc->resourceLayout = res_desc->resourceLayout;
		new_desc->resourceType   = res_desc->resourceType;
		new_desc->set            = res_desc->set;
		new_desc->binding        = res_desc->binding;
		
		return new_desc;
	}
	case ReflectionType::ResourceBlock: {
		const auto* block_desc = static_cast<const ReflectionResourceBlockDesc*>(source);
		auto*       new_desc   = new ReflectionResourceBlockDesc;

		new_desc->type           = ReflectionType::ResourceBlock;
		new_desc->stageFlags     = block_desc->stageFlags;
		new_desc->resourceLayout = block_desc->resourceLayout;
		new_desc->resourceType   = block_desc->resourceType;
		new_desc->set            = block_desc->set;
		new_desc->binding        = block_desc->binding;
		new_desc->sizeInByte     = block_desc->sizeInByte;
		new_desc->memberCount    = block_desc->memberCount;
		new_desc->members        = new ReflectionBlockDesc*[block_desc->memberCount];
		new_desc->nameMapCount   = block_desc->nameMapCount;
		new_desc->nameMaps       = new ReflectionNameMap[block_desc->nameMapCount];

		for (uint32_t i = 0; i < block_desc->memberCount; ++i)
			new_desc->members[i] = copy_block_reflection(block_desc->members[i]);

		std::copy_n(block_desc->nameMaps, block_desc->nameMapCount, new_desc->nameMaps);

		return new_desc;
	}
	case ReflectionType::ResourceArray: {
		const auto* array_desc  = static_cast<const ReflectionResourceArrayDesc*>(source);
		auto*       new_desc    = new ReflectionResourceArrayDesc;
		auto*       new_element = copy_root_reflection(array_desc->element);
		
		new_desc->type           = ReflectionType::ResourceArray;
		new_desc->stageFlags     = array_desc->stageFlags;
		new_desc->resourceLayout = array_desc->resourceLayout;
		new_desc->resourceType   = array_desc->resourceType;
		new_desc->set            = array_desc->set;
		new_desc->binding        = array_desc->binding;
		new_desc->elementCount   = array_desc->elementCount;
		new_desc->element        = static_cast<ReflectionResourceDesc*>(new_element);
		
		return new_desc;
	}
	case ReflectionType::PushConstant: {
		const auto* pc_desc  = static_cast<const ReflectionPushConstantDesc*>(source);
		auto*       new_desc = new ReflectionPushConstantDesc;
		
		new_desc->type         = ReflectionType::PushConstant;
		new_desc->sizeInByte   = pc_desc->sizeInByte;
		new_desc->memberCount  = pc_desc->memberCount;
		new_desc->members      = new ReflectionBlockDesc*[pc_desc->memberCount];
		new_desc->nameMapCount = pc_desc->nameMapCount;
		new_desc->nameMaps     = new ReflectionNameMap[pc_desc->nameMapCount];
		
		for (uint32_t i = 0; i < pc_desc->memberCount; ++i)
			new_desc->members[i] = copy_block_reflection(pc_desc->members[i]);
		
		std::copy_n(pc_desc->nameMaps, pc_desc->nameMapCount, new_desc->nameMaps);
		
		return new_desc;
	}
	default:
		VERA_ASSERT_MSG(false, "invalid root reflection type");
	}
}

static ReflectionNameMap* find_name_map(ReflectionNameMap* name_maps, uint32_t name_map_count, const char* name)
{
	for (uint32_t i = 0; i < name_map_count; ++i)
		if (std::strcmp(name_maps[i].name, name) == 0)
			return name_maps + i;
	return nullptr;
}

template <class NameMapType>
static void insert_name_map(
	NameMapType&             name_map_type,
	string_pool&             pool,
	const ReflectionNameMap& new_name_map
) {
	ReflectionNameMap* name_map = nullptr;

	for (uint32_t i = 0; i < name_map_type.nameMapCount; ++i) {
		const auto& cmp_map = name_map_type.nameMaps[i];

		if (std::strcmp(cmp_map.name, new_name_map.name) == 0 && cmp_map.index == new_name_map.index) {
			name_map = name_map_type.nameMaps + i;
			break;
		}
	}
	
	if (name_map) {
		name_map->stageFlags |= new_name_map.stageFlags;
	} else {
		// TODO: consider allocate size=1,2,4,8...

		const uint32_t name_map_count     = name_map_type.nameMapCount;
		const uint32_t new_name_map_count = name_map_count + 1;
		auto*          new_name_maps      = new ReflectionNameMap[new_name_map_count];
		
		if (name_map_type.nameMaps) {
			std::copy_n(name_map_type.nameMaps, name_map_count, new_name_maps);
			delete[] name_map_type.nameMaps;
		}

		new_name_maps[name_map_count] = new_name_map;
		
		// TODO: consider more efficient way to keep sorted order
		std::sort(VERA_SPAN_ARRAY(new_name_maps, new_name_map_count), sort_name_map);

		name_map_type.nameMapCount = new_name_map_count;
		name_map_type.nameMaps     = new_name_maps;
	}
}

static void insert_name_map(
	std::vector<ReflectionNameMap>& name_maps,
	string_pool&                    pool,
	const ReflectionNameMap&        new_name_map
) {
	const auto find_name_map = [=](const ReflectionNameMap& a) {
		return std::strcmp(a.name, new_name_map.name) == 0 && a.index == new_name_map.index;
	};

	if (auto it = std::find_if(VERA_SPAN(name_maps), find_name_map); it != name_maps.cend()) {
		it->stageFlags |= new_name_map.stageFlags;
	} else {
		name_maps.push_back(new_name_map);
		std::sort(name_maps.begin(), name_maps.end(), sort_name_map);
	}
}

template <class NameMapType>
static const char* find_name_by_index(const NameMapType& name_map_type, uint32_t index)
{
	for (uint32_t i = 0; i < name_map_type.nameMapCount; ++i)
		if (name_map_type.nameMaps[i].index == index)
			return name_map_type.nameMaps[i].name;
	
	VERA_ASSERT_MSG(false, "name map with specified index not found");
	return nullptr; // to suppress warning
}

static void combine_block_reflection_name_map(
	ReflectionBlockDesc*       target,
	string_pool&               pool,
	const ReflectionBlockDesc* source
) {
	VERA_ASSERT(target->type == source->type);

	switch (target->type) {
	case ReflectionType::Primitive:
		// nothing to do
		break;
	case ReflectionType::Array: {
		const auto* array_source = static_cast<const ReflectionArrayDesc*>(source);
		auto*       array_target = static_cast<ReflectionArrayDesc*>(target);
		
		combine_block_reflection_name_map(
			array_target->element,
			pool,
			array_source->element);
	} break;
	case ReflectionType::Struct: {
		const auto* struct_source = static_cast<const ReflectionStructDesc*>(source);
		auto*       struct_target = static_cast<ReflectionStructDesc*>(target);

		for (uint32_t i = 0; i < struct_source->memberCount; ++i)
			combine_block_reflection_name_map(
				struct_target->members[i],
				pool,
				struct_source->members[i]);
		
		for (uint32_t i = 0; i < struct_source->nameMapCount; ++i)
			insert_name_map(*struct_target, pool, struct_source->nameMaps[i]);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid block reflection type");
	}
}

static void combine_root_reflection_name_map(
	ReflectionRootMemberDesc*       target,
	string_pool&                    pool,
	const ReflectionRootMemberDesc* source
) {
	VERA_ASSERT(target->type == source->type);

	switch (target->type) {
	case ReflectionType::Resource: {
		const auto* res_source = static_cast<const ReflectionResourceDesc*>(source);
		auto*       res_target = static_cast<ReflectionResourceDesc*>(target);

		res_target->stageFlags |= res_source->stageFlags;
	} break;
	case ReflectionType::ResourceBlock: {
		const auto* block_source = static_cast<const ReflectionResourceBlockDesc*>(source);
		auto*       block_target = static_cast<ReflectionResourceBlockDesc*>(target);

		for (uint32_t i = 0; i < block_source->memberCount; ++i)
			combine_block_reflection_name_map(
				block_target->members[i],
				pool,
				block_source->members[i]);

		block_target->stageFlags |= block_source->stageFlags;
		for (uint32_t i = 0; i < block_source->nameMapCount; ++i)
			insert_name_map(
				*block_target,
				pool,
				block_source->nameMaps[i]);
	} break;
	case ReflectionType::ResourceArray: {
		const auto* array_source = static_cast<const ReflectionResourceArrayDesc*>(source);
		auto*       array_target = static_cast<ReflectionResourceArrayDesc*>(target);

		combine_root_reflection_name_map(array_target->element, pool, array_source->element);

	} break;
	case ReflectionType::PushConstant: {
		const auto* pc_source = static_cast<const ReflectionPushConstantDesc*>(source);
		auto*       pc_target = static_cast<ReflectionPushConstantDesc*>(target);

		for (uint32_t i = 0; i < pc_source->memberCount; ++i)
			combine_block_reflection_name_map(
				pc_target->members[i],
				pool,
				pc_source->members[i]);

		pc_target->stageFlags |= pc_source->stageFlags;
		for (uint32_t i = 0; i < pc_source->nameMapCount; ++i)
			insert_name_map(
				*pc_target,
				pool,
				pc_source->nameMaps[i]);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid root reflection type");
	}
}

static bool find_min_binding(
	array_view<BindingReflectionArray> shader_bindings,
	array_view<uint32_t>               shader_indices,
	size_t&                            out_shader_idx,
	uint32_t&                          out_binding_id
) {
	out_shader_idx = 0;
	out_binding_id = UINT32_MAX;

	for (size_t i = 0; i < shader_bindings.size(); ++i) {
		const auto& bindings = shader_bindings[i];
		const auto  idx      = shader_indices[i];
		
		if (idx < bindings.size() && bindings[idx]->binding < out_binding_id) {
			out_shader_idx = i;
			out_binding_id = bindings[idx]->binding;
		}
	}

	return out_binding_id != UINT32_MAX;
}

static void merge_set(
	PipelineLayoutImpl&                       impl,
	array_view<const ShaderReflection*>       shader_reflections,
	array_view<BindingReflectionArray>        shader_bindings,
	uint32_t                                  set_id,
	std::vector<ReflectionRootMemberDescPtr>& out_root_descs,
	std::vector<ReflectionNameMap>&           out_name_maps
) {
	static_vector<uint32_t, VERA_MAX_SHADER_COUNT> indices(shader_bindings.size(), 0);

	size_t   cmp_shader_idx;
	uint32_t binding_id;

	while (find_min_binding(shader_bindings, indices, cmp_shader_idx, binding_id)) {
		const auto& cmp_shader     = shader_bindings[cmp_shader_idx];
		const auto* cmp_binding    = cmp_shader[indices[cmp_shader_idx]];
		const auto  reflection_idx = static_cast<uint32_t>(out_root_descs.size());
		auto*       new_desc       = copy_root_reflection(cmp_binding);

		out_root_descs.push_back(new_desc);
		out_name_maps.push_back(ReflectionNameMap{
			.name       = impl.namePool << find_name_by_index(
				*shader_reflections[cmp_shader_idx],
				cmp_binding->reflectionIndex),
			.stageFlags = cmp_binding->stageFlags,
			.index      = reflection_idx
		});

		++indices[cmp_shader_idx];

		for (size_t i = cmp_shader_idx + 1; i < shader_bindings.size(); ++i) {
			const auto& bindings = shader_bindings[i];
			const auto  idx      = indices[i];

			if (bindings.size() <= idx || bindings[idx]->binding != binding_id) continue;

			if (!check_compatible_reflection(cmp_binding, bindings[idx]))
				throw Exception("incompatible set binding at set={}, binding={}", set_id, binding_id);

			combine_root_reflection_name_map(new_desc, impl.namePool, bindings[idx]);

			const auto  target_idx = bindings[idx]->reflectionIndex;
			const char* name       = find_name_by_index(*shader_reflections[i], target_idx);

			out_name_maps.push_back(ReflectionNameMap{
				.name       = impl.namePool << name,
				.stageFlags = bindings[idx]->stageFlags,
				.index      = reflection_idx
			});

			++indices[i];
		}
	}
}

static uint32_t get_set_count(array_view<const_ref<Shader>> shaders)
{
	uint32_t max_set_count = 0;

	for (const auto& shader : shaders)
		max_set_count = std::max(max_set_count, CoreObject::getImpl(shader).reflection.maxSetCount);

	return max_set_count;
}

static int32_t find_reflection_first_binding(const ShaderReflection& reflection, uint32_t set_id)
{
	for (uint32_t i = 0; i < reflection.resourceCount; ++i) {
		const auto* res_desc = static_cast<const ReflectionResourceDesc*>(reflection.reflections[i]);

		if (res_desc->set == set_id)
			return static_cast<int32_t>(i);
	}

	return -1;
}

static const ReflectionPushConstantDesc* find_pc_reflection(const ShaderReflection& reflection)
{
	if (reflection.pushConstantCount == 0) return nullptr;

	VERA_ASSERT(reflection.pushConstantCount == 1);

	const auto* desc    = reflection.reflections[reflection.resourceCount];
	const auto* pc_desc = static_cast<const ReflectionPushConstantDesc*>(desc);

	VERA_ASSERT(pc_desc->type == ReflectionType::PushConstant);

	return pc_desc;
}

static const char* find_pc_name(const ShaderReflection& reflection)
{
	if (reflection.pushConstantCount == 0) return nullptr;

	VERA_ASSERT(reflection.pushConstantCount == 1);

	const auto idx = reflection.resourceCount;

	for (uint32_t i = 0; i < reflection.nameMapCount; ++i)
		if (reflection.nameMaps[i].index == idx)
			return reflection.nameMaps[i].name;
	
	return nullptr;
}

static void create_reflection_info(PipelineLayoutImpl& impl, array_view<const_ref<Shader>> shaders)
{
	static const auto array_is_empty = [](const BindingReflectionArray& arr) { return arr.empty(); };

	std::vector<ReflectionRootMemberDescPtr> root_descs;
	std::vector<ReflectionNameMap>           name_maps;

	const uint32_t shader_count = shaders.size();
	const uint32_t set_count    = get_set_count(shaders);
	uint32_t       res_count    = 0;
	uint32_t       pc_count     = 0;
	
	static_vector<const ShaderReflection*, VERA_MAX_SHADER_COUNT> shader_reflections;

	for (auto& shader : shaders)
		shader_reflections.push_back(&CoreObject::getImpl(shader).reflection);

	// check set compatibility and append resource reflection descriptors
	for (uint32_t set_id = 0; set_id < set_count; ++set_id) {
		static_vector<BindingReflectionArray, VERA_MAX_SHADER_COUNT> shader_bindings(shaders.size());

		for (uint32_t shader_idx = 0; shader_idx < shader_count; ++shader_idx) {
			const auto& reflection  = *shader_reflections[shader_idx];
			const auto* reflections = reflection.reflections;

			auto idx = find_reflection_first_binding(reflection, set_id);

			if (idx == -1) continue;

			for (uint32_t i = static_cast<uint32_t>(idx); i < reflection.resourceCount; ++i) {
				const auto* res_desc = static_cast<const ReflectionResourceDesc*>(reflections[i]);

				if (res_desc->set != set_id) break;

				shader_bindings[shader_idx].push_back(res_desc);
			}
		}

		const auto last = std::remove_if(VERA_SPAN(shader_bindings), array_is_empty);
		shader_bindings.erase(last, shader_bindings.cend());

		if (shader_bindings.empty())
			throw Exception("set={} not found, set must be in contiguous order", set_id);

		if (1 < shader_bindings.size()) {
			// will throw an exception if not compatible
			merge_set(impl, shader_reflections, shader_bindings, set_id, root_descs, name_maps);
		} else {
			for (const auto* desc : shader_bindings.front())
				root_descs.push_back(copy_root_reflection(desc));
		}
	}

	res_count = static_cast<uint32_t>(root_descs.size());

	// append push constant reflection descriptors
	for (size_t i = 0; i < shader_count; ++i) {
		const auto& target_reflection = *shader_reflections[i];
		const auto* target_desc       = find_pc_reflection(target_reflection);
		
		if (target_desc == nullptr) continue;

		auto* new_desc = copy_root_reflection(target_desc);
		auto  desc_idx = static_cast<uint32_t>(root_descs.size());

		root_descs.push_back(new_desc);
		name_maps.push_back(ReflectionNameMap{
			.name       = impl.namePool << find_pc_name(target_reflection),
			.stageFlags = target_desc->stageFlags,
			.index      = desc_idx
		});

		// TODO: optimize, skip checking if already pushed
		for (size_t j = i + 1; j < shader_count; ++j) {
			const auto& cmp_reflection = *shader_reflections[j];
			const auto* cmp_desc       = find_pc_reflection(target_reflection);

			if (cmp_desc != nullptr && check_compatible_reflection(target_desc, cmp_desc)) {
				const char* name = find_pc_name(cmp_reflection);
				
				new_desc->stageFlags |= cmp_desc->stageFlags;
				name_maps.push_back(ReflectionNameMap{
					.name       = impl.namePool << name,
					.stageFlags = cmp_desc->stageFlags,
					.index      = desc_idx
				});
			}
		}
	}

	pc_count = static_cast<uint32_t>(root_descs.size()) - res_count;

	for (uint32_t i = 0; i < root_descs.size(); ++i)
		root_descs[i]->reflectionIndex = i;

	std::sort(VERA_SPAN(name_maps), sort_name_map);

	impl.reflection.reflectionCount   = static_cast<uint32_t>(root_descs.size());
	impl.reflection.reflections       = new ReflectionRootMemberDesc*[root_descs.size()];
	impl.reflection.nameMapCount      = static_cast<uint32_t>(name_maps.size());
	impl.reflection.nameMaps          = new ReflectionNameMap[name_maps.size()];
	impl.reflection.resourceCount     = res_count;
	impl.reflection.pushConstantCount = pc_count;
	impl.reflection.maxSetCount       = set_count;

	// TODO: consider release vector memeory
	std::copy(VERA_SPAN(root_descs), impl.reflection.reflections);
	std::copy(VERA_SPAN(name_maps), impl.reflection.nameMaps);
}

static void create_resource_layout(PipelineLayoutImpl& impl)
{
	ResourceLayoutCreateInfo layout_info;

	for (uint32_t set_id = 0; set_id < impl.reflection.maxSetCount; ++set_id) {
		auto idx = find_reflection_first_binding(impl.reflection, set_id);

		VERA_ASSERT(idx != -1);

		layout_info.bindings.clear();
		layout_info.flags = ResourceLayoutCreateFlags{};

		for (; static_cast<uint32_t>(idx) < impl.reflection.resourceCount; ++idx) {
			const auto* desc     = impl.reflection.reflections[idx];
			const auto* res_desc = static_cast<const ReflectionResourceDesc*>(desc);
			
			if (res_desc->set != set_id) break;
			
			auto& layout_binding = layout_info.bindings.emplace_back();
			layout_binding.binding      = res_desc->binding;
			layout_binding.resourceType = res_desc->resourceType;
			layout_binding.stageFlags   = res_desc->stageFlags;

			if (res_desc->type == ReflectionType::ResourceArray) {
				auto& array_desc = static_cast<const ReflectionResourceArrayDesc&>(*res_desc);

				if (array_desc.elementCount == UINT32_MAX) {
					layout_binding.flags         = UNSIZED_ARRAY_BINDING_FLAGS;
					layout_binding.resourceCount = get_max_resource_count(impl, array_desc.resourceType);
					layout_info.flags            = ResourceLayoutCreateFlagBits::UpdateAfterBindPool;
				} else {
					layout_binding.resourceCount = array_desc.elementCount;
				}
			} else {
				layout_binding.resourceCount = 1;
			}
		}

		auto new_layout = ResourceLayout::create(impl.device, layout_info);
		impl.resourceLayouts.push_back(std::move(new_layout));
	}
}

static void create_pc_info(PipelineLayoutImpl& impl)
{
	const auto offset = impl.reflection.resourceCount;

	for (uint32_t i = offset; i < impl.reflection.reflectionCount; ++i) {
		const auto* desc    = impl.reflection.reflections[i];
		const auto* pc_desc = static_cast<const ReflectionPushConstantDesc*>(desc);
		
		VERA_ASSERT(pc_desc->type == ReflectionType::PushConstant);
		
		auto& range = impl.pushConstantRanges.emplace_back();
		range.offset     = 0;
		range.size       = pc_desc->sizeInByte;
		range.stageFlags = pc_desc->stageFlags;
	}
}

static void create_pipeline_layout(const DeviceImpl & device_impl, PipelineLayoutImpl& impl)
{
	static_vector<vk::DescriptorSetLayout, VERA_MAX_SET_COUNT> vk_layouts;
	static_vector<vk::PushConstantRange, VERA_MAX_SET_COUNT>   vk_constants;

	for (const auto& layout : impl.resourceLayouts)
		vk_layouts.push_back(get_vk_descriptor_set_layout(layout));
	for (const auto& range : impl.pushConstantRanges)
		vk_constants.push_back(get_vk_push_constant_range(range));
	
	vk::PipelineLayoutCreateInfo pipeline_layout_info;
	pipeline_layout_info.setLayoutCount         = static_cast<uint32_t>(vk_layouts.size());
	pipeline_layout_info.pSetLayouts            = vk_layouts.data();
	pipeline_layout_info.pushConstantRangeCount = static_cast<uint32_t>(vk_constants.size());
	pipeline_layout_info.pPushConstantRanges    = vk_constants.data();

	impl.pipelineLayout = device_impl.device.createPipelineLayout(pipeline_layout_info);
}

static ShaderStageFlags get_shader_stage_flags(array_view<const_ref<Shader>> shaders)
{
	ShaderStageFlags flags;

	for (const auto& shader : shaders) {
		const auto shader_stage = CoreObject::getImpl(shader).stageFlags;

		if (flags.has(shader_stage))
			throw Exception("duplicate shader stage");

		flags |= shader_stage;
	}

	return flags;
}

static PipelineBindPoint get_pipeline_bind_point(array_view<const_ref<Shader>> shaders)
{
	const auto& first_impl = CoreObject::getImpl(shaders.front());
	const auto  bind_point = first_impl.pipelineBindPoint;

	for (size_t i = 1; i < shaders.size(); ++i)
		if (CoreObject::getImpl(shaders[i]).pipelineBindPoint != bind_point)
			throw Exception("all shaders must have same pipeline bind point");

	return bind_point;
}

static bool check_shader_device(ref<Device> device, array_view<const_ref<Shader>> shaders)
{
	if (shaders.empty()) return false;

	for (const auto shader : shaders)
		if (!shader || CoreObject::getImpl(shader).device != device)
			return false;
	return true;
}

static bool check_layout_device(ref<Device> device, array_view<obj<ResourceLayout>> layouts)
{
	if (layouts.empty()) return false;

	for (const auto layout : layouts)
		if (!layout || CoreObject::getImpl(layout).device != device)
			return false;
	return true;
}

static hash_t hash_pipeline_layout_with_shaders(array_view<const_ref<Shader>> shaders)
{
	hash_t seed = 0;

	hash_combine(seed, shaders.size());
	for (const auto& shader : shaders)
		hash_combine(seed, shader->hash());

	return seed;
}

static hash_t hash_pipeline_layout(const PipelineLayoutCreateInfo& info)
{
	hash_t seed = 0;

	hash_combine(seed, info.resourceLayouts.size());
	for (const auto& layout : info.resourceLayouts)
		hash_combine(seed, layout->hash());
		
	hash_combine(seed, info.pushConstantRanges.size());
	for (const auto& range : info.pushConstantRanges) {
		hash_combine(seed, range.offset);
		hash_combine(seed, range.size);
		hash_combine(seed, static_cast<uint32_t>(range.stageFlags));
	}

	hash_combine(seed, info.pipelineBindPoint);

	return seed;
}

const vk::PipelineLayout& get_vk_pipeline_layout(const_ref<PipelineLayout> pipeline_layout)
{
	return CoreObject::getImpl(pipeline_layout).pipelineLayout;
}

vk::PipelineLayout& get_vk_pipeline_layout(ref<PipelineLayout> pipeline_layout)
{
	return CoreObject::getImpl(pipeline_layout).pipelineLayout;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, array_view<const_ref<Shader>> shaders)
{
	VERA_ASSERT_MSG(device, "device is null");
	VERA_ASSERT_MSG(check_shader_device(device, shaders), "shader device mismatch");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_pipeline_layout_with_shaders(shaders);

	if (auto it = device_impl.pipelineLayoutCacheMapByShader.find(hash_value);
		it != device_impl.pipelineLayoutCacheMapByShader.end())
		return unsafe_obj_cast<PipelineLayout>(it->second);

	auto  obj  = createNewCoreObject<PipelineLayout>();
	auto& impl = getImpl(obj);

	impl.device            = std::move(device);
	impl.pipelineBindPoint = get_pipeline_bind_point(shaders);
	impl.stageFlags        = get_shader_stage_flags(shaders);
	impl.hashValue         = 0;
	impl.shaderHashValue   = hash_value;

	create_reflection_info(impl, shaders);
	create_resource_layout(impl);
	create_pc_info(impl);
	create_pipeline_layout(device_impl, impl);

	for (uint32_t set_id = 0, resource_idx = 0; set_id < impl.reflection.maxSetCount; ++set_id) {
		auto& resource_layout = impl.resourceLayouts[set_id];

		while (resource_idx < impl.reflection.resourceCount) {
			auto* desc     = impl.reflection.reflections[resource_idx];
			auto* res_desc = static_cast<ReflectionResourceDesc*>(desc);

			if (res_desc->set != set_id) break;

			res_desc->resourceLayout = resource_layout;
			resource_idx            += 1;
		}
	}

	device_impl.pipelineLayoutCacheMapByShader.insert({ hash_value, obj });

	return obj;
}

obj<PipelineLayout> PipelineLayout::create(obj<Device> device, const PipelineLayoutCreateInfo& info)
{
	VERA_ASSERT_MSG(device, "device is null");
	VERA_ASSERT_MSG(check_layout_device(device, info.resourceLayouts), "layout device mismatch");

	auto&  device_impl = getImpl(device);
	hash_t hash_value  = hash_pipeline_layout(info);

	if (auto it = device_impl.pipelineLayoutCacheMap.find(hash_value);
		it != device_impl.pipelineLayoutCacheMap.end())
		return unsafe_obj_cast<PipelineLayout>(it->second);

	auto  obj  = createNewCoreObject<PipelineLayout>();
	auto& impl = getImpl(obj);

	impl.resourceLayouts.assign(info.resourceLayouts.begin(), info.resourceLayouts.end());
	impl.pushConstantRanges.assign(info.pushConstantRanges.begin(), info.pushConstantRanges.end());
	
	// TODO: fill reflection datas

	impl.device            = std::move(device);
	impl.reflection        = {};
	impl.pipelineBindPoint = info.pipelineBindPoint;
	impl.hashValue         = hash_value;
	impl.shaderHashValue   = 0;

	create_pipeline_layout(device_impl, impl);
	
	device_impl.pipelineLayoutCacheMap.insert({ hash_value, obj });

	return obj;
}

PipelineLayout::~PipelineLayout()
{
	auto& impl        = getImpl(this);
	auto& device_impl = getImpl(impl.device);

	if (impl.hashValue)
		device_impl.pipelineLayoutCacheMap.erase(impl.hashValue);
	if (impl.shaderHashValue)
		device_impl.pipelineLayoutCacheMapByShader.erase(impl.shaderHashValue);

	device_impl.device.destroy(impl.pipelineLayout);

	destroyObjectImpl(this);
}

obj<Device> PipelineLayout::getDevice()
{
	return getImpl(this).device;
}

array_view<ref<ResourceLayout>> PipelineLayout::getResourceLayouts() const
{
	auto& impl = getImpl(this);

	return { 
		reinterpret_cast<const ref<ResourceLayout>*>(impl.resourceLayouts.data()),
		impl.resourceLayouts.size()
	};
}

uint32_t PipelineLayout::getResourceLayoutCount() const
{
	return  static_cast<uint32_t>(getImpl(this).resourceLayouts.size());
}

const_ref<ResourceLayout> PipelineLayout::getResourceLayout(uint32_t set) const
{
	return getImpl(this).resourceLayouts[set];
}

array_view<PushConstantRange> PipelineLayout::getPushConstantRanges() const
{
	return getImpl(this).pushConstantRanges;
}

PipelineBindPoint PipelineLayout::getPipelineBindPoint() const
{
	return getImpl(this).pipelineBindPoint;
}

ShaderStageFlags PipelineLayout::getShaderStageFlags() const
{
	return getImpl(this).stageFlags;
}

bool PipelineLayout::hasReflection() const VERA_NOEXCEPT
{
	return 0 < getImpl(this).reflection.reflectionCount;
}

bool PipelineLayout::isCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT
{
	return isResourceCompatible(pipeline_layout) && isPushConstantCompatible(pipeline_layout);
}

bool PipelineLayout::isResourceCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT
{
	return false;
}

bool PipelineLayout::isPushConstantCompatible(const_ref<PipelineLayout> pipeline_layout) const VERA_NOEXCEPT
{
	return false;
}

size_t PipelineLayout::hash() const
{
	return getImpl(this).hashValue;
}

VERA_NAMESPACE_END