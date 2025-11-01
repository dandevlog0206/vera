#include "../../include/vera/core/shader_storage.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/pipeline_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/core/descriptor_pool.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
#include "../../include/vera/core/texture.h"
#include "../../include/vera/core/texture_view.h"

VERA_NAMESPACE_BEGIN

template <class T>
static constexpr auto primitive_type_v = ReflectionPrimitiveType::Unknown;
template <> static constexpr auto primitive_type_v<bool2>     = ReflectionPrimitiveType::Bool_2;
template <> static constexpr auto primitive_type_v<bool3>     = ReflectionPrimitiveType::Bool_3;
template <> static constexpr auto primitive_type_v<bool4>     = ReflectionPrimitiveType::Bool_4;
template <> static constexpr auto primitive_type_v<char2>     = ReflectionPrimitiveType::Int8_2;
template <> static constexpr auto primitive_type_v<char3>     = ReflectionPrimitiveType::Int8_3;
template <> static constexpr auto primitive_type_v<char4>     = ReflectionPrimitiveType::Int8_4;
template <> static constexpr auto primitive_type_v<uchar2>    = ReflectionPrimitiveType::UInt8_2;
template <> static constexpr auto primitive_type_v<uchar3>    = ReflectionPrimitiveType::UInt8_3;
template <> static constexpr auto primitive_type_v<uchar4>    = ReflectionPrimitiveType::UInt8_4;
template <> static constexpr auto primitive_type_v<short2>    = ReflectionPrimitiveType::Int16_2;
template <> static constexpr auto primitive_type_v<short3>    = ReflectionPrimitiveType::Int16_3;
template <> static constexpr auto primitive_type_v<short4>    = ReflectionPrimitiveType::Int16_4;
template <> static constexpr auto primitive_type_v<ushort2>   = ReflectionPrimitiveType::UInt16_2;
template <> static constexpr auto primitive_type_v<ushort3>   = ReflectionPrimitiveType::UInt16_3;
template <> static constexpr auto primitive_type_v<ushort4>   = ReflectionPrimitiveType::UInt16_4;
template <> static constexpr auto primitive_type_v<int2>      = ReflectionPrimitiveType::Int32_2;
template <> static constexpr auto primitive_type_v<int3>      = ReflectionPrimitiveType::Int32_3;
template <> static constexpr auto primitive_type_v<int4>      = ReflectionPrimitiveType::Int32_4;
template <> static constexpr auto primitive_type_v<uint2>     = ReflectionPrimitiveType::UInt32_2;
template <> static constexpr auto primitive_type_v<uint3>     = ReflectionPrimitiveType::UInt32_3;
template <> static constexpr auto primitive_type_v<uint4>     = ReflectionPrimitiveType::UInt32_4;
template <> static constexpr auto primitive_type_v<long2>     = ReflectionPrimitiveType::Int64_2;
template <> static constexpr auto primitive_type_v<long3>     = ReflectionPrimitiveType::Int64_3;
template <> static constexpr auto primitive_type_v<long4>     = ReflectionPrimitiveType::Int64_4;
template <> static constexpr auto primitive_type_v<ulong2>    = ReflectionPrimitiveType::UInt64_2;
template <> static constexpr auto primitive_type_v<ulong3>    = ReflectionPrimitiveType::UInt64_3;
template <> static constexpr auto primitive_type_v<ulong4>    = ReflectionPrimitiveType::UInt64_4;
template <> static constexpr auto primitive_type_v<float2>    = ReflectionPrimitiveType::Float_2;
template <> static constexpr auto primitive_type_v<float3>    = ReflectionPrimitiveType::Float_3;
template <> static constexpr auto primitive_type_v<float4>    = ReflectionPrimitiveType::Float_4;
template <> static constexpr auto primitive_type_v<float2x2>  = ReflectionPrimitiveType::Float_2x2;
template <> static constexpr auto primitive_type_v<float2x3>  = ReflectionPrimitiveType::Float_2x3;
template <> static constexpr auto primitive_type_v<float2x4>  = ReflectionPrimitiveType::Float_2x4;
template <> static constexpr auto primitive_type_v<float3x2>  = ReflectionPrimitiveType::Float_3x2;
template <> static constexpr auto primitive_type_v<float3x3>  = ReflectionPrimitiveType::Float_3x3;
template <> static constexpr auto primitive_type_v<float3x4>  = ReflectionPrimitiveType::Float_3x4;
template <> static constexpr auto primitive_type_v<float4x2>  = ReflectionPrimitiveType::Float_4x2;
template <> static constexpr auto primitive_type_v<float4x3>  = ReflectionPrimitiveType::Float_4x3;
template <> static constexpr auto primitive_type_v<float4x4>  = ReflectionPrimitiveType::Float_4x4;
template <> static constexpr auto primitive_type_v<double2>   = ReflectionPrimitiveType::Double_2;
template <> static constexpr auto primitive_type_v<double3>   = ReflectionPrimitiveType::Double_3;
template <> static constexpr auto primitive_type_v<double4>   = ReflectionPrimitiveType::Double_4;
template <> static constexpr auto primitive_type_v<double2x2> = ReflectionPrimitiveType::Double_2x2;
template <> static constexpr auto primitive_type_v<double2x3> = ReflectionPrimitiveType::Double_2x3;
template <> static constexpr auto primitive_type_v<double2x4> = ReflectionPrimitiveType::Double_2x4;
template <> static constexpr auto primitive_type_v<double3x2> = ReflectionPrimitiveType::Double_3x2;
template <> static constexpr auto primitive_type_v<double3x3> = ReflectionPrimitiveType::Double_3x3;
template <> static constexpr auto primitive_type_v<double3x4> = ReflectionPrimitiveType::Double_3x4;
template <> static constexpr auto primitive_type_v<double4x2> = ReflectionPrimitiveType::Double_4x2;
template <> static constexpr auto primitive_type_v<double4x3> = ReflectionPrimitiveType::Double_4x3;
template <> static constexpr auto primitive_type_v<double4x4> = ReflectionPrimitiveType::Double_4x4;

