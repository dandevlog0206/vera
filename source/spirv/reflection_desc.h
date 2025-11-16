#pragma once

#include "../../../include/vera/core/pipeline_layout.h"
#include "../../../include/vera/util/flat_hash_map.h"
#include "../../../include/vera/util/array_view.h"
#include "../../../include/vera/util/range.h"
#include "../../../include/vera/util/flag.h"
#include <memory_resource>
#include <string_view>
#include <string>
#include <vector>

VERA_NAMESPACE_BEGIN

enum class ReflectionParseMode VERA_ENUM
{
	Minimal,
	Full
};

enum class ReflectionPropertyFlagBits VERA_FLAG_BITS
{
	None           = 0,
	Type           = 1 << 0,
	StageFlags     = 1 << 1,
	Offset         = 1 << 2,
	NameList       = 1 << 3,
	DescriptorType = 1 << 4,
	PaddedSize     = 1 << 5,
	Set            = 1 << 6,
	PrimitiveType  = 1 << 7,
	Binding        = 1 << 8,
	MemberNodes    = 1 << 9,
	ElementNode    = 1 << 10,
	ElementCount   = 1 << 11,
	Stride         = 1 << 12,
	NameMap        = 1 << 13
} VERA_ENUM_FLAGS(ReflectionPropertyFlagBits, ReflectionPropertyFlags)

// enum value has propery bits
enum class ReflectionType VERA_ENUM
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
		ReflectionPropertyFlagBits::NameList |
		ReflectionPropertyFlagBits::DescriptorType |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding),

	DescriptorBlock = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::NameList |
		ReflectionPropertyFlagBits::DescriptorType |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding |
		ReflectionPropertyFlagBits::MemberNodes |
		ReflectionPropertyFlagBits::NameMap),

	DescriptorArray = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::NameList |
		ReflectionPropertyFlagBits::DescriptorType |
		ReflectionPropertyFlagBits::Set |
		ReflectionPropertyFlagBits::Binding |
		ReflectionPropertyFlagBits::ElementNode |
		ReflectionPropertyFlagBits::ElementCount |
		ReflectionPropertyFlagBits::Stride),

	PushConstant    = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::NameList |
		ReflectionPropertyFlagBits::MemberNodes |
		ReflectionPropertyFlagBits::NameMap),

	Struct          = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::NameList |
		ReflectionPropertyFlagBits::Offset |
		ReflectionPropertyFlagBits::PaddedSize |
		ReflectionPropertyFlagBits::MemberNodes |
		ReflectionPropertyFlagBits::NameMap),

	Array           = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::NameList |
		ReflectionPropertyFlagBits::Offset |
		ReflectionPropertyFlagBits::PaddedSize |
		ReflectionPropertyFlagBits::Stride |
		ReflectionPropertyFlagBits::ElementCount |
		ReflectionPropertyFlagBits::ElementNode),

	Primitive       = static_cast<uint32_t>(
		ReflectionPropertyFlagBits::Type |
		ReflectionPropertyFlagBits::StageFlags |
		ReflectionPropertyFlagBits::NameList |
		ReflectionPropertyFlagBits::Offset |
		ReflectionPropertyFlagBits::PaddedSize |
		ReflectionPropertyFlagBits::PrimitiveType)
};

// Node offset layout for x64 architecture
/*
 * offset0   - type
 * offset4   - stageFlags
 * offset8   - nameList
 * offset16  - descriptorType / offset
 * offset20  - set / paddedSize
 * offset24  - binding / primitiveType
 * offset28  - blockSize / elementCount
 * offset32  - memberNodes / elementNode
 * offset40  - stride
 * offset48  - nameMap
 */

template <ReflectionPropertyFlagBits FlagBit>
static VERA_CONSTEXPR size_t reflection_property_offset_v = UINT64_MAX;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::Type>           = 0;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::StageFlags>     = 4;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::NameList>       = 8;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::DescriptorType> = 16;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::Offset>         = 16;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::Set>            = 20;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::PaddedSize>     = 20;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::Binding>        = 24;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::PrimitiveType>  = 24;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::MemberNodes>    = 32;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::ElementNode>    = 32;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::ElementCount>   = 40;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::Stride>         = 44;
template <> static VERA_CONSTEXPR size_t reflection_property_offset_v<ReflectionPropertyFlagBits::NameMap>        = 48;

class ReflectionNode;
class ReflectionRootMemberNode;
class ReflectionDescriptorNode;
class ReflectionBlockMemberNode;
class ReflectionPushConstantNode;

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4200) // nonstandard extension used: zero-sized array in struct/union
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wpedantic"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif

