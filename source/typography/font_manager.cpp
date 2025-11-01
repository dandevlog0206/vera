#include "../../include/vera/typography/font_manager.h"

#include "../../include/vera/core/exception.h"
#include "open_type.h"
#include <filesystem>
#include <fstream>

VERA_NAMESPACE_BEGIN

obj<FontManager> FontManager::create()
{
	return obj<FontManager>(new FontManager());
}

FontManager::FontManager() VERA_NOEXCEPT :
	m_fonts(),
	m_code_ranges(VERA_SPAN(Font::getDefaultCodeRanges())) {}

FontManager::~FontManager()
{
	// nothing to do
}

void FontManager::loadFont(std::string_view path)
{
	std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
	
	if (!file.is_open())
		throw Exception("failed to open font file");

	std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));

	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(data.data()), data.size());
	file.close();

	std::string ext = std::filesystem::path(path).extension().string();

	if (ext == ".ttc") {
		std::vector<obj<Font>> new_fonts;

		auto offsets = OpenTypeImpl::getTTCFontOffsets(data.data(), data.size());

		if (offsets.empty())
			throw Exception("no fonts found in TTC file");

		for (const uint32_t offset : offsets) {
			auto& new_font = new_fonts.emplace_back(new Font);

			new_font->m_impl = std::make_unique<OpenTypeImpl>(data.data(), data.size(), offset);
			new_font->m_impl->format  = FontFormat::TrueTypeCollection;
			new_font->m_impl->manager = this;
		}

		for (auto& font : new_fonts)
			m_fonts.emplace(font->getName(), font);
	} else if (ext == ".ttf" || ext == ".otf") {
		auto new_font = obj<Font>(new Font);
		new_font->m_impl = std::make_unique<OpenTypeImpl>(data.data(), data.size(), 0);
		new_font->m_impl->format  = FontFormat::OpenType;
		new_font->m_impl->manager = this;

		m_fonts.emplace(new_font->getName(), new_font);
	} else {
		throw Exception("unsupported font format: {}", ext);
	}
}

void FontManager::addFont(obj<Font> font)
{
	if (!font)
		throw Exception("cannot add null font to FontManager");
	if (font->m_impl->manager)
		throw Exception("font is already managed by another FontManager");

	m_fonts.emplace(font->getName(), font);
}

obj<Font> FontManager::getFont(std::string_view name) VERA_NOEXCEPT
{
	auto it = m_fonts.find(std::string(name));

	if (it != m_fonts.end())
		return it->second;
	else
		return nullptr;
}

std::vector<obj<Font>> FontManager::getFonts() VERA_NOEXCEPT
{
	std::vector<obj<Font>> fonts;

	fonts.reserve(m_fonts.size());

	for (const auto& pair : m_fonts)
		fonts.push_back(pair.second);

	return fonts;
}

size_t FontManager::getFontCount() const VERA_NOEXCEPT
{
	return m_fonts.size();
}

array_view<CodeRange> FontManager::getCodeRanges() const VERA_NOEXCEPT
{
	return m_code_ranges;
}

bool FontManager::addCodeRange(const CodeRange& range) VERA_NOEXCEPT
{
	if (range.getUnicodeRange() == UnicodeRange::ALL) {
		m_code_ranges.clear();
		m_code_ranges.push_back(range);
		return true;
	}

	auto it = std::find_if(VERA_SPAN(m_code_ranges),
		[&range](const CodeRange& existing_range) {
			return existing_range.contain(range);
		});

	if (it == m_code_ranges.end()) {
		m_code_ranges.push_back(range);
		return true;
	}

	return false;
}

void FontManager::clear() VERA_NOEXCEPT
{
	m_fonts.clear();
}

bool FontManager::empty() const VERA_NOEXCEPT
{
	return m_fonts.empty();
}

VERA_NAMESPACE_END
