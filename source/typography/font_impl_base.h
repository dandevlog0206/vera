#pragma once

#include "../../include/vera/core/intrusive_ptr.h"
#include "../../include/vera/typography/font.h"

VERA_NAMESPACE_BEGIN

class FontManager;

VERA_PRIV_NAMESPACE_BEGIN

class FontImplBase abstract
{
public:
	virtual ~FontImplBase() VERA_NOEXCEPT = default;

	virtual std::string_view getName() const VERA_NOEXCEPT = 0;

	virtual void loadAllGlyphs() = 0;
	virtual void loadGlyphRange(const basic_range<uint32_t>& range) = 0;
	virtual void loadCodeRange(const CodeRange& range) = 0;

	virtual uint32_t getGlyphCount() const VERA_NOEXCEPT = 0;
	virtual GlyphID getGlyphID(char32_t codepoint) const = 0;
	virtual const Glyph& findGlyph(GlyphID glyph_id) const = 0;
	virtual const Glyph& getGlyph(GlyphID glyph_id) = 0;
	virtual const Glyph& findGlyphByCodepoint(char32_t codepoint) const = 0;
	virtual const Glyph& getGlyphByCodepoint(char32_t codepoint) = 0;

	FontFormat       format;
	ref<FontManager> manager;
	float            unitsPerEM;
};

VERA_PRIV_NAMESPACE_END
VERA_NAMESPACE_END
