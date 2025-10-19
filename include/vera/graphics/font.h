#pragma once

#include "../math/vector_types.h"
#include "../util/result_message.h"
#include <string_view>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

struct FontData;

VERA_PRIV_NAMESPACE_END

enum class FontResultType VERA_ENUM
{
	Success,
	FailedToOpenFile,
	UnsupportedFormat,
	InvalidID,
	InvalidFormat,
	ReadError,
};

typedef ResultMessage<FontResultType> FontResult;

class Font
{
public:
	Font() VERA_NOEXCEPT;
	Font(std::string_view path) VERA_NOEXCEPT;
	Font(const Font& other) VERA_NOEXCEPT;
	Font(Font&& other) VERA_NOEXCEPT;
	~Font() VERA_NOEXCEPT;
	Font& operator=(const Font& other) VERA_NOEXCEPT;
	Font& operator=(Font&& other) VERA_NOEXCEPT;

	FontResult load(std::string_view path) VERA_NOEXCEPT;

	VERA_NODISCARD FontResult getLastResult() const VERA_NOEXCEPT;

private:
	priv::FontData* m_data;
	FontResult      m_result;
};

VERA_NAMESPACE_END
