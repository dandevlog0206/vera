#include "../../include/vera/core/shader_storage.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/pipeline_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/resource_layout.h"
#include "../../include/vera/core/resource_binding_pool.h"
#include "../../include/vera/core/sampler.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/core/buffer_view.h"
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









//template <class T>
//static void store_scalar_impl(ShaderStorageData* storage_ptr, ReflectionDesc* desc_ptr, uint32_t offset, const T& value)
//{
//	VERA_ASSERT(desc_ptr && desc_ptr->type == ReflectionType::Primitive);
//	VERA_ASSERT(storage_ptr && (storage_ptr->storageType == ShaderStorageDataType::BufferBlock ||
//		storage_ptr->storageType == ShaderStorageDataType::PushConstant));
//
//	auto& desc    = *static_cast<ReflectionPrimitiveDesc*>(desc_ptr);
//	auto& storage = *static_cast<BlockStorage*>(storage_ptr);
//	auto* data    = storage.blockStorage.data() + offset;
//
//	// automatic type casting
//	switch (desc.primitiveType) {
//	case ReflectionPrimitiveType::Bool:
//		*reinterpret_cast<bool*>(data) = static_cast<bool>(value);
//		break;
//	case ReflectionPrimitiveType::Int8:
//		*reinterpret_cast<int8_t*>(data) = static_cast<int8_t>(value);
//		break;
//	case ReflectionPrimitiveType::UInt8:
//		*reinterpret_cast<uint8_t*>(data) = static_cast<uint8_t>(value);
//		break;
//	case ReflectionPrimitiveType::Int16:
//		*reinterpret_cast<int16_t*>(data) = static_cast<int16_t>(value);
//		break;
//	case ReflectionPrimitiveType::UInt16:
//		*reinterpret_cast<uint16_t*>(data) = static_cast<uint16_t>(value);
//		break;
//	case ReflectionPrimitiveType::Int32:
//		*reinterpret_cast<int32_t*>(data) = static_cast<int32_t>(value);
//		break;
//	case ReflectionPrimitiveType::UInt32:
//		*reinterpret_cast<uint32_t*>(data) = static_cast<uint32_t>(value);
//		break;
//	case ReflectionPrimitiveType::Int64:
//		*reinterpret_cast<int64_t*>(data) = static_cast<int64_t>(value);
//		break;
//	case ReflectionPrimitiveType::UInt64:
//		*reinterpret_cast<uint64_t*>(data) = static_cast<uint64_t>(value);
//		break;
//	case ReflectionPrimitiveType::Float:
//		*reinterpret_cast<float*>(data) = static_cast<float>(value);
//		break;
//	case ReflectionPrimitiveType::Double:
//		*reinterpret_cast<double*>(data) = static_cast<double>(value);
//		break;
//	}
//}
//
//template <class Type>
//static void store_primitive_impl(ShaderStorageData* storage_ptr, ReflectionDesc* desc_ptr, uint32_t offset, const Type& value)
//{
//	VERA_ASSERT(desc_ptr && desc_ptr->type == ReflectionType::Primitive);
//	VERA_ASSERT(storage_ptr->storageType == ShaderStorageDataType::BufferBlock ||
//		storage_ptr->storageType == ShaderStorageDataType::PushConstant);
//
//	auto& desc    = *static_cast<ReflectionPrimitiveDesc*>(desc_ptr);
//	auto& storage = *static_cast<BlockStorage*>(storage_ptr);
//
//	VERA_ASSERT(desc.primitiveType == primitive_type_v<Type>);
//
//	// GLSL and GLM have different matrix layout
//	if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Float_2x3) {
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 0])  = value[0];
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 16]) = value[1];
//	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Float_3x3) {
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 0])  = value[0];
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 16]) = value[1];
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 32]) = value[2];
//	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Float_4x3) {
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 0])  = value[0];
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 16]) = value[1];
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 32]) = value[2];
//		*reinterpret_cast<float3*>(&storage.blockStorage[offset + 48]) = value[3];
//	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Double_2x3) {
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 0])  = value[0];
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 16]) = value[1];
//	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Double_3x3) {
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 0])  = value[0];
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 16]) = value[1];
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 32]) = value[2];
//	} else if constexpr (primitive_type_v<Type> == ReflectionPrimitiveType::Double_4x3) {
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 0])  = value[0];
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 16]) = value[1];
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 32]) = value[2];
//		*reinterpret_cast<double3*>(&storage.blockStorage[offset + 48]) = value[3];
//	} else {
//		*reinterpret_cast<Type*>(&storage.blockStorage[offset]) = value;
//	}
//}

