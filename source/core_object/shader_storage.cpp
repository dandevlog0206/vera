#include "../../include/vera/core/shader_storage.h"
#include "../impl/shader_storage_impl.h"
#include "../impl/shader_reflection_impl.h"
#include "../impl/pipeline_layout_impl.h"

#include "../../include/vera/core/device.h"
#include "../../include/vera/core/shader_reflection.h"
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







static uint32_t get_offset(ReflectionDesc& desc)
{
	switch (desc.type) {
	case ReflectionType::Primitive: return static_cast<ReflectionPrimitiveDesc*>(&desc)->offset;
	case ReflectionType::Array:     return static_cast<ReflectionArrayDesc*>(&desc)->offset;
	case ReflectionType::Struct:    return static_cast<ReflectionStructDesc*>(&desc)->offset;
	}
	return 0;
}

template <class MemberDesc>
static int32_t find_member_index(const MemberDesc& desc, const char* name)
{
	int32_t lo = 0;
	int32_t hi = desc.memberCount - 1;

	while (lo <= hi) {
		int32_t mid = lo + (hi - lo) / 2;
		int32_t cmp = strcmp(desc.members[mid]->name, name);

		if (cmp == 0) {
			return mid;
		} else if (cmp < 0) {
			lo = mid + 1;
		} else /* 0 < cmp */ {
			hi = mid - 1;
		}
	}

	return -1;
}

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








template <class StorageArrayData>
static ShaderStorageData* subscript_storage_array_data(ShaderStorageData* data, uint32_t idx)
{
	auto& array_data = *static_cast<StorageArrayData*>(data);

	VERA_ASSERT_MSG(idx < array_data.elements.size(), "array subscript out of range");

	return &array_data.elements[idx];
}

template <class StorageArrayData>
static ShaderStorageData* access_storage_array_data(ShaderStorageData* data, uint32_t idx)
{
	auto& array_data = *static_cast<StorageArrayData*>(data);

	if (array_data.elements.size() <= idx)
		throw Exception("resource array out of bounds: {} <= {}", array_data.elements.size(), idx);

	return &array_data.elements[idx];
}

static void create_resource_data(ShaderStorageImpl& impl, const ReflectionResourceDesc& desc)
{
	switch (desc.resourceType) {
	case ResourceType::Sampler:
		impl.storageDatas.push_back(new ShaderStorageSamplerData);
		break;
	case ResourceType::CombinedImageSampler:
		impl.storageDatas.push_back(new ShaderStorageCombinedImageSamplerData);
		break;
	case ResourceType::SampledImage:
	case ResourceType::StorageImage:
		impl.storageDatas.push_back(new ShaderStorageTextureData);
		break;
	case ResourceType::UniformTexelBuffer:
	case ResourceType::StorageTexelBuffer:
		impl.storageDatas.push_back(new ShaderStorageTexelBufferData);
		break;
	case ResourceType::InputAttachment:
		impl.storageDatas.push_back(new ShaderStorageTextureData);
		break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource type for resource data");
	}
}

static void create_resource_block_data(ShaderStorageImpl& impl, const ReflectionResourceBlockDesc& desc)
{
	VERA_ASSERT(
		desc.resourceType == ResourceType::UniformBuffer ||
		desc.resourceType == ResourceType::UniformBufferDynamic ||
		desc.resourceType == ResourceType::StorageBuffer ||
		desc.resourceType == ResourceType::StorageBufferDynamic);

	auto* data = new ShaderStorageBufferBlockData;
	data->block.resize(desc.sizeInByte);

	impl.storageDatas.push_back(data);
}

static void create_push_constant_data(ShaderStorageImpl& impl, const ReflectionPushConstantDesc& desc)
{
	impl.storageDatas.push_back(new ShaderStoragePushConstantData);
}

