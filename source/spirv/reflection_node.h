#pragma once

#include "../../../include/vera/core/enum_types.h"
#include "../../../include/vera/core/pipeline_layout.h"
#include "../../../include/vera/util/flat_hash_map.h"
#include "../../../include/vera/util/array_view.h"
#include "../../../include/vera/util/range.h"
#include "../../../include/vera/util/flag.h"
#include <memory_resource>
#include <string_view>
#include <unordered_map>

namespace spv_reflect
{
	class ShaderModule;
}

VERA_NAMESPACE_BEGIN

class ReflectionNode;
class ReflectionResourceNode;
class ReflectionBlockNode;
class ReflectionRootNode;
class ReflectionDescriptorNode;
class ReflectionDescriptorArrayNode;
class ReflectionDescriptorBlockNode;
class ReflectionPushConstantNode;
class ReflectionStructNode;
class ReflectionArrayNode;
class ReflectionPrimitiveNode;

// Node offset layout for x64 architecture
/*
 * offset0  - type
 * offset4  - stageFlags
 * offset8  - name
 * offset16 - set
 * offset20 - binding
 * offset24 - descriptorType / offset
 * offset28 - paddedSize
 * offset32 - memberNodes / block / elementNode / primitiveType
 * offset40 - elementCount
 * offset44 - stride
 * offset48 - nameMap
 */

enum class ReflectionTargetFlagBits VERA_ENUM
{
	None                = 0,
	DescriptorSetLayout = 1 << 0,
	Shader              = 1 << 1,
	PipelineLayout      = 1 << 2
} VERA_ENUM_FLAGS(ReflectionTargetFlagBits, ReflectionTargetFlags)

enum class ReflectionPropertyFlagBits VERA_FLAG_BITS
{
	None           = 0,
	Type           = 1 << 0,
	StageFlags     = 1 << 1,
	Name           = 1 << 2,
	DescriptorType = 1 << 3,
	Offset         = 1 << 4,
	Set            = 1 << 5,
	PaddedSize     = 1 << 6,
	MemberNodes    = 1 << 7,
	Binding        = 1 << 8,
	PrimitiveType  = 1 << 9,
	NameMap        = 1 << 10,
	Block          = 1 << 11,
	ElementNode    = 1 << 12,
	ElementCount   = 1 << 13,
	Stride         = 1 << 14
} VERA_ENUM_FLAGS(ReflectionPropertyFlagBits, ReflectionPropertyFlags)

// enum value has propery bits
enum class ReflectionNodeType VERA_ENUM
{
	Unknown         = 0,

	Root            = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::MemberNodes |
		ReflectionPropertyFlagBits::NameMap),

	Descriptor      = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::Name |
		ReflectionPropertyFlagBits::DescriptorType |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding),

	DescriptorArray = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::Name |
		ReflectionPropertyFlagBits::DescriptorType |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding |
		ReflectionPropertyFlagBits::ElementNode |
		ReflectionPropertyFlagBits::ElementCount |
		ReflectionPropertyFlagBits::Stride),

	DescriptorBlock = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::Name |
		ReflectionPropertyFlagBits::DescriptorType |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding |
		ReflectionPropertyFlagBits::Block),

	PushConstant    = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::Name |
		ReflectionPropertyFlagBits::Offset |
		ReflectionPropertyFlagBits::PaddedSize |
		ReflectionPropertyFlagBits::Block),

	Struct          = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::Name |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding |
		ReflectionPropertyFlagBits::Offset |
		ReflectionPropertyFlagBits::PaddedSize |
		ReflectionPropertyFlagBits::MemberNodes |
		ReflectionPropertyFlagBits::NameMap),

	Array           = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::Name |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding |
		ReflectionPropertyFlagBits::Offset |
		ReflectionPropertyFlagBits::PaddedSize |
		ReflectionPropertyFlagBits::ElementNode |
		ReflectionPropertyFlagBits::ElementCount |
		ReflectionPropertyFlagBits::Stride),

	Primitive       = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::Name |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding |
		ReflectionPropertyFlagBits::Offset |
		ReflectionPropertyFlagBits::PaddedSize |
		ReflectionPropertyFlagBits::PrimitiveType)
};

typedef array_view<const ReflectionDescriptorNode*> ReflectionSetRange;

