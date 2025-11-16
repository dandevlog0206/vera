#include "spirv_parser.h"

#include <spirv-headers/spirv.h>

#define CHECK_RESULT(res)                              \
	do {                                               \
		if ((res) != SpvParserResultType::Success)     \
			return res;                                \
	} while (0)

#define CHECK_ALLOC(alloc)                                                            \
	try {                                                                             \
		alloc;                                                                        \
	} catch (const std::bad_alloc&) {                                                 \
		return { SpvParserResultType::ErrorAllocFailed, "memory allocation failed" }; \
	}

VERA_NAMESPACE_BEGIN

enum
{
	SPIRV_STARTING_WORD_OFFSET = 5
};

enum class SpvParserPassType
{
	None,
	Minimal,
	Continue,
	Full
};

// Provides iterator-like interface
class spv_inst
{
public:
	VERA_INLINE spv_inst() VERA_NOEXCEPT :
		m_ptr(nullptr) {}

	VERA_INLINE spv_inst(array_view<uint32_t> spirv_code, uint32_t offset) VERA_NOEXCEPT :
		m_ptr(spirv_code.data() + offset) {}

	VERA_INLINE spv_inst(const uint32_t* ptr, uint32_t offset) VERA_NOEXCEPT :
		m_ptr(ptr + offset) {}

	VERA_NODISCARD VERA_INLINE spv_inst next() const VERA_NOEXCEPT
	{
		return spv_inst(m_ptr, length());
	}

	VERA_NODISCARD VERA_INLINE auto operator<=>(const spv_inst& rhs) const VERA_NOEXCEPT
	{
		return m_ptr <=> rhs.m_ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator==(const spv_inst& rhs) const VERA_NOEXCEPT
	{
		return m_ptr == rhs.m_ptr;
	}

	VERA_NODISCARD VERA_INLINE bool operator!=(const spv_inst& rhs) const VERA_NOEXCEPT
	{
		return m_ptr != rhs.m_ptr;
	}

	VERA_INLINE uint32_t length() const VERA_NOEXCEPT
	{
		return *m_ptr & 0xffff;
	}

	VERA_INLINE SpvOp get_op() const VERA_NOEXCEPT
	{
		return static_cast<SpvOp>(*m_ptr >> 16);
	}

	VERA_INLINE SpvId get_id(uint32_t offset) const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const SpvId*>(m_ptr + offset);
	}

	VERA_INLINE array_view<SpvId> get_ids(uint32_t offset, uint32_t id_count) const VERA_NOEXCEPT
	{
		return array_view<SpvId>(reinterpret_cast<const SpvId*>(m_ptr + offset), id_count);
	}

	VERA_INLINE uint32_t get_uint32(uint32_t offset) const VERA_NOEXCEPT
	{
		return *(m_ptr + offset);
	}

	template <class EnumType>
	VERA_INLINE EnumType get_enum(uint32_t offset) const VERA_NOEXCEPT
	{
		return *reinterpret_cast<const EnumType*>(m_ptr + offset);
	}

	VERA_INLINE const char* get_string(uint32_t offset, uint32_t& end_offset) const VERA_NOEXCEPT
	{
		const char* str     = reinterpret_cast<const char*>(m_ptr + offset);
		uint32_t    str_len = strlen(str);
		
		end_offset = offset + (str_len - 1) / sizeof(uint32_t) + 1;
		
		return str;
	}

private:
	const uint32_t* m_ptr;
};

struct PackedNode
{
	SpvId    id   = UINT32_MAX;
	spv_inst inst = {};

	union {
		struct {
			const char*       name;
			ShaderStageFlags  stage;
			array_view<SpvId> interfaceIds;
		} entryPoint;
	};
};

class SpvParserContext
{
public:
	std::pmr::memory_resource*            memory;
	std::pmr::memory_resource*            tempMemory;

	uint32_t             spirvCodeSize = 0;
	const uint32_t*      spirvCode     = nullptr;

	uint32_t                     functionSectionOffset = 0;

	std::pmr::vector<PackedNode> nodes;

	static SpvParserContext* create(
		std::pmr::memory_resource* memory,
		std::pmr::memory_resource* temp_memory
	) {
		auto* result = static_cast<SpvParserContext*>(temp_memory->allocate(
			sizeof(SpvParserContext), alignof(SpvParserContext)));
		return std::construct_at(result, memory, temp_memory);
	}

