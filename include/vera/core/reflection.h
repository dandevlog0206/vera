#pragma once

#include "enum_types.h"
#include "../util/array_view.h"
#include <string_view>
#include <optional>

VERA_NAMESPACE_BEGIN

class ReflectionSpecConstant;

enum
{
	MAX_REFLECTION_ARRAY_DIMENSION_COUNT = 8
};

enum class ReflectionPrimitiveType VERA_ENUM
{
	Unknown,

	// Scalar types
	Bool,
	Char,
	UChar,
	Short,
	Int,
	UInt,
	Long,
	ULong,
	Float,
	Double,

	// Vector types
	Bool2,
	Bool3,
	Bool4,

	Char2,
	Char3,
	Char4,

	UChar2,
	UChar3,
	UChar4,

	Short2,
	Short3,
	Short4,

	UShort,
	UShort2,
	UShort3,
	UShort4,

	Int2,
	Int3,
	Int4,

	UInt2,
	UInt3,
	UInt4,

	Long2,
	Long3,
	Long4,

	ULong2,
	ULong3,
	ULong4,

	Float2,
	Float3,
	Float4,

	Double2,
	Double3,
	Double4,

	// Row-major matrix types
	RFloat2x2,
	RFloat2x3,
	RFloat2x4,
	RFloat3x2,
	RFloat3x3,
	RFloat3x4,
	RFloat4x2,
	RFloat4x3,
	RFloat4x4,

	RDouble2x2,
	RDouble2x3,
	RDouble2x4,
	RDouble3x2,
	RDouble3x3,
	RDouble3x4,
	RDouble4x2,
	RDouble4x3,
	RDouble4x4,

	// Column-major matrix types
	CFloat2x2,
	CFloat2x3,
	CFloat2x4,
	CFloat3x2,
	CFloat3x3,
	CFloat3x4,
	CFloat4x2,
	CFloat4x3,
	CFloat4x4,

	CDouble2x2,
	CDouble2x3,
	CDouble2x4,
	CDouble3x2,
	CDouble3x3,
	CDouble3x4,
	CDouble4x2,
	CDouble4x3,
	CDouble4x4
};

enum class ReflectionDecorationFlagBits VERA_FLAG_BITS64
{
	None                        = 0ull,
	RelaxedPrecision            = 1ull << 0,  // SpvDecorationRelaxedPrecision
	SpecId                      = 1ull << 1,  // SpvDecorationSpecId
	Block                       = 1ull << 2,  // SpvDecorationBlock
	BufferBlock                 = 1ull << 3,  // SpvDecorationBufferBlock
	RowMajor                    = 1ull << 4,  // SpvDecorationRowMajor
	ColMajor                    = 1ull << 5,  // SpvDecorationColMajor
	ArrayStride                 = 1ull << 6,  // SpvDecorationArrayStride
	MatrixStride                = 1ull << 7,  // SpvDecorationMatrixStride
	GLSLShared                  = 1ull << 8,  // SpvDecorationGLSLShared
	GLSLPacked                  = 1ull << 9,  // SpvDecorationGLSLPacked
	CPacked                     = 1ull << 10, // SpvDecorationCPacked
	BuiltIn                     = 1ull << 11, // SpvDecorationBuiltIn
	NoPerspective               = 1ull << 12, // SpvDecorationNoPerspective
	Flat                        = 1ull << 13, // SpvDecorationFlat
	Patch                       = 1ull << 14, // SpvDecorationPatch
	Centroid                    = 1ull << 15, // SpvDecorationCentroid
	Sample                      = 1ull << 16, // SpvDecorationSample
	Invariant                   = 1ull << 17, // SpvDecorationInvariant
	Restrict                    = 1ull << 18, // SpvDecorationRestrict
	Aliased                     = 1ull << 19, // SpvDecorationAliased
	Volatile                    = 1ull << 20, // SpvDecorationVolatile
	Constant                    = 1ull << 21, // SpvDecorationConstant
	Coherent                    = 1ull << 22, // SpvDecorationCoherent
	NonWritable                 = 1ull << 23, // SpvDecorationNonWritable
	NonReadable                 = 1ull << 24, // SpvDecorationNonReadable
	Uniform                     = 1ull << 25, // SpvDecorationUniform
	UniformId                   = 1ull << 26, // SpvDecorationUniformId
	SaturatedConversion         = 1ull << 27, // SpvDecorationSaturatedConversion
	Stream                      = 1ull << 28, // SpvDecorationStream
	Location                    = 1ull << 29, // SpvDecorationLocation
	Component                   = 1ull << 30, // SpvDecorationComponent
	Index                       = 1ull << 31, // SpvDecorationIndex
	Binding                     = 1ull << 32, // SpvDecorationBinding
	DescriptorSet               = 1ull << 33, // SpvDecorationDescriptorSet
	Offset                      = 1ull << 34, // SpvDecorationOffset
	XfbBuffer                   = 1ull << 35, // SpvDecorationXfbBuffer
	XfbStride                   = 1ull << 36, // SpvDecorationXfbStride
	FuncParamAttr               = 1ull << 37, // SpvDecorationFuncParamAttr
	FPRoundingMode              = 1ull << 38, // SpvDecorationFPRoundingMode
	FPFastMathMode              = 1ull << 39, // SpvDecorationFPFastMathMode
	NoContraction               = 1ull << 43, // SpvDecorationNoContraction
	InputAttachmentIndex        = 1ull << 40, // SpvDecorationInputAttachmentIndex
	Alignment                   = 1ull << 41, // SpvDecorationAlignment
	MaxByteOffset               = 1ull << 42, // SpvDecorationMaxByteOffset
	PerPrimitive                = 1ull << 43, // SpvDecorationPerPrimitiveNV
	PerView                     = 1ull << 44, // SpvDecorationPerViewNV
	PerTask                     = 1ull << 44, // SpvDecorationPerTaskNV
	PerVertex                   = 1ull << 45, // SpvDecorationPerVertexNV
	UserSemantic                = 1ull << 46, // SpvDecorationUserSemantic
} VERA_ENUM_FLAGS(ReflectionDecorationFlagBits, ReflectionDecorationFlags)