static uint32_t get_offset(ReflectionDesc& desc)
{
	switch (desc.type) {
	case ReflectionType::Primitive: return static_cast<ReflectionPrimitiveDesc*>(&desc)->offset;
	case ReflectionType::Array:     return static_cast<ReflectionArrayDesc*>(&desc)->offset;
	case ReflectionType::Struct:    return static_cast<ReflectionStructDesc*>(&desc)->offset;
	}
	return 0;
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

template <class StorageArrayData>
static ShaderStorageData* subscript_storage_array_data(ShaderStorageData* data, uint32_t idx)
{
	auto& array_data = *static_cast<StorageArrayData*>(data);

	VERA_ASSERT_MSG(idx < array_data.elements.size(), "array subscript out of range");

	return array_data.elements[idx];
}

template <class StorageArrayData>
static ShaderStorageData* access_storage_array_data(ShaderStorageData* data, uint32_t idx)
{
	auto& array_data = *static_cast<StorageArrayData*>(data);

	if (array_data.elements.size() <= idx)
		throw Exception("resource array out of bounds: {} <= {}", array_data.elements.size(), idx);

	return &array_data.elements[idx];
}

ShaderVariable::ShaderVariable(
	ref<ShaderStorage> storage,
	ShaderStorageData* data,
	ReflectionDesc*    desc,
	uint32_t           offset
) :
	m_storage(storage),
	m_data(data),
	m_desc(desc),
	m_offset(offset)
{
	VERA_ASSERT(storage && data && desc);
}

ShaderVariable ShaderVariable::operator[](std::string_view name) VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_desc->type == ReflectionType::Struct, "variable does not have any member");

	auto& struct_desc = static_cast<const ReflectionStructDesc&>(*m_desc);

	if (int32_t idx = find_member_index(struct_desc, name.data()); 0 <= idx) {
		auto member_desc = struct_desc.members[idx];
		auto new_offset  = m_offset + get_offset(*member_desc);

		return ShaderVariable(m_storage, m_data, member_desc, new_offset);
	}

	VERA_ASSERT_MSG(false, "variable does not have member");
	// to supress warning
	return {};
}

ShaderVariable ShaderVariable::operator[](uint32_t idx) VERA_NOEXCEPT
{
	switch (m_desc->type) {
	case ReflectionType::Array: {
		const auto& array_desc = static_cast<const ReflectionArrayDesc&>(*m_desc);
		const auto  new_offset = m_offset + array_desc.stride * idx;

		VERA_ASSERT_MSG(array_desc.elementCount <= idx, "array subscript out of range");

		return ShaderVariable(m_storage, m_data, array_desc.element, new_offset);
	}
	case ReflectionType::ResourceArray: {
		auto& resource_array = static_cast<ShaderStorageResourceArrayData&>(*m_data);
		auto* element_desc   = static_cast<ReflectionResourceArrayDesc*>(m_desc)->element;

		VERA_ASSERT_MSG(idx < resource_array.elements.size(), "resource array subscript out of range");

		auto* array_element  = resource_array.elements[idx].frames.front();

		return ShaderVariable(m_storage, array_element, element_desc, 0);
	}
	default:
		VERA_ASSERT_MSG(false, "variable is not subscriptable");
	}

	// to supress warning
	return {};
}