typedef std::pmr::polymorphic_allocator<std::pair<std::string_view,
	const ReflectionNode*>> ReflectionNameMapAllocator;

typedef std::pmr::polymorphic_allocator<std::pair<uint64_t, const ReflectionDescriptorNode*>>
	ReflectionDescriptorBindingAllocator;

//typedef ska::flat_hash_map<std::string_view, const ReflectionNode*, std::hash<std::string_view>,
//	std::equal_to<std::string_view>, ReflectionNameMapAllocator> ReflectionNameMap;

typedef std::pmr::unordered_map<std::string_view, const ReflectionNode*> ReflectionNameMap;

typedef ska::flat_hash_map<uint64_t, const ReflectionDescriptorNode*, std::hash<uint64_t>, std::equal_to<uint64_t>,
	ReflectionDescriptorBindingAllocator> ReflectionBindingMap;

class ReflectionNode
{
public:
	template <ReflectionPropertyFlagBits FlagBit>
	static VERA_CONSTEXPR size_t prop_off_v = UINT64_MAX;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::Type>           = 0;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::StageFlags>     = 4;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::Name>           = 8;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::Set>            = 16;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::Binding>        = 20;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::DescriptorType> = 24;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::Offset>         = 24;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::PaddedSize>     = 28;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::MemberNodes>    = 32;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::Block>          = 32;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::ElementNode>    = 32;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::PrimitiveType>  = 32;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::ElementCount>   = 40;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::Stride>         = 44;
	template <> static VERA_CONSTEXPR size_t prop_off_v<ReflectionPropertyFlagBits::NameMap>        = 48;

	ReflectionNodeType type;

	template <class NodeType>
	VERA_NODISCARD VERA_INLINE const NodeType* as() const VERA_NOEXCEPT;

	template <class NodeType>
	VERA_NODISCARD VERA_INLINE NodeType* as() VERA_NOEXCEPT
	{
		return const_cast<NodeType*>(static_cast<const ReflectionNode*>(this)->as<NodeType>());
	}

	VERA_NODISCARD VERA_INLINE ReflectionNodeType getType() const VERA_NOEXCEPT
	{
		return type;
	}

	VERA_NODISCARD VERA_INLINE ReflectionPropertyFlags getPropertyFlags() const VERA_NOEXCEPT
	{
		return std::bit_cast<ReflectionPropertyFlags>(type);
	}

	VERA_NODISCARD VERA_INLINE bool hasProperty(ReflectionPropertyFlags flags) const VERA_NOEXCEPT
	{
		return getPropertyFlags().has(flags);
	}

	VERA_NODISCARD VERA_INLINE ShaderStageFlags getStageFlags() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::StageFlags),
			"stage flags property is not available for this node type");

		return *reinterpret_cast<const ShaderStageFlags*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::StageFlags>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getOffset() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Offset),
			"offset property is not available for this node type");
	
		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::Offset>);
	}

	VERA_NODISCARD VERA_INLINE const char* getName() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Name),
			"name property is not available for this node type");

		return *reinterpret_cast<const char* const*>(
			reinterpret_cast<const std::byte*>(this) + 
			prop_off_v<ReflectionPropertyFlagBits::Name>);
	}

	VERA_NODISCARD VERA_INLINE DescriptorType getDescriptorType() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::DescriptorType),
			"descriptor type property is not available for this node type");

		return *reinterpret_cast<const DescriptorType*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::DescriptorType>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getPaddedSize() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::PaddedSize),
			"padded size property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::PaddedSize>);
	}
	
	VERA_NODISCARD VERA_INLINE uint32_t getSet() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Set),
			"set property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::Set>);
	}

	VERA_NODISCARD VERA_INLINE ReflectionPrimitiveType getPrimitiveType() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::PrimitiveType),
			"primitive type property is not available for this node type");

		return *reinterpret_cast<const ReflectionPrimitiveType*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::PrimitiveType>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getBinding() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Binding),
			"binding property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::Binding>);
	}

	VERA_NODISCARD VERA_INLINE array_view<const ReflectionNode*> getMemberNodes() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::MemberNodes),
			"member nodes property is not available for this node type");

		return *reinterpret_cast<const array_view<const ReflectionNode*>*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::MemberNodes>);
	}

	VERA_NODISCARD VERA_INLINE array_view<const ReflectionResourceNode*> getRootMemberNodes() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			type == ReflectionNodeType::Root,
			"root member nodes is not available for this node type");

		return *reinterpret_cast<const array_view<const ReflectionResourceNode*>*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::MemberNodes>);
	}

	VERA_NODISCARD VERA_INLINE const ReflectionNameMap& getNameMap() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::NameMap),
			"name map property is not available for this node type");

		return *reinterpret_cast<const ReflectionNameMap*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::NameMap>);
	}

	VERA_NODISCARD VERA_INLINE const ReflectionStructNode* getBlock() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Block),
			"block property is not available for this node type");

		return *reinterpret_cast<const ReflectionStructNode* const*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::Block>);
	}

	VERA_NODISCARD VERA_INLINE const ReflectionNode* getElementNode() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::ElementNode),
			"element node property is not available for this node type");

		return *reinterpret_cast<const ReflectionNode* const*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::ElementNode>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getElementCount() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::ElementCount),
			"element count property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::ElementCount>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getStride() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Stride),
			"stride property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			prop_off_v<ReflectionPropertyFlagBits::Stride>);
	}
};

