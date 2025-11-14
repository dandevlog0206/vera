#include "spirv_parser.h"

#include <spirv-headers/spirv.h>

VERA_NAMESPACE_BEGIN

class SpvInstruction
{
public:
	SpvInstruction(const uint32_t* code, size_t size);
	~SpvInstruction();

	VERA_INLINE uint32_t getWordCount() const VERA_NOEXCEPT
	{
		return *m_ptr & 0xffff;
	}

	VERA_INLINE SpvOp getOp() const VERA_NOEXCEPT
	{
		return static_cast<SpvOp>(*m_ptr >> 16);
	}

private:
	const uint32_t* m_ptr;
};

array_view<uint32_t> SpvParser::strip_reflection_info(const uint32_t* spirv_code, size_t size_in_byte)
{

	return array_view<uint32_t>();
}

VERA_NAMESPACE_END