struct ReflectionNameChain
{
	const ReflectionNameChain* next;
	ShaderStageFlags           stageFlags;
	char                       name[];
};

#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

struct ReflectionEntryPoint
{
	ShaderStageFlags stageFlags;
	const char*      name;
};

struct ReflectionNodeMapping
{
	const ReflectionNode* node;
	ShaderStageFlags      stageFlags;
};

typedef const ReflectionNameChain* ReflectionNameListPtr;

typedef array_view<const ReflectionDescriptorNode*> ReflectionSetRange;

typedef std::pmr::polymorphic_allocator<std::pair<std::string_view,
	ReflectionNodeMapping>> ReflectionNameMapAllocator;

typedef std::pmr::polymorphic_allocator<std::pair<uint64_t, const ReflectionDescriptorNode*>>
	ReflectionDescriptorBindingAllocator;

typedef ska::flat_hash_map<std::string_view, ReflectionNodeMapping, std::hash<std::string_view>,
	std::equal_to<std::string_view>, ReflectionNameMapAllocator> ReflectionNameMap;

typedef ska::flat_hash_map<uint64_t, const ReflectionDescriptorNode*, std::hash<uint64_t>, std::equal_to<uint64_t>,
	ReflectionDescriptorBindingAllocator> ReflectionBindingMap;

class ReflectionNode
{
public:
	ReflectionType type;

	template <class NodeType>
	VERA_NODISCARD VERA_INLINE const NodeType* as() const VERA_NOEXCEPT;

	template <class NodeType>
	VERA_NODISCARD VERA_INLINE NodeType* as() VERA_NOEXCEPT
	{
		return const_cast<NodeType*>(static_cast<const ReflectionNode*>(this)->as<NodeType>());
	}

	VERA_NODISCARD VERA_INLINE ReflectionType getType() const VERA_NOEXCEPT
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
			reflection_property_offset_v<ReflectionPropertyFlagBits::StageFlags>);
	}

	VERA_NODISCARD VERA_INLINE const ReflectionNameChain* getNameList() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::NameList),
			"name property is not available for this node type");

		return reinterpret_cast<const ReflectionNameChain*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::NameList>);
	}

	VERA_NODISCARD VERA_INLINE const char* getName() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::NameList),
			"name property is not available for this node type");

		const auto* chain = *reinterpret_cast<const ReflectionNameListPtr*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::NameList>);

		return chain ? chain->name : nullptr;
	}

	VERA_NODISCARD VERA_INLINE DescriptorType getDescriptorType() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::DescriptorType),
			"descriptor type property is not available for this node type");

		return *reinterpret_cast<const DescriptorType*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::DescriptorType>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getOffset() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Offset),
			"offset property is not available for this node type");
	
		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::Offset>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getSet() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Set),
			"set property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::Set>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getPaddedSize() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::PaddedSize),
			"padded size property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::PaddedSize>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getBinding() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Binding),
			"binding property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::Binding>);
	}

	VERA_NODISCARD VERA_INLINE ReflectionPrimitiveType getPrimitiveType() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::PrimitiveType),
			"primitive type property is not available for this node type");

		return *reinterpret_cast<const ReflectionPrimitiveType*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::PrimitiveType>);
	}

	VERA_NODISCARD VERA_INLINE array_view<const ReflectionNode*> getMemberNodes() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::MemberNodes),
			"member nodes property is not available for this node type");

		return *reinterpret_cast<const array_view<const ReflectionNode*>*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::MemberNodes>);
	}

	VERA_NODISCARD VERA_INLINE array_view<const ReflectionRootMemberNode*> getRootMemberNodes() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			type == ReflectionType::Root,
			"root member nodes is not available for this node type");

		return *reinterpret_cast<const array_view<const ReflectionRootMemberNode*>*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::MemberNodes>);
	}

	VERA_NODISCARD VERA_INLINE array_view<const ReflectionBlockMemberNode*> getBlockMemberNodes() VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			type == ReflectionType::DescriptorBlock ||
			type == ReflectionType::PushConstant ||
			type == ReflectionType::Struct,
			"block member nodes is not available for this node type");

		return *reinterpret_cast<const array_view<const ReflectionBlockMemberNode*>*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::MemberNodes>);
	}

	VERA_NODISCARD VERA_INLINE array_view<const ReflectionNode*> getRootMemberNodes() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			type == ReflectionType::Root,
			"root member nodes is not available for this node type");

		return *reinterpret_cast<const array_view<const ReflectionNode*>*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::MemberNodes>);
	}

	VERA_NODISCARD VERA_INLINE array_view<const ReflectionBlockMemberNode*> getBlockMemberNodes() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			type == ReflectionType::DescriptorBlock ||
			type == ReflectionType::PushConstant ||
			type == ReflectionType::Struct,
			"block member nodes is not available for this node type");

		return *reinterpret_cast<const array_view<const ReflectionBlockMemberNode*>*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::MemberNodes>);
	}

	VERA_NODISCARD VERA_INLINE const ReflectionNode* getElementNode() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::ElementNode),
			"element node property is not available for this node type");

		return *reinterpret_cast<const ReflectionNode* const*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::ElementNode>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getElementCount() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::ElementCount),
			"element count property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::ElementCount>);
	}

	VERA_NODISCARD VERA_INLINE uint32_t getStride() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::Stride),
			"stride property is not available for this node type");

		return *reinterpret_cast<const uint32_t*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::Stride>);
	}

	VERA_NODISCARD VERA_INLINE const ReflectionNameMap& getNameMap() const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(
			hasProperty(ReflectionPropertyFlagBits::NameMap),
			"name map property is not available for this node type");

		return *reinterpret_cast<const ReflectionNameMap*>(
			reinterpret_cast<const std::byte*>(this) +
			reflection_property_offset_v<ReflectionPropertyFlagBits::NameMap>);
	}
};