class ReflectionResourceNode : public ReflectionNode
{
public:
//	ReflectionNodeType type;
	ShaderStageFlags   stageFlags;
	const char*        name;
};

class ReflectionBlockNode : public ReflectionNode
{
public:
//	ReflectionNodeType type;
	ShaderStageFlags   stageFlags;
	const char*        name;
	uint32_t           set;
	uint32_t           binding;
	uint32_t           offset;
	uint32_t           paddedSize;
};

//// DO NOT MODIFY BELOW THIS LINE ////

class ReflectionRootNode : public ReflectionNode
{
public:
	static const ReflectionRootNode* create(
		const spv_reflect::ShaderModule& shader_module,
		std::pmr::memory_resource*       memory
	);
	
	static const ReflectionRootNode* merge(
		array_view<const ReflectionRootNode*> roots,
		std::pmr::memory_resource*            memory
	);

//	ReflectionNodeType                        type;
	ShaderStageFlags                          stageFlags;
	ReflectionTargetFlags                     targetFlags;
	uint32_t                                  setCount;
	uint32_t                                  minSet;
	uint32_t                                  maxSet;
	uint32_t                                  descriptorCount;
	uint32_t                                  pushConstantCount;
	array_view<const ReflectionResourceNode*> memberNodes;
	ReflectionNameMap                         nameMap;
	array_view<ReflectionSetRange>            setRanges;
	ReflectionBindingMap                      bindingMap;

	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const VERA_NOEXCEPT;

	VERA_NODISCARD const char* getEntryPointName(ShaderStageFlagBits stage = {}) const VERA_NOEXCEPT;

	VERA_NODISCARD uint32_t getSetCount() const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t getMinSet() const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t getMaxSet() const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<const ReflectionDescriptorNode*> enumerateDescriptor() const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<const ReflectionDescriptorNode*> enumerateDescriptorSet(uint32_t set) const VERA_NOEXCEPT;
	VERA_NODISCARD array_view<const ReflectionPushConstantNode*> enumeratePushConstant() const VERA_NOEXCEPT;
	
	VERA_NODISCARD const ReflectionDescriptorNode* findDescriptor(uint32_t set, uint32_t binding) const VERA_NOEXCEPT;
	VERA_NODISCARD const ReflectionPushConstantNode* findPushConstant(ShaderStageFlags stage_flags = {}) const VERA_NOEXCEPT;
};

class ReflectionDescriptorNode : public ReflectionResourceNode
{
public:
//	ReflectionNodeType type;
//	ShaderStageFlags   stageFlags;
//	const char*        name;
	uint32_t           set;
	uint32_t           binding;
	DescriptorType     descriptorType;
	VERA_MEMBER_PADDING(uint32_t)
};

class ReflectionDescriptorArrayNode : public ReflectionDescriptorNode
{
public:
//	ReflectionNodeType              type;
//	ShaderStageFlags                stageFlags;
//	const char*                     name;
//	uint32_t                        set;
//	uint32_t                        binding;
//	DescriptorType                  descriptorType;
//	VERA_MEMBER_PADDING(uint32_t)
	const ReflectionDescriptorNode* elementNode;
	uint32_t                        elementCount;
	uint32_t                        stride;
};

