#pragma once

#include "object_impl.h"

#include "../../include/vera/core/shader.h"
#include "../../include/vera/core/resource_layout.h"

#include <variant>
#include <unordered_map>

VERA_NAMESPACE_BEGIN

class Shader;

/*
Primitive      -> must be leaf
Array          -> Primitive / Struct
Struct         -> eache members are Primitive/Array/Struct
Resource       -> sampler or texture or buffer
ResourceBlock  -> uniform buffer, storage buffer
PushConstant   -> pushConstant
ResourceArray  -> Resource/ResourceBlock
*/

enum class ReflectionType VERA_ENUM
{
	Unknown,
	Primitive,
	Array,
	Struct,
	Resource,
	ResourceBlock,
	PushConstant,
	ResourceArray
};

enum class ReflectionPrimitiveType VERA_ENUM
{
	Unknown,

	Bool,
	Bool_2,
	Bool_3,
	Bool_4,

	Int8,
	Int8_2,
	Int8_3,
	Int8_4,

	UInt8,
	UInt8_2,
	UInt8_3,
	UInt8_4,

	Int16,
	Int16_2,
	Int16_3,
	Int16_4,

	UInt16,
	UInt16_2,
	UInt16_3,
	UInt16_4,

	Int32,
	Int32_2,
	Int32_3,
	Int32_4,

	UInt32,
	UInt32_2,
	UInt32_3,
	UInt32_4,

	Int64,
	Int64_2,
	Int64_3,
	Int64_4,

	UInt64,
	UInt64_2,
	UInt64_3,
	UInt64_4,

	Float,
	Float_2,
	Float_3,
	Float_4,

	Float_2x2,
	Float_2x3,
	Float_2x4,
	Float_3x2,
	Float_3x3,
	Float_3x4,
	Float_4x2,
	Float_4x3,
	Float_4x4,

	Double,
	Double_2,
	Double_3,
	Double_4,

	Double_2x2,
	Double_2x3,
	Double_2x4,
	Double_3x2,
	Double_3x3,
	Double_3x4,
	Double_4x2,
	Double_4x3,
	Double_4x4
};

struct ReflectionArrayTraits
{
	uint32_t dimCount;
	uint32_t stride;
	uint32_t dims[8];
};

struct ReflectionDesc
{
	ReflectionType      type;
	const char*         name;
};

struct ReflectionPrimitiveDesc : ReflectionDesc
{
	ReflectionPrimitiveType primitiveType;
	uint32_t                offset;
};

struct ReflectionArrayDesc : ReflectionDesc
{
	uint32_t        stride;
	uint32_t        elementCount; // UINT32_MAX for unsized array
	ReflectionDesc* element;      // pointer to single element descriptor
	uint32_t        offset;
};

struct ReflectionStructDesc : ReflectionDesc
{
	uint32_t         memberCount;
	ReflectionDesc** members;
	uint32_t         offset;
};

struct ReflectionRootDesc : ReflectionDesc
{
	ShaderStageFlags    shaderStageFlags;
	ref<ResourceLayout> resourceLayout;
};

struct ReflectionResourceDesc : ReflectionRootDesc
{
	ResourceType     resourceType;
	uint16_t         set;
	uint16_t         binding;
};

struct ReflectionResourceBlockDesc : ReflectionRootDesc
{
	uint32_t          sizeInByte;
	ResourceType      resourceType;
	uint16_t          set;
	uint16_t          binding;
	uint32_t          memberCount;
	ReflectionDesc**  members;
};

struct ReflectionPushConstantDesc : ReflectionRootDesc
{
	uint32_t          sizeInByte;
	uint32_t          memberCount;
	ReflectionDesc**  members;
};

struct ReflectionResourceArrayDesc : ReflectionRootDesc
{
	ResourceType     resourceType;
	uint16_t         set;
	uint16_t         binding;
	uint32_t         elementCount; // UINT32_MAX for unsized array
	ReflectionDesc*  element;      // pointer to single element descriptor
};

struct ShaderReflectionImpl
{
	using hash_map        = std::unordered_map<std::string_view, uint32_t>;
	using Shaders         = std::vector<obj<Shader>>;
	using ReflectionDescs = std::vector<ReflectionDesc*>;

	obj<Device>      device;
	Shaders          shaders;

	hash_map         hashMap;
	ReflectionDescs  descriptors;
	ShaderStageFlags shaderStageFlags;
};

VERA_NAMESPACE_END