enum class ReflectionBuiltIn VERA_ENUM
{
	Unknown,
	Position,
	PointSize,
	ClipDistance,
	CullDistance,
	VertexId,
	InstanceId,
	PrimitiveId,
	InvocationId,
	Layer,
	ViewportIndex,
	TessLevelOuter,
	TessLevelInner,
	TessCoord,
	PatchVertices,
	FragCoord,
	PointCoord,
	FrontFacing,
	SampleId,
	SamplePosition,
	SampleMask,
	FragDepth,
	HelperInvocation,
	NumWorkgroups,
	WorkgroupSize,
	WorkgroupId,
	LocalInvocationId,
	GlobalInvocationId,
	LocalInvocationIndex,
	WorkDim,
	GlobalSize,
	EnqueuedWorkgroupSize,
	GlobalOffset,
	GlobalLinearId,
	SubgroupSize,
	SubgroupMaxSize,
	NumSubgroups,
	NumEnqueuedSubgroups,
	SubgroupId,
	SubgroupLocalInvocationId,
	VertexIndex,
	InstanceIndex
};

enum class ReflectionInterfaceIO VERA_ENUM
{
	Unknown,
	Input,
	Output
};

class ReflectionArrayTraits
{
public:
	VERA_CONSTEXPR ReflectionArrayTraits() VERA_NOEXCEPT :
		m_dim_count(0),
		m_stride(0),
		m_dims(nullptr),
		m_spec_consts(nullptr),
		m_is_runtimes(nullptr) {}

	VERA_CONSTEXPR ReflectionArrayTraits(
		uint32_t                             dim_count,
		uint32_t                             stride,
		const uint32_t*                      dims,
		const ReflectionSpecConstant* const* spec_consts = nullptr,
		const bool*                          is_runtime  = nullptr
	) VERA_NOEXCEPT :
		m_dim_count(dim_count),
		m_stride(stride),
		m_dims(dims),
		m_spec_consts(spec_consts),
		m_is_runtimes(is_runtime) {}

