#pragma once

#include "../../../include/vera/core/shader.h"
#include "../../../include/vera/util/range.h"

VERA_NAMESPACE_BEGIN

enum class DescriptorType VERA_ENUM;

class DescriptorSetLayout;

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
	ResourceArray,
	PushConstant
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

/*

###################################################################################################
##########   Struct Inheritance Diagram   #########################################################
###################################################################################################


ReflectionDesc (abstract)
	戍式式 ReflectionBlockDesc (abstract)
	弛   戍式式 ReflectionPrimitiveDesc
	弛   戍式式 ReflectionArrayDesc
	弛   戌式式 ReflectionStructDesc
	弛
	戌式式 ReflectionRootMemberDesc (abstract)
		戍式式 ReflectionResourceDesc
		弛   戍式式 ReflectionResourceBlockDesc
		弛   戌式式 ReflectionResourceArrayDesc
		弛
		戌式式 ReflectionPushConstantDesc


*/

struct ReflectionNameMap
{
	const char*      name;
	ShaderStageFlags stageFlags;
	uint32_t         index;
};

struct ReflectionDesc
{
	ReflectionType type;
};

struct ReflectionBlockDesc : ReflectionDesc
{
//  ReflectionType      type;
	uint32_t offset;
};

struct ReflectionRootMemberDesc : ReflectionDesc
{
//  ReflectionType      type;
	ShaderStageFlags    stageFlags;
	uint32_t            reflectionIndex;
};

struct ReflectionPrimitiveDesc : ReflectionBlockDesc
{
//  ReflectionType          type;                = ReflectionType::Primitive
//  uint32_t                offset;
	ReflectionPrimitiveType primitiveType;
};

struct ReflectionArrayDesc : ReflectionBlockDesc
{
//  ReflectionType       type;                   = ReflectionType::Array
//  uint32_t             offset;
	uint32_t             stride;
	uint32_t             elementCount;           // UINT32_MAX for unsized array
	ReflectionBlockDesc* element;                // pointer to single element descriptor
};

struct ReflectionStructDesc : ReflectionBlockDesc
{
//  ReflectionType        type;                  = ReflectionType::Struct
//  uint32_t              offset;
	uint32_t              memberCount;
	ReflectionBlockDesc** members;
	uint32_t              nameMapCount;
	ReflectionNameMap*    nameMaps;
};

struct ReflectionResourceDesc : ReflectionRootMemberDesc
{
//  ReflectionType                 type;         = ReflectionType::Resource
//  ShaderStageFlags               stageFlags;
// 	uint32_t                       reflectionIndex;
	const_ref<DescriptorSetLayout> descriptorSetLayout;
	DescriptorType                 descriptorType;
	uint32_t                       set;
	uint32_t                       binding;
};

struct ReflectionResourceBlockDesc : ReflectionResourceDesc
{
//  ReflectionType                 type;         = ReflectionType::ResourceBlock
//  ShaderStageFlags               stageFlags;
// 	uint32_t                       reflectionIndex;
//  const_ref<DescriptorSetLayout> descriptorSetLayout;
//  DescriptorType                 descriptorType;
//  uint32_t                       set;
//  uint32_t                       binding;
	uint32_t                       sizeInByte;
	uint32_t                       memberCount;
	ReflectionBlockDesc**          members;
	uint32_t                       nameMapCount;
	ReflectionNameMap*             nameMaps;
};

struct ReflectionResourceArrayDesc : ReflectionResourceDesc
{
//  ReflectionType                 type;         = ReflectionType::ResourceArray
//  ShaderStageFlags               stageFlags;
// 	uint32_t                       reflectionIndex;
//  const_ref<DescriptorSetLayout> descriptorSetLayout;
//  DescriptorType                 descriptorType;
//  uint32_t                       set;
//  uint32_t                       binding;
	uint32_t                       elementCount; // UINT32_MAX for unsized array
	ReflectionResourceDesc*        element;      // pointer to ReflectionResourceDesc or ReflectionResourceBlockDesc
};

struct ReflectionPushConstantDesc : ReflectionRootMemberDesc
{
//  ReflectionType        type;                  = ReflectionType::PushConstant
//  ShaderStageFlags      stageFlags;
// 	uint32_t              reflectionIndex;
	uint32_t              sizeInByte;
	uint32_t              memberCount;
	ReflectionBlockDesc** members;
	uint32_t              nameMapCount;
	ReflectionNameMap*    nameMaps;
};

typedef ReflectionDesc* ReflectionDescPtr;
typedef ReflectionResourceDesc* ReflectionResourceDescPtr;
typedef ReflectionBlockDesc* ReflectionBlockDescPtr;
typedef ReflectionRootMemberDesc* ReflectionRootMemberDescPtr;

struct ShaderReflection
{
	uint32_t                   reflectionCount;  // must ordered by set, binding, pc must be the last
	ReflectionRootMemberDesc** reflections;
	uint32_t                   nameMapCount;
	ReflectionNameMap*         nameMaps;         // must ordered by name
	uint32_t                   setRangeCount;
	basic_range<uint32_t>*     setRanges;
	uint32_t                   resourceCount;
	uint32_t                   pushConstantCount;
};

extern void destroy_shader_reflection(ShaderReflection& reflection);

VERA_NAMESPACE_END