template <class ShaderStorageDataType>
ShaderStorageDataType& construct_storage_data(ShaderStorageResource& resource)
{
	auto* data = reinterpret_cast<ShaderStorageData*>(&resource.data);
	auto* ptr  = static_cast<ShaderStorageDataType*>(data);
	return *std::construct_at(ptr);
}

template <class ShaderStorageDataType>
void destroy_storage_data(ShaderStorageResource& resource)
{
	auto* data = reinterpret_cast<ShaderStorageData*>(&resource.data);
	auto* ptr  = static_cast<ShaderStorageDataType*>(data);
	std::destroy_at(ptr);
}

static uint32_t get_array_element_count(const ReflectionResourceArrayDesc* desc)
{
	return std::min(desc->elementCount, 1024u);
}

static uint32_t get_offset(const ReflectionDesc& desc)
{
	switch (desc.type) {
	case ReflectionType::Primitive: return static_cast<const ReflectionPrimitiveDesc*>(&desc)->offset;
	case ReflectionType::Array:     return static_cast<const ReflectionArrayDesc*>(&desc)->offset;
	case ReflectionType::Struct:    return static_cast<const ReflectionStructDesc*>(&desc)->offset;
	}
	return 0;
}

static TextureLayout get_default_texture_layout(TextureUsageFlags flags)
{
	if (flags.has(TextureUsageFlagBits::Sampled))
		return TextureLayout::ShaderReadOnlyOptimal;
	if (flags.has(TextureUsageFlagBits::Storage))
		return TextureLayout::ShaderReadOnlyOptimal;
	if (flags.has(TextureUsageFlagBits::ColorAttachment))
		return TextureLayout::ColorAttachmentOptimal;
	if (flags.has(TextureUsageFlagBits::DepthStencilAttachment))
		return TextureLayout::DepthStencilAttachmentOptimal;
	if (flags.has(TextureUsageFlagBits::InputAttachment))
		return TextureLayout::ShaderReadOnlyOptimal;

	VERA_ASSERT_MSG(false, "unsupported texture usage");
	return TextureLayout::Undefined;
}

template <class MemberDescType>
static int32_t find_member_index(const MemberDescType& desc, const char* name)
{
	int32_t lo = 0;
	int32_t hi = desc.nameMapCount - 1;

	while (lo <= hi) {
		int32_t mid  = lo + (hi - lo) / 2;
		auto&   item = desc.nameMaps[mid];
		int32_t cmp  = strcmp(item.name, name);

		if (cmp == 0) {
			return item.index;
		} else if (cmp < 0) {
			lo = mid + 1;
		} else /* 0 < cmp */ {
			hi = mid - 1;
		}
	}

	return -1;
}

ShaderVariable ShaderVariable::operator[](std::string_view name) VERA_NOEXCEPT
{
	switch (m_desc->type) {
	case ReflectionType::Struct: {
		const auto& struct_desc = static_cast<const ReflectionStructDesc&>(*m_desc);
		
		int32_t idx = find_member_index(struct_desc, name.data());

		VERA_ASSERT_MSG(0 <= idx, "member not found");

		auto& member_desc = struct_desc.members[idx];
		
		return ShaderVariable(
			m_storage,
			m_resource,
			member_desc,
			m_offset + get_offset(*member_desc)
		);
	}
	case ReflectionType::ResourceBlock: {
		const auto& block_desc = static_cast<const ReflectionResourceBlockDesc&>(*m_desc);

		int32_t idx = find_member_index(block_desc, name.data());

		VERA_ASSERT_MSG(0 <= idx, "member not found");

		auto& member_desc = block_desc.members[idx];

		return ShaderVariable(
			m_storage,
			m_resource,
			member_desc,
			m_offset + get_offset(*member_desc)
		);
	}
	case ReflectionType::PushConstant: {
		const auto& pc_desc = static_cast<const ReflectionPushConstantDesc&>(*m_desc);

		int32_t idx = find_member_index(pc_desc, name.data());

		VERA_ASSERT_MSG(0 <= idx, "member not found");

		auto& member_desc = pc_desc.members[idx];

		return ShaderVariable(
			m_storage,
			m_resource,
			member_desc,
			m_offset + get_offset(*member_desc)
		);
	}
	default:
		VERA_ASSERT_MSG(false, "variable has no member");
	}

	// to suppress warning
	return {};
}

ShaderVariable ShaderVariable::operator[](uint32_t idx) VERA_NOEXCEPT
{
	switch (m_desc->type) {
	case ReflectionType::Array: {
		const auto* array_desc = static_cast<const ReflectionArrayDesc*>(m_desc);

		VERA_ASSERT_MSG(idx < array_desc->elementCount, "array index out of range");

		return ShaderVariable(
			m_storage,
			m_resource,
			array_desc->element,
			m_offset + array_desc->stride * idx
		);
	}
	case ReflectionType::ResourceArray: {
		const auto* array_desc = static_cast<const ReflectionResourceArrayDesc*>(m_desc);

		VERA_ASSERT_MSG(idx < get_array_element_count(array_desc), "array index out of range");

		return ShaderVariable(
			m_storage,
			m_resource + idx,
			array_desc->element,
			m_offset
		);
	}
	default:
		VERA_ASSERT_MSG(false, "variable is not subscriptable");
	}

	// to suppress warning
	return {};
}

ShaderVariable::ShaderVariable(
	ref<ShaderStorage>     storage,
	ShaderStorageResource* resource,
	ReflectionDesc*        desc,
	uint32_t               offset
) :
	m_storage(std::move(storage)),
	m_resource(resource),
	m_desc(desc),
	m_offset(offset) {}

ShaderVariable ShaderVariable::at(std::string_view name)
{
	return {};
}

ShaderVariable ShaderVariable::at(uint32_t idx)
{
	return {};
}

void ShaderVariable::operator=(obj<Sampler> sampler)
{
	m_storage->setSampler(*this, sampler);
}

