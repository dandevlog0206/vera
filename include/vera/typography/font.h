#pragma once

#include "../core/intrusive_ptr.h"
#include "../util/result_message.h"
#include "../util/array_view.h"
#include "../util/range.h"
#include "code_range.h"
#include "glyph.h"
#include <string_view>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

struct FontImpl;

VERA_PRIV_NAMESPACE_END

enum class FontResultType VERA_ENUM
{
	Success,
	AlreadyLoaded,
	FailedToOpenFile,
	TooManyFontsInFile,
	UnsupportedFormat,
	UnsupportedFeature,
	AllocationFailed,
	InvalidFormat,
	MissingCodepoint,
	MissingGlyph,
	OutOfBounds
};

typedef ResultMessage<FontResultType> FontResult;

class Font : public ManagedObject
{
	friend class FontManager;
	friend class FontAtlas;
	Font() VERA_NOEXCEPT;
public:
	VERA_NODISCARD static array_view<CodeRange> getDefaultCodeRanges() VERA_NOEXCEPT;

	VERA_NODISCARD obj<Font> create() VERA_NOEXCEPT;
	VERA_NODISCARD obj<Font> create(std::string_view path);
	~Font() VERA_NOEXCEPT;

	FontResult load(std::string_view path) VERA_NOEXCEPT;
	
	VERA_NODISCARD std::string_view getName() const VERA_NOEXCEPT;

	FontResult loadGlyphRange(const basic_range<uint32_t>& range) VERA_NOEXCEPT;
	FontResult loadCodeRange(const CodeRange& range) VERA_NOEXCEPT;

	VERA_NODISCARD uint32_t getGlyphCount() const VERA_NOEXCEPT;
	VERA_NODISCARD const Glyph* getGlyph(GlyphID glyph_id) const VERA_NOEXCEPT;
	VERA_NODISCARD const Glyph* findGlyph(char32_t codepoint) const VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;
private:
	priv::FontImpl* m_impl;
};

VERA_NAMESPACE_END