	VERA_NODISCARD VERA_CONSTEXPR uint32_t operator[](uint32_t dim) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(dim < m_dim_count, "array dimension index out of range");
		return m_dims[dim];
	}

	VERA_NODISCARD VERA_CONSTEXPR uint32_t dims() const VERA_NOEXCEPT
	{
		return m_dim_count;
	}

	VERA_NODISCARD VERA_CONSTEXPR uint32_t stride() const VERA_NOEXCEPT
	{
		return m_stride;
	}

	VERA_NODISCARD VERA_CONSTEXPR const ReflectionSpecConstant* getDimSpecConstant(uint32_t dim) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(dim < m_dim_count, "array dimension index out of range");
		return m_spec_consts ? m_spec_consts[dim] : nullptr;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool isDimRuntime(uint32_t dim) const VERA_NOEXCEPT
	{
		VERA_ASSERT_MSG(dim < m_dim_count, "array dimension index out of range");
		return m_is_runtimes ? m_is_runtimes[dim] : false;
	}

	VERA_NODISCARD VERA_CONSTEXPR uint32_t getTotalElementCount() const VERA_NOEXCEPT
	{
		if (m_dim_count == 0) return 0;

		uint32_t totalCount = 1;
		
		for (uint32_t i = 0; i < m_dim_count; ++i)
			totalCount *= m_dims[i];

		return totalCount;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool isUnsized() const VERA_NOEXCEPT
	{
		return m_dim_count > 0 && m_dims[m_dim_count - 1] == 0;
	}

	VERA_NODISCARD VERA_CONSTEXPR bool empty() const VERA_NOEXCEPT
	{
		return m_dim_count == 0;
	}

private:
	uint32_t                             m_dim_count;
	uint32_t                             m_stride;
	const uint32_t*                      m_dims;
	const ReflectionSpecConstant* const* m_spec_consts;
	const bool*                          m_is_runtimes;
};

class ReflectionInterfaceVariable
{
public:
	ShaderStageFlags                               stageFlags;
	std::string_view                               name;
	uint32_t                                       location;
	uint32_t                                       component;
	ReflectionInterfaceIO                          io;
	std::string_view                               semantic;
	ReflectionDecorationFlags                      decorationFlags;
	ReflectionBuiltIn                              builtIn;
	ReflectionPrimitiveType                        primitiveType;
	Format                                         format;
	ReflectionArrayTraits                          arrayTraits;
	array_view<const ReflectionInterfaceVariable*> members;
};

class ReflectionBlockVariable
{
public:
	ShaderStageFlags                           stageFlags;
	std::string_view                           name;
	uint32_t                                   offset;
	uint32_t                                   absoluteOffset;
	uint32_t                                   size;
	uint32_t                                   paddedSize;
	ReflectionDecorationFlags                  decorationFlags;
	ReflectionPrimitiveType                    primitiveType;
	ReflectionArrayTraits                      arrayTraits;
	array_view<const ReflectionBlockVariable*> members;
};

class ReflectionDescriptorBinding
{
public:
	ShaderStageFlags               stageFlags;
	std::string_view               name;
	uint32_t                       set;
	uint32_t                       binding;
	uint32_t                       inputAttachmentIndex;
	DescriptorType                 descriptorType;
	ReflectionDecorationFlags      decorationFlags;
	ReflectionArrayTraits          arrayTraits;
	const ReflectionBlockVariable* block;
	uint32_t                       elementCount;
	bool                           accessed;
};

class ReflectionDescriptorSet
{
public:
	ShaderStageFlags                               stageFlags;
	uint32_t                                       set;
	array_view<const ReflectionDescriptorBinding*> bindings;

	VERA_NODISCARD VERA_CONSTEXPR const ReflectionDescriptorBinding* getBinding(uint32_t binding) const VERA_NOEXCEPT
	{
		for (const auto& b : bindings)
			if (b->binding == binding)
				return b;

		VERA_ERROR_MSG("Descriptor binding not found");
	}
};

class ReflectionPushConstant
{
public:
	ShaderStageFlags               stageFlags;
	std::string_view               name;
	uint32_t                       offset;
	uint32_t                       size;
	const ReflectionBlockVariable* block;
};

class ReflectionSpecConstant
{
public:
	std::string_view name;
	uint32_t         constantId;
};

VERA_NAMESPACE_END