void ShaderVariable::operator=(obj<Texture> texture)
{
	auto layout = get_default_texture_layout(texture->getUsageFlags());
	auto obj    = TextureView::create(texture, TextureViewCreateInfo{
		.type   = TextureViewType::View2D,
		.format = texture->getFormat()
	});

	m_storage->setTextureView(*this, std::move(obj), layout);
}

void ShaderVariable::operator=(obj<TextureView> texture_view)
{
	m_storage->setTextureView(*this, texture_view);
}

void ShaderVariable::operator=(obj<BufferView> buffer_view)
{
	m_storage->setBufferView(*this, buffer_view);
}

void ShaderVariable::operator=(obj<Buffer> buffer)
{
	m_storage->setBuffer(*this, buffer);
}

void ShaderVariable::setSampler(obj<Sampler> sampler)
{
	m_storage->setSampler(*this, sampler);
}

void ShaderVariable::setTextureView(obj<TextureView> texture_view)
{
	m_storage->setTextureView(*this, texture_view);
}

void ShaderVariable::setTextureView(obj<TextureView> texture_view, TextureLayout texture_layout)
{
	m_storage->setTextureView(*this, texture_view, texture_layout);
}

void ShaderVariable::setBufferView(obj<BufferView> buffer_view)
{
	m_storage->setBufferView(*this, buffer_view);
}

void ShaderVariable::setBuffer(obj<Buffer> buffer)
{
	m_storage->setBuffer(*this, buffer);
}

static void reset_shader_storage_frame(
	ShaderStorageImpl&        impl,
	const ShaderStorageFrame* prev_frame,
	ShaderStorageFrame&       out_frame
) {
	out_frame.commandBuffer     = {};
	out_frame.commandBufferSync = {};
	out_frame.samplers          = {};
	out_frame.textureViews      = {};
	out_frame.bufferViews       = {};
	out_frame.buffers           = {};

	for (auto frame_set : out_frame.frameSets) {
		frame_set.subFrameIndex = 0;

		if (frame_set.subFrames.size() == 1) continue;
	
		auto&  first_frame = frame_set.subFrames.front();
		auto&  set         = impl.sets[frame_set.set];
		size_t res_offset  = impl.setResourceRanges[frame_set.set].first();

		for (size_t i = 0; i < first_frame.lastSubFrameIDs.size(); ++i) {
			const auto& resource = impl.resources[res_offset + i];
			
			if (first_frame.lastSubFrameIDs[i] < resource.lastSubFrameID) {
				auto& binding = set.bindings[resource.bindingIndex];
				binding.needUpdate = true;
				set.needUpdate     = true;
				
				// TODO: optimize range insertion
				if (resource.arrayIndex != UINT32_MAX)
					binding.arrayUpdateRange.insert(resource.arrayIndex);
			}
		}
	}
}

static uint32_t get_variable_descriptor_count(const ShaderStorageImpl& impl, uint32_t set)
{
	auto& reflection = impl.reflection;

	auto** first = reflection.reflections;
	auto** last  = first + reflection.resourceCount;

	auto it = std::prev(std::upper_bound(first, last, set,
		[](uint32_t set, const ReflectionRootMemberDesc* desc) {
			return set < static_cast<const ReflectionResourceDesc*>(desc)->set;
		}));

	auto* res_desc = static_cast<const ReflectionResourceDesc*>(*it);

	VERA_ASSERT(res_desc->set == set);

	if (res_desc->type != ReflectionType::ResourceArray)
		return 1;

	return get_array_element_count(static_cast<const ReflectionResourceArrayDesc*>(*it));
}

static void update_all_set(ShaderStorageImpl& impl)
{
	for (auto& set : impl.sets) {
		for (auto& binding : set.bindings) {
			uint32_t array_count     = static_cast<uint32_t>(binding.resourceRange.size());
			binding.arrayUpdateRange = basic_range(0u, array_count);
			binding.needUpdate       = true;
		}

		set.needUpdate = true;
	}
}

static void append_shader_storage_frame(
	ShaderStorageImpl&        impl,
	const ShaderStorageFrame* prev_frame,
	uint32_t                  at
) {
	auto& new_frame = *impl.frames.emplace(impl.frames.cbegin() + at);

	new_frame.descriptorPool = DescriptorPool::create(impl.device);

	new_frame.frameSets.resize(impl.reflection.setRangeCount);
	for (uint32_t i = 0; i < new_frame.frameSets.size(); ++i) {
		auto& set = new_frame.frameSets[i];
		set.set                = i;
		set.variableArrayCount = get_variable_descriptor_count(impl, i);
		set.subFrameIndex      = 0;

		auto  set_range  = impl.setResourceRanges[i];
		auto  set_layout = impl.pipelineLayout->getDescriptorSetLayout(i);
		auto& subframe   = set.subFrames.emplace_back();

		subframe.descriptorSet = new_frame.descriptorPool->
			allocate(set_layout, set.variableArrayCount);
		subframe.lastSubFrameIDs.resize(set_range.size(), 0);
	}

	update_all_set(impl);
}

static void next_frame(ShaderStorageImpl& impl)
{
	impl.frameID      += 1;
	impl.subFrameIndex = 0;
	
	auto& curr_frame = impl.frames[impl.frameIndex];
	auto& curr_sync  = curr_frame.commandBufferSync;

	if (curr_sync.empty()) return;

	if (curr_sync.isComplete()) {
		reset_shader_storage_frame(impl, nullptr, curr_frame);
	} else if (impl.frames.size() == 1) {
		append_shader_storage_frame(impl, &curr_frame, 1);
		impl.prevFrameIndex = 0;
		impl.frameIndex     = 1;
	} else {
		auto  next_idx   = (impl.frameIndex + 1) % static_cast<uint32_t>(impl.frames.size());
		auto& next_frame = impl.frames[next_idx];
		auto& next_sync  = next_frame.commandBufferSync;

		if (next_sync.isComplete()) {
			reset_shader_storage_frame(impl, &curr_frame, next_frame);
		} else {
			append_shader_storage_frame(impl, &curr_frame, next_idx);
		}

		impl.prevFrameIndex = std::exchange(impl.frameIndex, next_idx);
	}
}