ShaderVariable ShaderVariable::at(std::string_view name)
{
	if (m_desc->type != ReflectionType::Struct)
		throw Exception("variable does not have any member");
	
	auto& struct_desc = static_cast<const ReflectionStructDesc&>(*m_desc);

	if (int32_t idx = find_member_index(struct_desc, name.data()); 0 <= idx) {
		auto member_desc = struct_desc.members[idx];
		auto new_offset  = m_offset + get_offset(*member_desc);

		return ShaderVariable(m_storage, m_data, member_desc, new_offset);
	}

	throw Exception("variable does not have member: {}", name);
}

ShaderVariable ShaderVariable::at(uint32_t idx)
{
	switch (m_desc->type) {
	case ReflectionType::Array: {
		const auto& array_desc = static_cast<const ReflectionArrayDesc&>(*m_desc);
		const auto  new_offset = m_offset + array_desc.stride * idx;

		if (array_desc.elementCount <= idx)
			throw Exception("array subscript out of range");

		return ShaderVariable(m_storage, m_data, array_desc.element, new_offset);
	}
	case ReflectionType::ResourceArray: {
		auto& resource_array = static_cast<ShaderStorageResourceArrayData&>(*m_data);
		auto* element_desc   = static_cast<ReflectionResourceArrayDesc*>(m_desc)->element;
		
		if (resource_array.elements.size() <= idx)
			throw Exception("resource array subscript out of range");

		auto* array_element  = resource_array.elements[idx].frames.front();

		return ShaderVariable(m_storage, array_element, element_desc, 0);
	}
	default:
		throw Exception("variable is not subscriptable");
	}
}

void ShaderVariable::operator=(obj<Sampler> obj)
{
	setSampler(std::move(obj));
}

void ShaderVariable::operator=(obj<TextureView> obj)
{
	setTextureView(std::move(obj));
}

void ShaderVariable::operator=(obj<BufferView> obj)
{
	setBufferView(std::move(obj));
}

void ShaderVariable::operator=(obj<Buffer> obj)
{
	setBuffer(std::move(obj));
}

void ShaderVariable::setSampler(obj<Sampler> sampler)
{
#ifdef _DEBUG
	auto& desc = static_cast<ReflectionResourceDesc&>(*m_desc);
	VERA_ASSERT(m_desc->type == ReflectionType::Resource && (
		desc.resourceType == ResourceType::Sampler ||
		desc.resourceType == ResourceType::CombinedImageSampler));
#endif
	m_storage->setSampler(*this, std::move(sampler));
}

void ShaderVariable::setTextureView(obj<TextureView> texture_view)
{
	auto& desc = static_cast<ReflectionResourceDesc&>(*m_desc);

	VERA_ASSERT(m_desc->type == ReflectionType::Resource && (
		desc.resourceType == ResourceType::CombinedImageSampler ||
		desc.resourceType == ResourceType::SampledImage ||
		desc.resourceType == ResourceType::StorageImage ||
		desc.resourceType == ResourceType::InputAttachment));

	m_storage->setTextureView(
		*this,
		std::move(texture_view),
		desc.resourceType == ResourceType::InputAttachment ?
		TextureLayout::ColorAttachmentOptimal :
		TextureLayout::ShaderReadOnlyOptimal);
}

void ShaderVariable::setBufferView(obj<BufferView> buffer_view)
{
#ifdef _DEBUG
	auto& desc = static_cast<ReflectionResourceDesc&>(*m_desc);
	VERA_ASSERT(m_desc->type == ReflectionType::Resource && (
		desc.resourceType == ResourceType::UniformTexelBuffer ||
		desc.resourceType == ResourceType::StorageTexelBuffer));
#endif
	m_storage->setBufferView(*this, std::move(buffer_view));
}

void ShaderVariable::setBuffer(obj<Buffer> buffer)
{
#ifdef _DEBUG
	auto& desc = static_cast<ReflectionResourceDesc&>(*m_desc);
	VERA_ASSERT(m_desc->type == ReflectionType::Resource && (
		desc.resourceType == ResourceType::UniformBuffer ||
		desc.resourceType == ResourceType::UniformBufferDynamic ||
		desc.resourceType == ResourceType::StorageBuffer ||
		desc.resourceType == ResourceType::StorageBufferDynamic));
#endif
	m_storage->setBuffer(
		*this,
		std::move(buffer),
		0,
		VK_WHOLE_SIZE);
}

