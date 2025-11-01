#pragma once

#include "../core/intrusive_ptr.h"
#include "../util/array_view.h"
#include "code_range.h"
#include "glyph.h"
#include <string_view>
#include <memory>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

class FontImplBase;

VERA_PRIV_NAMESPACE_END

enum class FontFormat VERA_ENUM
{
	Unknown,
	TrueType,
	TrueTypeCollection,
	OpenType,
	SVG
};

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

	void load(std::string_view path);
	
	VERA_NODISCARD FontFormat getFormat() const VERA_NOEXCEPT;
	VERA_NODISCARD std::string_view getName() const VERA_NOEXCEPT;

	void loadAllGlyphs();
	void loadGlyphRange(const basic_range<uint32_t>& range);
	void loadCodeRange(const CodeRange& range);

	VERA_NODISCARD uint32_t getGlyphCount() const VERA_NOEXCEPT;
	VERA_NODISCARD GlyphID getGlyphID(char32_t codepoint) const;
	VERA_NODISCARD const Glyph& findGlyph(GlyphID glyph_id) const;
	VERA_NODISCARD const Glyph& getGlyph(GlyphID glyph_id);
	VERA_NODISCARD const Glyph& findGlyphByCodepoint(char32_t codepoint) const;
	VERA_NODISCARD const Glyph& getGlyphByCodepoint(char32_t codepoint);

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	std::unique_ptr<priv::FontImplBase> m_impl;
};

VERA_NAMESPACE_END