static void prepare_frame(ShaderStorageImpl& impl)
{
	// TODO: optimize next frame checking !!!!!
	if (!impl.currentSync.empty() &&
		CommandBufferState::Executable <= impl.currentSync.getState())
		next_frame(impl);
}

static void backup_resource_prev_frame(ShaderStorageImpl& impl, ShaderStorageResource& resource)
{
	if (impl.prevFrameIndex == 0 || impl.frames.size() == 1) return;

	auto& prev_frame = impl.frames[impl.prevFrameIndex];

	if (prev_frame.commandBufferSync.isComplete()) return;

	switch (resource.resourceDesc->descriptorType) {
	case DescriptorType::Sampler: {
		auto& image_data = get_storage_data<ShaderStorageImageData>(resource);
		if (image_data.sampler.count() == 1)
			prev_frame.samplers.push_back(image_data.sampler);
	} break;
	case DescriptorType::CombinedImageSampler: {
		auto& image_data = get_storage_data<ShaderStorageImageData>(resource);
		if (image_data.sampler.count() == 1)
			prev_frame.samplers.push_back(image_data.sampler);
		if (image_data.textureView.count() == 1)
			prev_frame.textureViews.push_back(image_data.textureView);
	} break;
	case DescriptorType::SampledImage:
	case DescriptorType::StorageImage:
	case DescriptorType::InputAttachment: {
		auto& image_data = get_storage_data<ShaderStorageImageData>(resource);
		if (image_data.sampler.count() == 1)
			prev_frame.textureViews.push_back(image_data.textureView);
	} break;
	case DescriptorType::UniformTexelBuffer:
	case DescriptorType::StorageTexelBuffer: {
		auto& view_data = get_storage_data<ShaderStorageBufferViewData>(resource);
		if (view_data.bufferView.count() == 1)
			prev_frame.bufferViews.push_back(view_data.bufferView);
	} break;
	case DescriptorType::UniformBuffer:
	case DescriptorType::UniformBufferDynamic:
	case DescriptorType::StorageBuffer:
	case DescriptorType::StorageBufferDynamic: {
		if (resource.resourceDesc->type == ReflectionType::ResourceBlock) {
			auto& block_data = get_storage_data<ShaderStorageBufferBlockData>(resource);
			if (block_data.buffer.count() == 1)
				prev_frame.buffers.push_back(block_data.buffer);
		} else /* resource.resourceDesc->type == ReflectionType::Resource */ {
			VERA_ASSERT(resource.resourceDesc->type == ReflectionType::Resource);

			auto& buffer_data = get_storage_data<ShaderStorageBufferData>(resource);
			if (buffer_data.buffer.count() == 1)
				prev_frame.buffers.push_back(buffer_data.buffer);
		}
	} break;
	}
}

static void prepare_resource_write(ShaderStorageImpl& impl, ShaderStorageResource& resource)
{
	if (resource.lastSubFrameID == impl.subFrameID) return;

	auto& set     = impl.sets[resource.set];
	auto& binding = set.bindings[resource.bindingIndex];

	set.needUpdate     = true;
	binding.needUpdate = true;

	if (resource.arrayIndex != UINT32_MAX)
		binding.arrayUpdateRange.insert(resource.arrayIndex);
	
	backup_resource_prev_frame(impl, resource);

	resource.lastSubFrameID = impl.subFrameID;
}

template <class T>
static void store_scalar_impl(
	ShaderStorageImpl&     impl,
	ShaderStorageResource* resource,
	const ReflectionDesc*  desc,
	uint32_t               offset,
	const T&               value
) {
	VERA_ASSERT_MSG(desc->type == ReflectionType::Primitive, "invalid resource type");

	prepare_frame(impl);

	if (resource->set != UINT32_MAX) // push constant don't need to be prepared
		prepare_resource_write(impl, *resource);

	auto& data     = get_storage_data<ShaderStorageBlockData>(*resource);
	auto* data_ptr = data.block.data() + offset;

	// automatic type casting
	switch (static_cast<const ReflectionPrimitiveDesc*>(desc)->primitiveType) {
	case ReflectionPrimitiveType::Bool:
		*reinterpret_cast<bool*>(data_ptr) = static_cast<bool>(value);
		break;
	case ReflectionPrimitiveType::Int8:
		*reinterpret_cast<int8_t*>(data_ptr) = static_cast<int8_t>(value);
		break;
	case ReflectionPrimitiveType::UInt8:
		*reinterpret_cast<uint8_t*>(data_ptr) = static_cast<uint8_t>(value);
		break;
	case ReflectionPrimitiveType::Int16:
		*reinterpret_cast<int16_t*>(data_ptr) = static_cast<int16_t>(value);
		break;
	case ReflectionPrimitiveType::UInt16:
		*reinterpret_cast<uint16_t*>(data_ptr) = static_cast<uint16_t>(value);
		break;
	case ReflectionPrimitiveType::Int32:
		*reinterpret_cast<int32_t*>(data_ptr) = static_cast<int32_t>(value);
		break;
	case ReflectionPrimitiveType::UInt32:
		*reinterpret_cast<uint32_t*>(data_ptr) = static_cast<uint32_t>(value);
		break;
	case ReflectionPrimitiveType::Int64:
		*reinterpret_cast<int64_t*>(data_ptr) = static_cast<int64_t>(value);
		break;
	case ReflectionPrimitiveType::UInt64:
		*reinterpret_cast<uint64_t*>(data_ptr) = static_cast<uint64_t>(value);
		break;
	case ReflectionPrimitiveType::Float:
		*reinterpret_cast<float*>(data_ptr) = static_cast<float>(value);
		break;
	case ReflectionPrimitiveType::Double:
		*reinterpret_cast<double*>(data_ptr) = static_cast<double>(value);
		break;
	}
}