static void create_resource_array_data(ShaderStorageImpl& impl, const ReflectionResourceArrayDesc& desc)
{
	size_t element_count = desc.elementCount == UINT32_MAX ? VERA_UNSIZED_ARRAY_RESOURCE_COUNT : desc.elementCount;

	switch (desc.resourceType) {
	case ResourceType::Sampler: {
		auto* data = new ShaderStorageSamplerArrayData;		
		data->elements.resize(element_count);
		impl.storageDatas.push_back(data);
	} break;
	case ResourceType::CombinedImageSampler: {
		auto* data = new ShaderStorageCombinedImageSamplerArrayData;
		data->elements.resize(element_count);
		impl.storageDatas.push_back(data);
	} break;
	case ResourceType::SampledImage:
	case ResourceType::StorageImage: {
		auto* data = new ShaderStorageTextureArrayData;
		data->elements.resize(element_count);
		impl.storageDatas.push_back(data);
	} break;
	case ResourceType::UniformTexelBuffer:
	case ResourceType::StorageTexelBuffer: {
		auto* data = new ShaderStorageTexelBufferArrayData;
		data->elements.resize(element_count);
		impl.storageDatas.push_back(data);
	} break;
	case ResourceType::UniformBuffer:
	case ResourceType::UniformBufferDynamic:
	case ResourceType::StorageBuffer:
	case ResourceType::StorageBufferDynamic: {
		VERA_ASSERT_MSG(false, "check at runtime");

		const auto* block_desc = static_cast<const ReflectionResourceBlockDesc*>(desc.element);

		auto* data = new ShaderStorageBufferBlockArrayData;
		data->elements.resize(element_count);
		for (auto& elem : data->elements)
			elem.block.resize(block_desc->sizeInByte);

		impl.storageDatas.push_back(data);
	} break;
	case ResourceType::InputAttachment: {
		auto* data = new ShaderStorageTextureArrayData;
		data->elements.resize(element_count);
		impl.storageDatas.push_back(data);
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid resource type for resource array");
	}
}

static void create_storage_data(ShaderStorageImpl& impl, const ShaderReflectionImpl& reflection_impl)
{
	for (auto& desc : reflection_impl.descriptors) {
		switch (desc->type) {
		case ReflectionType::Resource: {
			auto& res_desc = static_cast<const ReflectionResourceDesc&>(*desc);
			create_resource_data(impl, res_desc);
		} break;
		case ReflectionType::ResourceBlock: {
			auto& block_desc = static_cast<const ReflectionResourceBlockDesc&>(*desc);
			create_resource_block_data(impl, block_desc);
		} break;
		case ReflectionType::PushConstant: {
			auto& pc_desc = static_cast<const ReflectionPushConstantDesc&>(*desc);
			create_push_constant_data(impl, pc_desc);
		} break;
		case ReflectionType::ResourceArray: {
			auto& array_desc = static_cast<const ReflectionResourceArrayDesc&>(*desc);
			create_resource_array_data(impl, array_desc);
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
		const auto& array_desc = static_cast<const ReflectionResourceArrayDesc&>(*m_desc);

		switch (array_desc.resourceType) {
		case ResourceType::Sampler: {
			auto* data = subscript_storage_array_data<ShaderStorageSamplerArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		case ResourceType::CombinedImageSampler: {
			auto* data = subscript_storage_array_data<ShaderStorageCombinedImageSamplerArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		case ResourceType::SampledImage:
		case ResourceType::StorageImage: {
			auto* data = subscript_storage_array_data<ShaderStorageTextureArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}	
		case ResourceType::UniformTexelBuffer:
		case ResourceType::StorageTexelBuffer: {
			auto* data = subscript_storage_array_data<ShaderStorageTexelBufferArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		case ResourceType::UniformBuffer:
		case ResourceType::UniformBufferDynamic:
		case ResourceType::StorageBuffer:
		case ResourceType::StorageBufferDynamic: {
			if (typeid(*m_data) == typeid(ShaderStorageBufferArrayData)) {
				auto* data = subscript_storage_array_data<ShaderStorageBufferArrayData>(m_data, idx);
				return ShaderVariable(m_storage, data, array_desc.element, 0);
			} else /* typeid(*m_data) == typeid(ShaderStorageBufferBlockArrayData) */ {
				auto* data = subscript_storage_array_data<ShaderStorageBufferBlockArrayData>(m_data, idx);
				return ShaderVariable(m_storage, data, array_desc.element, 0);
			}
		}
		case ResourceType::InputAttachment: {
			auto* data = subscript_storage_array_data<ShaderStorageTextureArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		default:
			VERA_ASSERT_MSG(false, "invalid resource type for resource array");
		}
	}
	default:
		VERA_ASSERT_MSG(false, "variable is not subscriptable");
	}
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
		const auto& array_desc = static_cast<const ReflectionResourceArrayDesc&>(*m_desc);

		switch (array_desc.resourceType) {
		case ResourceType::Sampler: {
			auto* data = access_storage_array_data<ShaderStorageSamplerArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		case ResourceType::CombinedImageSampler: {
			auto* data = access_storage_array_data<ShaderStorageCombinedImageSamplerArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		case ResourceType::SampledImage:
		case ResourceType::StorageImage: {
			auto* data = access_storage_array_data<ShaderStorageTextureArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}	
		case ResourceType::UniformTexelBuffer:
		case ResourceType::StorageTexelBuffer: {
			auto* data = access_storage_array_data<ShaderStorageTexelBufferArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		case ResourceType::UniformBuffer:
		case ResourceType::UniformBufferDynamic:
		case ResourceType::StorageBuffer:
		case ResourceType::StorageBufferDynamic: {
			if (typeid(*m_data) == typeid(ShaderStorageBufferArrayData)) {
				auto* data = access_storage_array_data<ShaderStorageBufferArrayData>(m_data, idx);
				return ShaderVariable(m_storage, data, array_desc.element, 0);
			} else /* typeid(*m_data) == typeid(ShaderStorageBufferBlockArrayData) */ {
				auto* data = access_storage_array_data<ShaderStorageBufferBlockArrayData>(m_data, idx);
				return ShaderVariable(m_storage, data, array_desc.element, 0);
			}
		}
		case ResourceType::InputAttachment: {
			auto* data = access_storage_array_data<ShaderStorageTextureArrayData>(m_data, idx);
			return ShaderVariable(m_storage, data, array_desc.element, 0);
		}
		default:
			VERA_ASSERT_MSG(false, "invalid resource type for resource array");
		}
	}
	default:
		throw Exception("variable '{}' is not subscriptable", m_desc->name);
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

obj<ShaderStorage> ShaderStorage::create(obj<ShaderReflection> reflection)
{
	VERA_ASSERT_MSG(reflection, "empty shader reflection");
	
	auto  obj             = createNewCoreObject<ShaderStorage>();
	auto& impl            = getImpl(obj);
	auto& reflection_impl = getImpl(reflection);

	impl.device     = reflection_impl.device;
	impl.reflection = std::move(reflection);
	impl.frameIndex = 0;

	create_storage_data(impl, reflection_impl);

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

obj<ShaderReflection> ShaderStorage::getShaderReflection() VERA_NOEXCEPT
{
	return getImpl(this).reflection;
}

ShaderVariable ShaderStorage::accessVariable(std::string_view name)
{
	auto& impl            = getImpl(this);
	auto& reflection_impl = getImpl(impl.reflection);

	auto it = reflection_impl.hashMap.find(name);
	if (it == reflection_impl.hashMap.cend())
		throw Exception("failed to access variable: {}", name);

	auto& desc = *reflection_impl.descriptors[it->second];
	auto& data = *impl.storageDatas[it->second];

	return ShaderVariable(this, &data, &desc, 0);
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

void ShaderStorage::tryWrite()
{
	auto& impl     = getImpl(this);
	auto& frame    = impl.storageFrames[impl.frameIndex];
	auto cmd_state = frame.commandSync.getState();
	
	if (cmd_state == CommandBufferState::Recording || cmd_state == CommandBufferState::Executable)
		return;
	
	auto  next_index = static_cast<size_t>(impl.frameIndex + 1) % impl.storageFrames.size();
	auto& next_frame = impl.storageFrames[next_index];
}

VERA_NAMESPACE_END