class ReflectionRootMemberNode abstract : public ReflectionNode
{
public:
//	ReflectionType        type;
	ShaderStageFlags      stageFlags;
	ReflectionNameListPtr nameList;
};

class ReflectionBlockMemberNode abstract : public ReflectionNode
{
public:
//	ReflectionType        type;
	ShaderStageFlags      stageFlags;
	ReflectionNameListPtr nameList;
	uint32_t              offset;
	uint32_t              paddedSize;
};

//// DO NOT MODIFY BELOW THIS LINE ////

class ReflectionRootNode : public ReflectionNode
{
public:
//	ReflectionType                              type;
	ShaderStageFlags                            stageFlags;
	ReflectionTargetFlags                       targetFlags;
	ReflectionParseMode                         parseMode;
	array_view<ReflectionEntryPoint>            entryPoints;
	array_view<const ReflectionRootMemberNode*> memberNodes;
	ReflectionNameMap                           nameMap;
	ReflectionBindingMap                        bindingMap;
	array_view<ReflectionSetRange>              setRanges;
	uint32_t                                    descriptorCount;
	uint32_t                                    pushConstantCount;
	uint32_t                                    minSet;
	uint32_t                                    maxSet;
	uint32_t                                    localSizeX;
	uint32_t                                    localSizeY;
	uint32_t                                    localSizeZ;