static ShaderStorageData* create_resource_data(ShaderStorageImpl& impl, const ReflectionResourceDesc& desc)
{
	// TODO: use default resource

	switch (desc.resourceType) {
	case ResourceType::Sampler: {
		auto* data = new ShaderStorageSamplerData;
		data->sampler = impl.device->getDefaultSampler();
		return data;
	} break;
	case ResourceType::CombinedImageSampler: {
		auto* data = new ShaderStorageCombinedImageSamplerData;
		data->sampler = impl.device->getDefaultSampler();
		return data;
	} break;
	case ResourceType::SampledImage:
	case ResourceType::StorageImage:
	case ResourceType::InputAttachment: {
		auto* data = new ShaderStorageTextureData;
		data->textureView = impl.device->getDefaultTextureView();
		return data;
	} break;
	case ResourceType::UniformTexelBuffer:
	case ResourceType::StorageTexelBuffer: {
		auto* data = new ShaderStorageTexelBufferData;
		data->bufferView = impl.device->getDefaultBufferView();
		return data;
	}
	case ResourceType::UniformBuffer:
	case ResourceType::UniformBufferDynamic:
	case ResourceType::StorageBuffer:
	case ResourceType::StorageBufferDynamic: {
		auto* data = new ShaderStorageBufferData;
		data->buffer = impl.device->getDefaultBuffer();
		data->offset = 0;
		data->range  = VK_WHOLE_SIZE;
		return data;
	}
	default:
		VERA_ASSERT_MSG(false, "invalid resource type for resource data");
	}
}