	SpvParserContext(
		std::pmr::memory_resource* memory,
		std::pmr::memory_resource* temp_memory
	) VERA_NOEXCEPT :
		memory(memory),
		tempMemory(temp_memory) {}

	template <class T>
	T* alloc(uint32_t count = 1)
	{
		return static_cast<T*>(memory->allocate(sizeof(T) * count, alignof(T)));
	}

	template <class T>
	T* alloc_temp(uint32_t count = 1)
	{
		return static_cast<T*>(tempMemory->allocate(sizeof(T) * count, alignof(T)));
	}
};

static ShaderStageFlagBits get_shader_stage(SpvExecutionModel model)
{
	switch (model) {
	case SpvExecutionModelVertex:
		return ShaderStageFlagBits::Vertex;
	case SpvExecutionModelTessellationControl:
		return ShaderStageFlagBits::TessellationControl;
	case SpvExecutionModelTessellationEvaluation:
		return ShaderStageFlagBits::TessellationEvaluation;
	case SpvExecutionModelGeometry:
		return ShaderStageFlagBits::Geometry;
	case SpvExecutionModelFragment:
		return ShaderStageFlagBits::Fragment;
	case SpvExecutionModelGLCompute:
		return ShaderStageFlagBits::Compute;
	case SpvExecutionModelRayGenerationNV:
		return ShaderStageFlagBits::RayGen;
	case SpvExecutionModelIntersectionNV:
		return ShaderStageFlagBits::Intersection;
	case SpvExecutionModelAnyHitNV:
		return ShaderStageFlagBits::AnyHit;
	case SpvExecutionModelClosestHitNV:
		return ShaderStageFlagBits::ClosestHit;
	case SpvExecutionModelMissNV:
		return ShaderStageFlagBits::Miss;
	case SpvExecutionModelCallableNV:
		return ShaderStageFlagBits::Callable;
	default:
		return ShaderStageFlagBits::None;
	}
}

static bool is_parse_flag_mutually_exclusive(SpvParserFlags flags)
{
	return
		(flags.has(SpvParserFlagBits::parseMinimal) ? 1 : 0) +
		(flags.has(SpvParserFlagBits::parseContinue) ? 1 : 0) +
		(flags.has(SpvParserFlagBits::parseFull) ? 1 : 0) <= 1;
}

static SpvParserResult parse_node_minimal(
	SpvParserContext& ctx,
	const spv_inst    first,
	const spv_inst    last
) {
	spv_inst inst = first;

	while (inst != last) {
		SpvOp    op  = inst.get_op();
		uint32_t len = inst.length();
		
		switch (op) {
		case SpvOpEntryPoint: {
			uint32_t    interface_off;
			SpvId       id   = inst.get_id(2);

			PackedNode& node = ctx.nodes[id];
			node.id                      = id;
			node.inst                    = inst;
			node.entryPoint.name         = inst.get_string(3, interface_off);
			node.entryPoint.stage        = get_shader_stage(inst.get_enum<SpvExecutionModel>(1));
			node.entryPoint.interfaceIds = inst.get_ids(interface_off, len - interface_off);
		} break;
		}

		if (op == SpvOpFunction)
			break;

		inst = inst.next();

		if (inst > last)
			return { SpvParserResultType::ErrorSpirvUnexpectedEof,
				"unexpected end of SPIR-V code" };
	}

	return SpvParserResultType::Success;
}

static SpvParserResult parse_nodes(
	SpvParserContext& ctx,
	spv_inst          first,
	spv_inst          last
) {
	return SpvParserResultType::Success;
}

static SpvParserResult parse_impl(
	SpvParserContext&      ctx,
	SpvParserShaderModule& module,
	array_view<uint32_t>   spirv_code,
	SpvParserPassType      pass_type
) {
	if (spirv_code.empty())
		return { SpvParserResultType::ErrorNullPointer, "SPIR-V code is null or empty" };
	if (spirv_code.size() <= 5)
		return { SpvParserResultType::ErrorSpirvInvalidCodeSize, "SPIR-V code is too small" };
	if (spirv_code[0] != SpvMagicNumber)
		return { SpvParserResultType::ErrorSpirvInvalidMagicNumber, "invalid SPIR-V magic number" };

	uint32_t spv_version   = spirv_code[1];
	uint32_t spv_generator = spirv_code[2];
	uint32_t spv_id_bound  = spirv_code[3];

	module.spirvVersion   = Version(
		(spv_version >> 16) & 0xFF,
		(spv_version >> 8) & 0xFF,
		0);
	module.generatorMagic = spv_generator;


	spv_inst spv_first(spirv_code, SPIRV_STARTING_WORD_OFFSET);
	spv_inst spv_last(spirv_code, static_cast<uint32_t>(spirv_code.size()));

	// CHECK_RESULT(get_node_info(ctx, spirv_code));
}

