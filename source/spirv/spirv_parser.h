#pragma once

#include "../../include/vera/core/enum_types.h"
#include "../../include/vera/util/array_view.h"
#include "../../include/vera/util/result_message.h"
#include "../../include/vera/util/version.h"
#include <memory_resource>

VERA_NAMESPACE_BEGIN

enum class SpvParserResultType VERA_ENUM
{
	Success,
	NotReady,
	ErrorInvalidFlag,
	ErrorParseFailed,
	ErrorAllocFailed,
	ErrorRangeExceeded,
	ErrorNullPointer,
	ErrorInternalError,
	ErrorCountMismatch,
	ErrorElementNotFound,
	ErrorSpirvInvalidCodeSize,
	ErrorSpirvInvalidMagicNumber,
	ErrorSpirvUnexpectedEof,
	ErrorSpirvInvalidIdReference,
	ErrorSpirvSetNumberOverflow,
	ErrorSpirvInvalidStorageClass,
	ErrorSpirvRecursion,
	ErrorSpirvInvalidInstruction,
	ErrorSpirvUnexpectedBlockData,
	ErrorSpirvInvalidBlockMemberReference,
	ErrorSpirvInvalidEntryPoint,
	ErrorSpirvInvalidExecutionMode,
	ErrorSpirvMaxRecursiveExceeded,
};

enum class SpvParserFlagBits VERA_FLAG_BITS
{
	None              = 0,
	SpirvCodeNoCopy   = 1 << 0,
	ContextNoPreserve = 1 << 1,

	// flags below are mutually exclusive
	parseMinimal      = 1 << 3, 
	parseContinue     = 1 << 4,
	parseFull         = 1 << 5
} VERA_ENUM_FLAGS(SpvParserFlagBits, SpvParserFlags)

typedef ResultMessage<SpvParserResultType> SpvParserResult;

struct SpvParserContext;

struct SpvParserLocalSize
{
	uint32_t x;
	uint32_t y;
	uint32_t z;
};

struct SpvParserBlockVariable
{

};

struct SpvParserDescriptorBinding
{

};

struct SpvParserEntryPoint
{
	const char*        name;
	ShaderStageFlags   stage;
	SpvParserLocalSize localSize;

};

struct SpvParserShaderModule
{
	Version                         spirvVersion;
	uint32_t                        generatorMagic;
	array_view<SpvParserEntryPoint> entryPoints;
};

class SpvParser
{
public:
	static array_view<uint32_t> stripReflectionInfo(array_view<uint32_t> spirv_code);

	SpvParser() VERA_NOEXCEPT;
	~SpvParser();

	// no-exception is guaranteed, but the result must be checked
	// even though last parse() was unsuccessful, previous parse result may be preserved
	// check getResult() if current SpvParserShaderModule is available
	VERA_NODISCARD SpvParserResult parse(SpvParserFlags flags, array_view<uint32_t> spirv_code = {}) VERA_NOEXCEPT;
	VERA_NODISCARD SpvParserResult getResult() const VERA_NOEXCEPT;

	VERA_NODISCARD const SpvParserShaderModule& getShaderModule() const VERA_NOEXCEPT;

	void clear() VERA_NOEXCEPT;

private:
	std::pmr::monotonic_buffer_resource m_memory;
	std::pmr::monotonic_buffer_resource m_temp_memory;
	SpvParserShaderModule*              m_module;
	SpvParserContext*                   m_context;
	SpvParserResult                     m_result;
};

VERA_NAMESPACE_END