class ReflectionDescriptorBlockNode : public ReflectionDescriptorNode
{
public:
//	ReflectionNodeType          type;
//	ShaderStageFlags            stageFlags;
//	const char*                 name;
//	uint32_t                    set;
//	uint32_t                    binding;
//	DescriptorType              descriptorType;
//	VERA_MEMBER_PADDING(uint32_t)
	const ReflectionStructNode* block;
};

class ReflectionPushConstantNode : public ReflectionResourceNode
{
public:
//	ReflectionNodeType          type;
//	ShaderStageFlags            stageFlags;
//	const char*                 name;
	VERA_MEMBER_PADDING(uint32_t)
	VERA_MEMBER_PADDING(uint32_t)
	uint32_t                    offset;
	uint32_t                    paddedSize;
	const ReflectionStructNode* block;
};

class ReflectionStructNode : public ReflectionBlockNode
{
public:
//	ReflectionNodeType                     type;
//	ShaderStageFlags                       stageFlags;
//	const char*                            name;
//	uint32_t                               set;
//	uint32_t                               binding;
//	uint32_t                               offset;
//	uint32_t                               paddedSize;
	array_view<const ReflectionBlockNode*> memberNodes;
	ReflectionNameMap                      nameMap;
};

class ReflectionArrayNode : public ReflectionBlockNode
{
public:
//	ReflectionNodeType         type;
//	ShaderStageFlags           stageFlags;
//	const char*                name;
//	uint32_t                   set;
//	uint32_t                   binding;
//	uint32_t                   offset;
//	uint32_t                   paddedSize;
	const ReflectionBlockNode* elementNode;
	uint32_t                   elementCount;
	uint32_t                   stride;
};

class ReflectionPrimitiveNode : public ReflectionBlockNode
{
public:
//	ReflectionNodeType      type;
//	ShaderStageFlags        stageFlags;
//	const char*             name;
//	uint32_t                set;
//	uint32_t                binding;
//	uint32_t                offset;
//	uint32_t                paddedSize;
	ReflectionPrimitiveType primitiveType;
};

template <class NodeType>
VERA_NODISCARD VERA_INLINE const NodeType* ReflectionNode::as() const VERA_NOEXCEPT
{
#ifdef _DEBUG
	if constexpr (std::is_same_v<NodeType, ReflectionBlockNode>) {
		VERA_ASSERT_MSG(
			type == ReflectionNodeType::Struct ||
			type == ReflectionNodeType::Array ||
			type == ReflectionNodeType::Primitive, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionResourceNode>) {
		VERA_ASSERT_MSG(
			type == ReflectionNodeType::Descriptor ||
			type == ReflectionNodeType::DescriptorBlock ||
			type == ReflectionNodeType::DescriptorArray ||
			type == ReflectionNodeType::PushConstant, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionRootNode>) {
		VERA_ASSERT_MSG(type == ReflectionNodeType::Root, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionDescriptorNode>) {
		VERA_ASSERT_MSG(
			type == ReflectionNodeType::Descriptor ||
			type == ReflectionNodeType::DescriptorBlock ||
			type == ReflectionNodeType::DescriptorArray, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionDescriptorBlockNode>) {
		VERA_ASSERT_MSG(type == ReflectionNodeType::DescriptorBlock, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionDescriptorArrayNode>) {
		VERA_ASSERT_MSG(type == ReflectionNodeType::DescriptorArray, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionPushConstantNode>) {
		VERA_ASSERT_MSG(type == ReflectionNodeType::PushConstant, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionStructNode>) {
		VERA_ASSERT_MSG(type == ReflectionNodeType::Struct, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionArrayNode>) {
		VERA_ASSERT_MSG(type == ReflectionNodeType::Array, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionPrimitiveNode>) {
		VERA_ASSERT_MSG(type == ReflectionNodeType::Primitive, "invalid node type cast");
	}
#endif // _DEBUG

	return reinterpret_cast<const NodeType*>(this);
}

//// DO NOT MODIFY ABOVE THIS LINE ////

VERA_NAMESPACE_END