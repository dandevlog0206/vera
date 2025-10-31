#pragma once

#include "code_range.h"
#include "font.h"
#include <unordered_map>
#include <string>

VERA_NAMESPACE_BEGIN

class FontManager : public ManagedObject
{
	FontManager() VERA_NOEXCEPT;
public:
	VERA_NODISCARD static obj<FontManager> create();
	~FontManager();

	FontResult load(std::string_view path) VERA_NOEXCEPT;

	void addFont(obj<Font> font) VERA_NOEXCEPT;

	VERA_NODISCARD obj<Font> getFont(std::string_view name) VERA_NOEXCEPT;
	VERA_NODISCARD std::vector<obj<Font>> getFonts() VERA_NOEXCEPT;
	VERA_NODISCARD size_t getFontCount() const VERA_NOEXCEPT;

	VERA_NODISCARD array_view<CodeRange> getCodeRanges() const VERA_NOEXCEPT;
	bool addCodeRange(const CodeRange& range) VERA_NOEXCEPT;

	void clear() VERA_NOEXCEPT;

	VERA_NODISCARD bool empty() const VERA_NOEXCEPT;

private:
	using FontMap = std::unordered_map<std::string, obj<Font>>;

	FontMap                m_fonts;
	std::vector<CodeRange> m_code_ranges;
};

VERA_NAMESPACE_END