static ShaderStorageData* create_resource_block_data(ShaderStorageImpl& impl, const ReflectionResourceBlockDesc& desc)
{
	auto& block_desc = static_cast<const ReflectionResourceBlockDesc&>(desc);
	auto* data       = new ShaderStorageBufferBlockData;

	switch (desc.resourceType) {
	case ResourceType::UniformBuffer:
	case ResourceType::UniformBufferDynamic: {
		data->buffer = Buffer::createUniform(impl.device, block_desc.sizeInByte);
	} break;
	case ResourceType::StorageBuffer:
	case ResourceType::StorageBufferDynamic: {
		data->buffer = Buffer::createStorage(impl.device, block_desc.sizeInByte);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource type for resource block");
	}

	return data;
}

static ShaderStorageData* create_push_constant_data(ShaderStorageImpl& impl, const ReflectionPushConstantDesc& desc)
{
	auto* data = new ShaderStoragePushConstantData;

	data->block.resize(desc.sizeInByte);

	return data;
}

static ShaderStorageResourceArrayData* create_resource_array_data(ShaderStorageImpl& impl, const ReflectionResourceArrayDesc& desc)
{
	auto*  array_data    = new ShaderStorageResourceArrayData;
	size_t element_count = desc.elementCount == UINT32_MAX ? VERA_UNSIZED_ARRAY_RESOURCE_COUNT : desc.elementCount;

	array_data->elements.resize(element_count);

	switch (desc.element->type) {
	case ReflectionType::Resource: {
		for (size_t i = 0; i < element_count; i++) {
			auto& element_desc  = static_cast<const ReflectionResourceDesc&>(*desc.element);
			auto* resource_data = create_resource_data(impl, element_desc);
			array_data->elements[i].frames.push_back(resource_data);
		}
	} break;
	case ReflectionType::ResourceBlock: {
		for (size_t i = 0; i < element_count; i++) {
			auto& element_desc  = static_cast<const ReflectionResourceBlockDesc&>(*desc.element);
			auto* resource_data = create_resource_block_data(impl, element_desc);
			array_data->elements[i].frames.push_back(resource_data);
		}
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid element type for resource array");
	}

	return array_data;
}

static void create_storage_data(ShaderStorageImpl& impl, const PipelineLayoutImpl& layout_impl)
{
	auto reflections = array_view(layout_impl.reflection.reflections, layout_impl.reflection.reflectionCount);

	impl.storageDatas.reserve(reflections.size());
	for (auto& desc : reflections) {
		switch (desc->type) {
		case ReflectionType::Resource: {
			auto& res_desc      = static_cast<const ReflectionResourceDesc&>(*desc);
			auto* resource_data = new ShaderStorageResourceData;
			resource_data->frames.push_back(create_resource_data(impl, res_desc));
			impl.storageDatas.push_back(resource_data);
		} break;
		case ReflectionType::ResourceBlock: {
			auto& block_desc    = static_cast<const ReflectionResourceBlockDesc&>(*desc);
			auto* resource_data = new ShaderStorageResourceData;
			resource_data->frames.push_back(create_resource_block_data(impl, block_desc));
			impl.storageDatas.push_back(resource_data);
		} break;
		case ReflectionType::PushConstant: {
			auto& pc_desc       = static_cast<const ReflectionPushConstantDesc&>(*desc);
			auto* resource_data = new ShaderStorageResourceData;
			resource_data->frames.push_back(create_push_constant_data(impl, pc_desc));
			impl.storageDatas.push_back(resource_data);
		} break;
		case ReflectionType::ResourceArray: {
			auto& array_desc = static_cast<const ReflectionResourceArrayDesc&>(*desc);
			auto* array_data = create_resource_array_data(impl, array_desc);
			impl.storageDatas.push_back(array_data);
		} break;
		default:
			VERA_ASSERT_MSG(false, "invalid reflection type for resource data");
		}
	}
}

static void destroy_storage_data(ShaderStorageImpl& impl)
{
	for (auto* data : impl.storageDatas)
		delete data;
	impl.storageDatas.clear();
}

static void append_storage_frame(ShaderStorageImpl& impl, size_t at)
{
	auto& frame = *impl.storageFrames.emplace(impl.storageFrames.cbegin() + at);
	auto& pool  = impl.resourcePools.emplace_back();

	pool = ResourceBindingPool::create(impl.device);

	frame.frameID = impl.frameId++;
}

static void prepare_resource_write(ShaderStorageImpl& impl)
{

}

static void prepare_pc_write()
{

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

obj<ShaderStorage> ShaderStorage::create(obj<PipelineLayout> pipeline_layout)
{
	VERA_ASSERT_MSG(pipeline_layout, "empty pipeline layout");

	auto  obj         = createNewCoreObject<ShaderStorage>();
	auto& impl        = getImpl(obj);
	auto& layout_impl = getImpl(pipeline_layout);

	check_pipeline_layout(layout_impl);

	impl.device         = layout_impl.device;
	impl.pipelineLayout = std::move(pipeline_layout);
	impl.frameIndex     = 0;

	create_storage_data(impl, layout_impl);

	append_storage_frame(impl, 0);

	return obj;
}

ShaderStorage::~ShaderStorage()
{
	auto& impl = getImpl(this);

	destroy_storage_data(impl);

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

ShaderVariable ShaderStorage::accessVariable(std::string_view name)
{
	auto& impl        = getImpl(this);
	auto& layout_impl = getImpl(impl.pipelineLayout);

	int32_t idx = find_member_index(layout_impl.reflection, name.data());
	if (idx == -1)
		throw Exception("variable not found: {}", name);

	auto& reflection = *layout_impl.reflection.reflections[idx];
	auto& data       = *impl.storageDatas[idx];

	return ShaderVariable(this, &data, &reflection, 0);
}

void ShaderStorage::setSampler(const ShaderVariable& variable, obj<Sampler> sampler)
{
}

void ShaderStorage::setTextureView(const ShaderVariable& variable, obj<TextureView> texture_view, TextureLayout texture_layout)
{
}

void ShaderStorage::setBufferView(const ShaderVariable& variable, obj<BufferView> buffer_view)
{
}

void ShaderStorage::setBuffer(const ShaderVariable& variable, obj<Buffer> buffer, size_t offset, size_t range)
{
}

void ShaderStorage::setPrimitive(const ShaderVariable& variable, const bool value)
{
}

VERA_NAMESPACE_END