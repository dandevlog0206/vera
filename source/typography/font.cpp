#include "../../include/vera/typography/font.h"

#include "../../include/vera/core/exception.h"
#include "../../include/vera/typography/font_manager.h"
#include "../../include/vera/util/ranged_set.h"
#include "font_impl.h"
#include <filesystem>
#include <fstream>
#include <vector>

VERA_NAMESPACE_BEGIN

array_view<CodeRange> Font::getDefaultCodeRanges() VERA_NOEXCEPT
{
	static const CodeRange default_ranges[] = {
		CodeRange(UnicodeRange::BasicLatin)
	};

	return array_view<CodeRange>(default_ranges);
}

Font::Font() VERA_NOEXCEPT :
	m_impl(nullptr) {}

obj<Font> Font::create() VERA_NOEXCEPT
{
	return obj<Font>(new Font);
}

obj<Font> Font::create(std::string_view path)
{
	obj<Font> font = obj<Font>(new Font());

	FontResult result = font->load(path);

	if (result.result() != FontResultType::Success)
		throw Exception(result.what());

	return font;
}

Font::~Font() VERA_NOEXCEPT
{
	delete m_impl;
}

FontResult Font::load(std::string_view path) VERA_NOEXCEPT
{
	if (m_impl != nullptr)
		return FontResultType::AlreadyLoaded;

	std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
	
	if (!file.is_open())
		return FontResultType::FailedToOpenFile;

	std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));

	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(data.data()), data.size());
	file.close();

	std::string ext = std::filesystem::path(path).extension().string();

	if (ext == ".ttc") {
		uint32_t          font_count;
		OTFResult         result;
		OTFFontCollection font_collection;

		result = otf_get_ttc_font_count(data.data(), data.size(), &font_count);

		if (result != OTFResultType::Success)
			return get_font_result(result);
		if (1 < font_count)
			return { FontResultType::TooManyFontsInFile,
				"too many fonts in TTC file, consider using FontManager for collection of fonts" };

		result = otf_load_ttc(font_collection, data.data(), data.size());

		if (result != OTFResultType::Success)
			return get_font_result(result);

		m_impl       = new priv::FontImpl();
		m_impl->font = std::move(font_collection.fonts[0]);

		init_font_impl(*m_impl, nullptr);

		return FontResultType::Success;
	} else if (ext == ".ttf") {
		OTFResult result;
		OTFFont   font;

		result = otf_load_ttf(font, data.data(), data.size());

		if (result != OTFResultType::Success)
			return get_font_result(result);

		m_impl       = new priv::FontImpl();
		m_impl->font = std::move(font);

		init_font_impl(*m_impl, nullptr);

		return FontResultType::Success;
	} else {
		return FontResultType::UnsupportedFormat;
	}
}

std::string_view Font::getName() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl != nullptr, "font is not loaded");
	return m_impl->fontName;
}

FontResult Font::loadGlyphRange(const basic_range<uint32_t>& range) VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl != nullptr, "font is not loaded");
	return get_font_result(otf_load_glyph_range(m_impl->font, range.first(), range.last() - 1));
}

FontResult Font::loadCodeRange(const CodeRange& range) VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl != nullptr, "font is not loaded");
	return get_font_result(otf_load_code_range(m_impl->font, range.start(), range.end()));
}

uint32_t Font::getGlyphCount() const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl != nullptr, "font is not loaded");
	return static_cast<uint32_t>(m_impl->font.maxProfile.numGlyphs);
}

const Glyph* Font::getGlyph(GlyphID glyph_id) const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl != nullptr, "font is not loaded");

	const auto& glyph_map = m_impl->font.glyphs;

	if (auto glyph_it = glyph_map.find(glyph_id); glyph_it != glyph_map.cend())
		return &glyph_it->second;

	return nullptr;
}

const Glyph* Font::findGlyph(char32_t codepoint) const VERA_NOEXCEPT
{
	VERA_ASSERT_MSG(m_impl != nullptr, "font is not loaded");

	const auto& char_map = m_impl->font.characterMap.charToGlyphMap;

	if (auto it = char_map.find(codepoint); it != char_map.cend()) {
		const auto& glyph_map = m_impl->font.glyphs;
	
		if (auto glyph_it = glyph_map.find(it->second); glyph_it != glyph_map.cend())
			return &glyph_it->second;
	}

	return nullptr;
}

VERA_NAMESPACE_END