template <class T>
static void store_vector_impl(
	ShaderStorageImpl&     impl,
	ShaderStorageResource* resource,
	const ReflectionDesc*  desc,
	uint32_t               offset,
	const T&               value
) {
#ifdef _DEBUG
	const auto& prim_desc = static_cast<const ReflectionPrimitiveDesc&>(*desc);
	VERA_ASSERT_MSG(desc->type == ReflectionType::Primitive, "invalid resource type");
	VERA_ASSERT_MSG(prim_desc.primitiveType == primitive_type_v<T>, "invalid primitive type");
#endif

	prepare_frame(impl);

	if (resource->set != UINT32_MAX) // push constant don't need to be prepared
		prepare_resource_write(impl, *resource);

	auto& data     = get_storage_data<ShaderStorageBlockData>(*resource);
	auto* data_ptr = data.block.data() + offset;

	*reinterpret_cast<T*>(data_ptr) = value;
}

template <class T>
static void store_matrix_impl(
	ShaderStorageImpl&     impl,
	ShaderStorageResource* resource,
	const ReflectionDesc*  desc,
	uint32_t               offset,
	const T&               value
) {
#ifdef _DEBUG
	const auto& prim_desc = static_cast<const ReflectionPrimitiveDesc&>(*desc);
	VERA_ASSERT_MSG(desc->type == ReflectionType::Primitive, "invalid resource type");
	VERA_ASSERT_MSG(prim_desc.primitiveType == primitive_type_v<T>, "invalid primitive type");
#endif

	prepare_frame(impl);

	if (resource->set != UINT32_MAX) // push constant don't need to be prepared
		prepare_resource_write(impl, *resource);

	auto& data     = get_storage_data<ShaderStorageBlockData>(*resource);
	auto* data_ptr = data.block.data();

	// GLSL and some ::vr::matrix_base<> type have different matrix layout
	if constexpr (primitive_type_v<T> == ReflectionPrimitiveType::Float_2x3) {
		*reinterpret_cast<float3*>(&data_ptr[offset + 0])  = value[0];
		*reinterpret_cast<float3*>(&data_ptr[offset + 16]) = value[1];
	} else if constexpr (primitive_type_v<T> == ReflectionPrimitiveType::Float_3x3) {
		*reinterpret_cast<float3*>(&data_ptr[offset + 0])  = value[0];
		*reinterpret_cast<float3*>(&data_ptr[offset + 16]) = value[1];
		*reinterpret_cast<float3*>(&data_ptr[offset + 32]) = value[2];
	} else if constexpr (primitive_type_v<T> == ReflectionPrimitiveType::Float_4x3) {
		*reinterpret_cast<float3*>(&data_ptr[offset + 0])  = value[0];
		*reinterpret_cast<float3*>(&data_ptr[offset + 16]) = value[1];
		*reinterpret_cast<float3*>(&data_ptr[offset + 32]) = value[2];
		*reinterpret_cast<float3*>(&data_ptr[offset + 48]) = value[3];
	} else if constexpr (primitive_type_v<T> == ReflectionPrimitiveType::Double_2x3) {
		*reinterpret_cast<double3*>(&data_ptr[offset + 0])  = value[0];
		*reinterpret_cast<double3*>(&data_ptr[offset + 16]) = value[1];
	} else if constexpr (primitive_type_v<T> == ReflectionPrimitiveType::Double_3x3) {
		*reinterpret_cast<double3*>(&data_ptr[offset + 0])  = value[0];
		*reinterpret_cast<double3*>(&data_ptr[offset + 16]) = value[1];
		*reinterpret_cast<double3*>(&data_ptr[offset + 32]) = value[2];
	} else if constexpr (primitive_type_v<T> == ReflectionPrimitiveType::Double_4x3) {
		*reinterpret_cast<double3*>(&data_ptr[offset + 0])  = value[0];
		*reinterpret_cast<double3*>(&data_ptr[offset + 16]) = value[1];
		*reinterpret_cast<double3*>(&data_ptr[offset + 32]) = value[2];
		*reinterpret_cast<double3*>(&data_ptr[offset + 48]) = value[3];
	} else {
		*reinterpret_cast<T*>(&data_ptr[offset]) = value;
	}
}

template <class NameMapType>
static bool check_name_map_collision(const NameMapType& name_map_type)
{
	if (name_map_type.nameMapCount <= 1) return true;

	for (uint32_t i = 1; i < name_map_type.nameMapCount; ++i) {
		auto& prev = name_map_type.nameMaps[i - 1];
		auto& curr = name_map_type.nameMaps[i];

		if (strcmp(prev.name, curr.name) == 0)
			throw Exception("pipeline layout has duplicated variable name '{}'", prev.name);
	}

	return true;
}