array_view<uint32_t> SpvParser::stripReflectionInfo(array_view<uint32_t> spirv_code)
{
	VERA_NOT_IMPLEMENTED;
	return array_view<uint32_t>();
}

SpvParser::SpvParser() VERA_NOEXCEPT :
	m_module(nullptr),
	m_context(nullptr),
	m_result(SpvParserResultType::NotReady) {}

SpvParser::~SpvParser()
{
	clear();
}

SpvParserResult SpvParser::parse(SpvParserFlags flags, array_view<uint32_t> spirv_code) VERA_NOEXCEPT
{
	auto pass_type    = SpvParserPassType::None;
	bool has_minimal  = flags.has(SpvParserFlagBits::parseMinimal);
	bool has_continue = flags.has(SpvParserFlagBits::parseContinue);
	bool has_full     = flags.has(SpvParserFlagBits::parseFull);

	if (!is_parse_flag_mutually_exclusive(flags))
		return { SpvParserResultType::ErrorInvalidFlag, "mutually exclusive parse flags are set" };

	if (has_minimal || has_full) {
		if (spirv_code.empty())
			return { SpvParserResultType::ErrorNullPointer, "SPIR-V code is null or empty" };
		
		clear();

		try {
			m_context = SpvParserContext::create(&m_memory, &m_temp_memory);

			if (!flags.has(SpvParserFlagBits::SpirvCodeNoCopy)) {
				auto* new_memory = m_context->alloc_temp<uint32_t>(spirv_code.size());
				
				std::memcpy(new_memory, spirv_code.data(), sizeof(uint32_t) * spirv_code.size());
				m_context->spirvCodeSize = static_cast<uint32_t>(spirv_code.size());
				m_context->spirvCode     = new_memory;
				
				spirv_code = array_view(m_context->spirvCode, m_context->spirvCodeSize);
			}

			m_module = m_context->alloc<SpvParserShaderModule>();
		} catch (const std::bad_alloc&) {
			return m_result = { SpvParserResultType::ErrorAllocFailed, "memory allocation failed" };
		}

		pass_type = has_minimal ? SpvParserPassType::Minimal : SpvParserPassType::Full;
	} else if (has_continue) {
		if (!m_context)
			return { SpvParserResultType::ErrorNullPointer,
				"ContinueParse flag is set, but no context is available" };
		if (!m_context->spirvCode && spirv_code.empty())
			return { SpvParserResultType::ErrorNullPointer,
				"ContinueParse flag is set, but no SPIR-V code is available" };
		// TODO: check if spirv_code matches the previous one

		pass_type = SpvParserPassType::Continue;
	} else {
		return { SpvParserResultType::ErrorInvalidFlag,
			"no parse type flag is set" };
	}

	m_result = parse_impl(
			*m_context,
			*m_module,
			spirv_code,
			pass_type);

	if (m_result != SpvParserResultType::Success) {
		m_memory.release();
		m_temp_memory.release();
		m_context = nullptr;
		m_module  = nullptr;
		return m_result;
	}

	if (!flags.has(SpvParserFlagBits::ContextNoPreserve)) {
		m_temp_memory.release();
		m_context = nullptr;
	}

	return m_result;
}

SpvParserResult SpvParser::getResult() const VERA_NOEXCEPT
{
	return m_result;
}

const SpvParserShaderModule& SpvParser::getShaderModule() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_result == SpvParserResultType::Success && m_module,
		"shader module is not available");

	return *m_module;
}

void SpvParser::clear() VERA_NOEXCEPT
{
	m_memory.release();
	m_temp_memory.release();
	m_context = nullptr;
	m_module  = nullptr;
	m_result  = SpvParserResultType::NotReady;
}

VERA_NAMESPACE_END
