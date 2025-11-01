#include "../../include/vera/typography/font.h"

#include "../../include/vera/core/exception.h"
#include "open_type.h"
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

	font->load(path);

	return font;
}

Font::~Font() VERA_NOEXCEPT
{
	// nothing to do
}

void Font::load(std::string_view path)
{
	if (m_impl != nullptr)
		throw Exception("font is already loaded");

	std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
	
	if (!file.is_open())
		throw Exception("failed to open font file");

	std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));

	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(data.data()), data.size());
	file.close();

	std::string ext = std::filesystem::path(path).extension().string();

	if (ext == ".ttc") {
		auto offsets = OpenTypeImpl::getTTCFontOffsets(data.data(), data.size());

		if (offsets.empty())
			throw Exception("no fonts found in TTC file");
		if (offsets.size() > 1)
			throw Exception("too many fonts in TTC file, consider using FontManager for collection of fonts");

		m_impl = std::make_unique<OpenTypeImpl>(data.data(), data.size(), offsets.front());
		m_impl->format  = FontFormat::TrueTypeCollection;
		m_impl->manager = {};
	} else if (ext == ".ttf" || ext == ".otf") {
		m_impl = std::make_unique<OpenTypeImpl>(data.data(), data.size(), 0);
		m_impl->format  = FontFormat::TrueTypeCollection;
		m_impl->manager = {};
	} else {
		throw Exception("unsupported font format: {}", ext);
	}
}

FontFormat Font::getFormat() const VERA_NOEXCEPT
{
	return m_impl ? m_impl->format : FontFormat::Unknown;
}

std::string_view Font::getName() const VERA_NOEXCEPT
{
	return m_impl ? m_impl->getName() : std::string_view{};
}

void Font::loadAllGlyphs()
{
	if (!m_impl)
		throw Exception("font is not loaded");

	m_impl->loadAllGlyphs();
}

void Font::loadGlyphRange(const basic_range<uint32_t>& range)
{
	if (!m_impl)
		throw Exception("font is not loaded");

	m_impl->loadGlyphRange(range);
}

void Font::loadCodeRange(const CodeRange& range)
{
	if (!m_impl)
		throw Exception("font is not loaded");

	m_impl->loadCodeRange(range);
}

uint32_t Font::getGlyphCount() const VERA_NOEXCEPT
{
	return m_impl ? m_impl->getGlyphCount() : 0;
}

GlyphID Font::getGlyphID(char32_t codepoint) const
{
	if (!m_impl)
		throw Exception("font is not loaded");

	return m_impl->getGlyphID(codepoint);
}

const Glyph& Font::findGlyph(GlyphID glyph_id) const
{
	if (!m_impl)
		throw Exception("font is not loaded");

	return m_impl->findGlyph(glyph_id);
}

const Glyph& Font::getGlyph(GlyphID glyph_id)
{
	if (!m_impl)
		throw Exception("font is not loaded");

	return m_impl->getGlyph(glyph_id);
}

const Glyph& Font::findGlyphByCodepoint(char32_t codepoint) const
{
	if (!m_impl)
		throw Exception("font is not loaded");

	return m_impl->findGlyphByCodepoint(codepoint);
}

const Glyph& Font::getGlyphByCodepoint(char32_t codepoint)
{
	if (!m_impl)
		throw Exception("font is not loaded");

	return m_impl->getGlyphByCodepoint(codepoint);
}

bool Font::empty() const VERA_NOEXCEPT
{
	return m_impl == nullptr;
}

VERA_NAMESPACE_END