	VERA_NODISCARD ShaderStageFlags getShaderStageFlags() const VERA_NOEXCEPT;
	VERA_NODISCARD ReflectionTargetFlags getTargetFlags() const VERA_NOEXCEPT;

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

class ReflectionDescriptorNode : public ReflectionRootMemberNode
{
public:
//	ReflectionType        type;
//	ShaderStageFlags      stageFlags;
//	ReflectionNameListPtr nameList;
	DescriptorType        descriptorType;
	uint32_t              set;
	uint32_t              binding;
	VERA_MEMBER_PADDING(sizeof(uint32_t))
};

class ReflectionDescriptorArrayNode : public ReflectionDescriptorNode
{
public:
//	ReflectionType        type;
//	ShaderStageFlags      stageFlags;
//	ReflectionNameListPtr nameList;
//	DescriptorType        descriptorType;
//	uint32_t              set;
//	uint32_t              binding;
//	VERA_MEMBER_PADDING(sizeof(uint32_t))
	const ReflectionNode* elementNode;
	uint32_t              elementCount;
	uint32_t              stride;
};

class ReflectionDescriptorBlockNode : public ReflectionDescriptorNode
{
public:
//	ReflectionType                               type;
//	ShaderStageFlags                             stageFlags;
//	ReflectionNameListPtr                        nameList;
//	DescriptorType                               descriptorType;
//	uint32_t                                     set;
//	uint32_t                                     binding;
//	VERA_MEMBER_PADDING(sizeof(uint32_t))
	array_view<const ReflectionBlockMemberNode*> memberNodes;
	ReflectionNameMap                            nameMap;
};

class ReflectionPushConstantNode : public ReflectionRootMemberNode
{
public:
//	ReflectionType                               type;
//	ShaderStageFlags                             stageFlags;
//	ReflectionNameListPtr                        nameList;
	PushConstantRange                            pushConstantRange;
	VERA_MEMBER_PADDING(sizeof(uint32_t))
	array_view<const ReflectionBlockMemberNode*> memberNodes;
	ReflectionNameMap                            nameMap;
};

class ReflectionStructNode : public ReflectionBlockMemberNode
{
public:
//	ReflectionType                               type;
//	ShaderStageFlags                             stageFlags;
//	ReflectionNameListPtr                        nameList;
//	uint32_t                                     offset;
//	uint32_t                                     paddedSize;
	VERA_MEMBER_PADDING(sizeof(uint32_t))
	VERA_MEMBER_PADDING(sizeof(uint32_t))
	array_view<const ReflectionBlockMemberNode*> memberNodes;
	ReflectionNameMap                            nameMap;
};

class ReflectionArrayNode : public ReflectionBlockMemberNode
{
public:
//	ReflectionType                   type;
//	ShaderStageFlags                 stageFlags;
//	ReflectionNameListPtr            nameList;
//	uint32_t                         offset;
//	uint32_t                         paddedSize;
	VERA_MEMBER_PADDING(sizeof(uint32_t))
	VERA_MEMBER_PADDING(sizeof(uint32_t))
	const ReflectionBlockMemberNode* elementNode;
	uint32_t                         elementCount;
	uint32_t                         stride;
};

class ReflectionPrimitiveNode : public ReflectionBlockMemberNode
{
public:
//	ReflectionType          type;
//	ShaderStageFlags        stageFlags;
//	ReflectionNameListPtr   nameList;
//	uint32_t                offset;
//	uint32_t                paddedSize;
	ReflectionPrimitiveType primitiveType;
};

template <class NodeType>
VERA_NODISCARD VERA_INLINE const NodeType* ReflectionNode::as() const VERA_NOEXCEPT
{
#ifdef _DEBUG
	if constexpr (std::is_same_v<NodeType, ReflectionBlockMemberNode>) {
		VERA_ASSERT_MSG(
			type == ReflectionType::Struct ||
			type == ReflectionType::Array ||
			type == ReflectionType::Primitive, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionRootMemberNode>) {
		VERA_ASSERT_MSG(
			type == ReflectionType::Descriptor ||
			type == ReflectionType::DescriptorBlock ||
			type == ReflectionType::DescriptorArray ||
			type == ReflectionType::PushConstant, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionRootNode>) {
		VERA_ASSERT_MSG(type == ReflectionType::Root, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionDescriptorNode>) {
		VERA_ASSERT_MSG(
			type == ReflectionType::Descriptor ||
			type == ReflectionType::DescriptorBlock ||
			type == ReflectionType::DescriptorArray, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionDescriptorBlockNode>) {
		VERA_ASSERT_MSG(type == ReflectionType::DescriptorBlock, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionDescriptorArrayNode>) {
		VERA_ASSERT_MSG(type == ReflectionType::DescriptorArray, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionPushConstantNode>) {
		VERA_ASSERT_MSG(type == ReflectionType::PushConstant, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionStructNode>) {
		VERA_ASSERT_MSG(type == ReflectionType::Struct, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionArrayNode>) {
		VERA_ASSERT_MSG(type == ReflectionType::Array, "invalid node type cast");
	} else if constexpr (std::is_same_v<NodeType, ReflectionPrimitiveNode>) {
		VERA_ASSERT_MSG(type == ReflectionType::Primitive, "invalid node type cast");
	}
#endif // _DEBUG

	return reinterpret_cast<const NodeType*>(this);
}

//// DO NOT MODIFY ABOVE THIS LINE ////

class ReflectionDesc
{
public:
	static array_view<uint32_t> stripReflectionInstructions(
		const uint32_t* spirv_code,
		size_t          size_in_byte);

	ReflectionDesc() {};
	~ReflectionDesc() {};

	void parse(
		const uint32_t*           spirv_code,
		const size_t              size_in_byte,
		const ReflectionParseMode mode = ReflectionParseMode::Full);

	void merge(array_view<const ReflectionRootNode*> root_nodes);

	VERA_NODISCARD const ReflectionRootNode* getRootNode() const VERA_NOEXCEPT;

	void clear() VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	std::pmr::monotonic_buffer_resource m_memory;
	ReflectionRootNode*                 m_root_node;
	hash_t                              m_spirv_hash;
};

VERA_NAMESPACE_END