static void check_reflection_name_map(const ReflectionDesc* desc)
{
	VERA_ASSERT(desc);

	switch (desc->type) {
	case ReflectionType::Primitive: {
		// nothing to check
	} break;
	case ReflectionType::Array: {
		const auto* array_desc = static_cast<const ReflectionArrayDesc*>(desc);
		check_reflection_name_map(array_desc->element);
	} break;
	case ReflectionType::Struct: {
		const auto* struct_desc = static_cast<const ReflectionStructDesc*>(desc);
		check_name_map_collision(*struct_desc);
		for (uint32_t i = 0; i < struct_desc->memberCount; ++i)
			check_reflection_name_map(struct_desc->members[i]);
	} break;
	case ReflectionType::Resource: {
		// nothing to check
	} break;
	case ReflectionType::ResourceBlock: {
		const auto* struct_desc = static_cast<const ReflectionResourceBlockDesc*>(desc);
		check_name_map_collision(*struct_desc);
		for (uint32_t i = 0; i < struct_desc->memberCount; ++i)
			check_reflection_name_map(struct_desc->members[i]);
	} break;
	case ReflectionType::ResourceArray: {
		const auto* array_desc = static_cast<const ReflectionResourceArrayDesc*>(desc);
		check_reflection_name_map(array_desc->element);
	} break;
	case ReflectionType::PushConstant: {
		const auto* struct_desc = static_cast<const ReflectionPushConstantDesc*>(desc);
		check_name_map_collision(*struct_desc);
		for (uint32_t i = 0; i < struct_desc->memberCount; ++i)
			check_reflection_name_map(struct_desc->members[i]);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid reflection type");
	}
}

static void check_pipeline_layout(PipelineLayoutImpl& impl)
{
	const auto& reflection = impl.reflection;

	if (reflection.reflectionCount == 0)
		throw Exception("pipeline layout has no reflection");
	if (reflection.nameMapCount == 0)
		throw Exception("pipeline layout has no name data, try create pipeline layout directly from shaders");

	check_name_map_collision(reflection);

	for (uint32_t i = 0; i < reflection.reflectionCount; ++i)
		check_reflection_name_map(reflection.reflections[i]);
}

static void append_shader_storage_resource_data(
	ShaderStorageImpl&            impl,
	const ReflectionResourceDesc* desc,
	uint32_t                      binding_idx,
	uint32_t                      array_idx
) {
	auto& data = impl.resources.emplace_back();

	switch (desc->descriptorType) {
	case DescriptorType::Sampler:
	case DescriptorType::CombinedImageSampler: {
		auto& image_data = construct_storage_data<ShaderStorageImageData>(data);
		image_data.sampler    = impl.device->getDefaultSampler();
	} break;
	case DescriptorType::SampledImage:
	case DescriptorType::StorageImage: {
		construct_storage_data<ShaderStorageImageData>(data);
	} break;
	case DescriptorType::UniformTexelBuffer:
	case DescriptorType::StorageTexelBuffer: {
		construct_storage_data<ShaderStorageBufferViewData>(data);
	} break;
	case DescriptorType::UniformBuffer:
	case DescriptorType::UniformBufferDynamic:
	case DescriptorType::StorageBuffer:
	case DescriptorType::StorageBufferDynamic: {
		construct_storage_data<ShaderStorageBufferData>(data);
	} break;
	case DescriptorType::InputAttachment: {
		construct_storage_data<ShaderStorageImageData>(data);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource descriptor type");
	}

	data.resourceDesc   = desc;
	data.resourceID     = static_cast<uint32_t>(impl.resources.size() - 1);
	data.set            = desc->set;
	data.bindingIndex   = desc->binding;
	data.arrayIndex     = array_idx;
	data.lastSubFrameID = 0;
}

static void append_shader_storage_block_data(
	ShaderStorageImpl&                 impl,
	const ReflectionResourceBlockDesc* desc,
	uint32_t                           binding_idx,
	uint32_t                           array_idx
) {
	auto& data = impl.resources.emplace_back();

	switch (desc->descriptorType) {
	case DescriptorType::UniformBuffer:
	case DescriptorType::UniformBufferDynamic:
	case DescriptorType::StorageBuffer:
	case DescriptorType::StorageBufferDynamic: {
		auto& block_data = construct_storage_data<ShaderStorageBufferBlockData>(data);
		block_data.block.resize(desc->sizeInByte);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource block descriptor type");
	}

	data.resourceDesc   = desc;
	data.resourceID     = static_cast<uint32_t>(impl.resources.size() - 1);
	data.set            = desc->set;
	data.bindingIndex   = binding_idx;
	data.arrayIndex     = array_idx;
	data.lastSubFrameID = 0;
}

static void append_shader_storage_pc_data(ShaderStorageImpl& impl, const ReflectionPushConstantDesc* desc)
{
	VERA_ASSERT_MSG(desc->type == ReflectionType::PushConstant, "invalid push constant type");

	auto& data    = impl.resources.emplace_back();
	data.resourceDesc   = nullptr;
	data.resourceID     = static_cast<uint32_t>(impl.resources.size() - 1);
	data.set            = UINT32_MAX;
	data.bindingIndex   = UINT32_MAX;
	data.arrayIndex     = UINT32_MAX;
	data.lastSubFrameID = 0;

	auto& pc_data = construct_storage_data<ShaderStoragePushConstantData>(data);
	pc_data.stageFlags = desc->stageFlags;
	pc_data.block.resize(desc->sizeInByte);
}

static void append_shader_storage_resource_data(
	ShaderStorageImpl&              impl,
	const ReflectionRootMemberDesc* desc,
	uint32_t                        binding_idx
) {
	uint32_t resource_offset = static_cast<uint32_t>(impl.resources.size());

	switch (desc->type) {
	case ReflectionType::Resource: {
		const auto* res_desc = static_cast<const ReflectionResourceDesc*>(desc);
		append_shader_storage_resource_data(impl, res_desc, binding_idx, UINT32_MAX);
	} break;
	case ReflectionType::ResourceBlock: {
		const auto* block_desc = static_cast<const ReflectionResourceBlockDesc*>(desc);
		append_shader_storage_block_data(impl, block_desc, binding_idx, UINT32_MAX);
	} break;
	case ReflectionType::ResourceArray: {
		const auto* array_desc    = static_cast<const ReflectionResourceArrayDesc*>(desc);
		uint32_t    element_count = get_array_element_count(array_desc);

		if (array_desc->element->type == ReflectionType::Resource) {
			const auto* res_desc = static_cast<const ReflectionResourceDesc*>(array_desc->element);
			for (uint32_t i = 0; i < element_count; ++i)
				append_shader_storage_resource_data(impl, res_desc, binding_idx, i);
		} else if (array_desc->element->type == ReflectionType::ResourceBlock) {
			const auto* block_desc = static_cast<const ReflectionResourceBlockDesc*>(array_desc->element);
			for (uint32_t i = 0; i < element_count; ++i)
				append_shader_storage_block_data(impl, block_desc, binding_idx, i);
		} else {
			VERA_ASSERT_MSG(false, "invalid resource array element type");
		}
	} break;
	case ReflectionType::PushConstant: {
		const auto* pc_desc = static_cast<const ReflectionPushConstantDesc*>(desc);
		append_shader_storage_pc_data(impl, pc_desc);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource type");
	}

	impl.resourceRanges.emplace_back(resource_offset, static_cast<uint32_t>(impl.resources.size()));
}

static void create_shader_storage_resource_data(ShaderStorageImpl& impl)
{
	auto& layout_impl = CoreObject::getImpl(impl.pipelineLayout);

	impl.setResourceRanges.resize(impl.reflection.setRangeCount);

	uint32_t res_first        = 0;
	uint32_t res_last         = 0;
	uint32_t set_id           = 0;
	uint32_t binding_idx      = 0;

	for (uint32_t i = 0; i < impl.reflection.resourceCount; ++i) {
		const auto* res_desc = static_cast<const ReflectionResourceDesc*>(impl.reflection.reflections[i]);
		
		if (res_desc->set != set_id)
			binding_idx = 0;

		res_last = static_cast<uint32_t>(impl.resources.size());
		append_shader_storage_resource_data(impl, res_desc, binding_idx++);

		if (res_desc->set != set_id) {
			impl.setResourceRanges[set_id] = basic_range(res_first, res_last);
			res_first                      = res_last;
			set_id                         = res_desc->set;
		}
	}

	if (!impl.setResourceRanges.empty()) {
		res_last               = static_cast<uint32_t>(impl.resources.size());
		impl.setResourceRanges[set_id] = basic_range(res_first, res_last);
	}

	uint32_t pc_first = static_cast<uint32_t>(impl.resources.size());
	uint32_t pc_last  = pc_first + impl.reflection.reflectionCount - impl.reflection.resourceCount;

	for (uint32_t i = impl.reflection.resourceCount; i < impl.reflection.reflectionCount; ++i)
		append_shader_storage_resource_data(impl, impl.reflection.reflections[i], UINT32_MAX);

	impl.pcRange = basic_range(pc_first, pc_last);
}

static const ReflectionResourceDesc* find_first_binding(const ShaderReflection& reflection, uint32_t set)
{
	const auto* first = reflection.reflections;
	const auto* last  = first + reflection.resourceCount;

	const auto* ptr = std::lower_bound(first, last, set,
		[](const ReflectionRootMemberDesc* desc, uint32_t set) {
			return static_cast<const ReflectionResourceDesc*>(desc)->set < set;
		});

	return ptr == last ? nullptr : static_cast<const ReflectionResourceDesc*>(*ptr);
}

static void create_shader_storage_set(ShaderStorageImpl& impl)
{
	const auto&    layout_impl = CoreObject::getImpl(impl.pipelineLayout);
	const uint32_t set_count   = impl.reflection.setRangeCount;

	impl.sets.resize(set_count);

	for (uint32_t set_id = 0; set_id < set_count; ++set_id) {
		auto& set       = impl.sets[set_id];
		auto  set_range = layout_impl.reflection.setRanges[set_id];

		set.bindings.resize(set_range.size());
		for (uint32_t binding_idx = 0; binding_idx < set.bindings.size(); ++binding_idx) {
			auto& binding    = set.bindings[binding_idx];
			auto  res_idx    = set_range.first() + binding_idx;
			auto& reflection = impl.reflection.reflections[res_idx];
			auto& res_desc   = static_cast<const ReflectionResourceDesc&>(*reflection);
			auto& res_range  = impl.setResourceRanges[set_id];

			binding.reflectionType    = res_desc.type;
			binding.descriptorType    = res_desc.descriptorType;
			binding.set               = res_desc.set;
			binding.binding           = res_desc.binding;
			binding.bindingIndex      = binding_idx;
			binding.setResourceOffset = res_range.first() - set_range.first();
			binding.resourceRange     = res_range;
			binding.arrayUpdateRange  = {};
			binding.needUpdate        = false;
		}

		set.set            = set_id;
		set.lastSubFrameID = 0;
		set.needUpdate     = false;
	}
}

static void destroy_shader_storage_resource_data(
	ShaderStorageImpl&            impl,
	const ReflectionResourceDesc* desc,
	uint32_t&                     res_id
) {
	switch (desc->descriptorType) {
	case DescriptorType::Sampler:
	case DescriptorType::CombinedImageSampler:
	case DescriptorType::SampledImage:
	case DescriptorType::StorageImage:
		destroy_storage_data<ShaderStorageImageData>(impl.resources[res_id++]);
		break;
	case DescriptorType::UniformTexelBuffer:
	case DescriptorType::StorageTexelBuffer:
		destroy_storage_data<ShaderStorageBufferViewData>(impl.resources[res_id++]);
		break;
	case DescriptorType::UniformBuffer:
	case DescriptorType::UniformBufferDynamic:
	case DescriptorType::StorageBuffer:
	case DescriptorType::StorageBufferDynamic:
		destroy_storage_data<ShaderStorageBufferData>(impl.resources[res_id++]);
		break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource descriptor type");
	}
}

static void destroy_shader_storage_block_data(
	ShaderStorageImpl&                 impl,
	const ReflectionResourceBlockDesc* desc,
	uint32_t&                          res_id
) {
	switch (desc->descriptorType) {
	case DescriptorType::UniformBuffer:
	case DescriptorType::UniformBufferDynamic:
	case DescriptorType::StorageBuffer:
	case DescriptorType::StorageBufferDynamic:
		destroy_storage_data<ShaderStorageBufferBlockData>(impl.resources[res_id++]);
		break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource block descriptor type");
	}
}

static void destroy_shader_storage_array_data(
	ShaderStorageImpl&            impl,
	const ReflectionResourceArrayDesc* desc,
	uint32_t&                     res_id
) {
	uint32_t element_count = get_array_element_count(desc);

	switch (desc->element->type) {
	case ReflectionType::Resource: {
		const auto* res_desc = static_cast<const ReflectionResourceDesc*>(desc->element);
		for (uint32_t j = 0; j < element_count; ++j)
			destroy_shader_storage_resource_data(impl, res_desc, res_id);
	} break;
	case ReflectionType::ResourceArray: {
		const auto* block_desc = static_cast<const ReflectionResourceBlockDesc*>(desc->element);
		for (uint32_t j = 0; j < element_count; ++j)
			destroy_shader_storage_block_data(impl, block_desc, res_id);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource array element type");
	}
}

static void destroy_shader_storage_resource_data(ShaderStorageImpl& impl)
{
	uint32_t res_id = 0;

	for (uint32_t i = 0; i < impl.reflection.reflectionCount; ++i) {
		const auto* reflection = impl.reflection.reflections[i];

		switch (reflection->type) {
		case ReflectionType::Resource: {
			const auto* res_desc = static_cast<const ReflectionResourceDesc*>(reflection);
			destroy_shader_storage_resource_data(impl, res_desc, res_id);
		} break;
		case ReflectionType::ResourceBlock: {
			const auto* block_desc = static_cast<const ReflectionResourceBlockDesc*>(reflection);
			destroy_shader_storage_block_data(impl, block_desc, res_id);
		} break;
		case ReflectionType::ResourceArray: {
			const auto* array_desc = static_cast<const ReflectionResourceArrayDesc*>(reflection);
			destroy_shader_storage_array_data(impl, array_desc, res_id);
		} break;
		case ReflectionType::PushConstant: {
			destroy_storage_data<ShaderStoragePushConstantData>(impl.resources[res_id++]);
		} break;
		}
	}

	impl.resources           = {};
	impl.resourceRanges      = {};
	impl.setResourceRanges   = {};
	impl.pcRange             = {};
}

obj<ShaderStorage> ShaderStorage::create(obj<PipelineLayout> pipeline_layout)
{
	VERA_ASSERT_MSG(pipeline_layout, "empty pipeline layout");

	auto  obj         = createNewCoreObject<ShaderStorage>();
	auto& impl        = getImpl(obj);
	auto& layout_impl = getImpl(pipeline_layout);

	check_pipeline_layout(layout_impl);

	impl.device            = layout_impl.device;
	impl.pipelineLayout    = std::move(pipeline_layout);
	impl.reflection        = layout_impl.reflection;
	impl.pipelineBindPoint = layout_impl.pipelineBindPoint;
	impl.currentSync       = {};
	impl.frameIndex        = 0;
	impl.prevFrameIndex    = UINT32_MAX;
	impl.frameID           = 1;
	impl.subFrameIndex     = 0;
	impl.subFrameID        = 1;

	create_shader_storage_resource_data(impl);
	create_shader_storage_set(impl);

	append_shader_storage_frame(impl, nullptr, 0);

	return obj;
}

ShaderStorage::~ShaderStorage()
{
	auto& impl = getImpl(this);

	destroy_shader_storage_resource_data(impl);

	destroyObjectImpl(this);
}

obj<Device> ShaderStorage::getDevice() VERA_NOEXCEPT
{
	return getImpl(this).device;
}

obj<PipelineLayout> ShaderStorage::getPipelineLayout() VERA_NOEXCEPT
{
	return getImpl(this).pipelineLayout;
}

bool ShaderStorage::hasVariable(std::string_view name) VERA_NOEXCEPT
{
	auto& impl = getImpl(this);
	return 0 <= find_member_index(impl.reflection, name.data());
}

ShaderVariable ShaderStorage::accessVariable(std::string_view name)
{
	auto& impl = getImpl(this);

	int32_t idx = find_member_index(impl.reflection, name.data());

	VERA_ASSERT_MSG(0 <= idx, "variable not found");

	auto& var_desc = impl.reflection.reflections[idx];
	auto& resource = impl.resources[impl.resourceRanges[idx].first()];

	return ShaderVariable(
		ref<ShaderStorage>(this),
		&resource,
		var_desc,
		0);
}

void ShaderStorage::setSampler(const ShaderVariable& variable, obj<Sampler> sampler)
{

}

void ShaderStorage::setTextureView(const ShaderVariable& variable, obj<TextureView> texture_view)
{
	auto usage_flags = texture_view->getTexture()->getUsageFlags();
	setTextureView(variable, texture_view, get_default_texture_layout(usage_flags));
}

void ShaderStorage::setTextureView(const ShaderVariable& variable, obj<TextureView> texture_view, TextureLayout texture_layout)
{
#ifdef _DEBUG
	auto& res_desc = static_cast<const ReflectionResourceDesc&>(*variable.m_desc);
	VERA_ASSERT(variable.m_desc->type == ReflectionType::Resource);
	VERA_ASSERT_MSG(res_desc.descriptorType == DescriptorType::CombinedImageSampler ||
					res_desc.descriptorType == DescriptorType::SampledImage ||
					res_desc.descriptorType == DescriptorType::StorageImage ||
					res_desc.descriptorType == DescriptorType::InputAttachment,
					"variable is not a texture");
#endif
	prepare_frame(getImpl(this));
	prepare_resource_write(getImpl(this), *variable.m_resource);

	auto& image_data = get_storage_data<ShaderStorageImageData&>(*variable.m_resource);
	image_data.textureView   = texture_view;
	image_data.textureLayout = texture_layout;
}

void ShaderStorage::setBufferView(const ShaderVariable& variable, obj<BufferView> buffer_view)
{

}

void ShaderStorage::setBuffer(const ShaderVariable& variable, obj<Buffer> buffer, size_t offset, size_t range)
{

}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const bool value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const int8_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uint8_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const int16_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uint16_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const int32_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uint32_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const int64_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uint64_t value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double value)
{
	store_scalar_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const bool2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const bool3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const bool4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const char2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const char3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const char4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uchar2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uchar3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uchar4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const short2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const short3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const short4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const ushort2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const ushort3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const ushort4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const int2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const int3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const int4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uint2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uint3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const uint4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const long2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const long3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const long4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const ulong2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const ulong3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const ulong4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double2& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double3& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double4& value)
{
	store_vector_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float2x2& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float2x3& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float2x4& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float3x2& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float3x3& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float3x4& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float4x2& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float4x3& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const float4x4& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double2x2& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double2x3& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double2x4& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double3x2& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double3x3& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double3x4& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double4x2& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double4x3& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const double4x4& value)
{
	store_matrix_impl(getImpl(this), variable.m_resource, variable.m_desc, variable.m_offset, value);
}

VERA_NAMESPACE_END
