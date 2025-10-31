#include "../../include/vera/typography/font_manager.h"

#include "font_impl.h"
#include <filesystem>
#include <fstream>

VERA_NAMESPACE_BEGIN

static OTFResult try_load_glyph_ranges(priv::FontImpl& impl)
{
	for (const auto& range : impl.manager->getCodeRanges())
		OTF_CHECK(otf_try_load_code_range(impl.font, range.start(), range.end()));

	return OTFResultType::Success;
}

obj<FontManager> FontManager::create()
{
	return obj<FontManager>(new FontManager());
}

FontManager::FontManager() VERA_NOEXCEPT :
	m_fonts(),
	m_code_ranges(VERA_SPAN(Font::getDefaultCodeRanges())) {}

FontManager::~FontManager()
{

}

FontResult FontManager::load(std::string_view path) VERA_NOEXCEPT
{
	std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
	
	if (!file.is_open())
		return FontResultType::FailedToOpenFile;

	std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));

	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(data.data()), data.size());
	file.close();

	std::string ext = std::filesystem::path(path).extension().string();

	if (ext == ".ttc") {
		std::vector<obj<Font>> new_fonts;
		uint32_t               font_count;
		OTFResult              result;
		OTFFontCollection      font_collection;

		result = otf_get_ttc_font_count(data.data(), data.size(), &font_count);
		if (result != OTFResultType::Success)
			return get_font_result(result);

		result = otf_load_ttc(font_collection, data.data(), data.size());
		if (result != OTFResultType::Success)
			return get_font_result(result);

		for (const auto& font : font_collection.fonts) {
			auto new_obj = obj<Font>(new Font);
			
			if (!new_obj)
				return FontResultType::AllocationFailed;
		
			new_obj->m_impl       = new priv::FontImpl();
			new_obj->m_impl->font = font;

			init_font_impl(*new_obj->m_impl, this);

			result = try_load_glyph_ranges(*new_obj->m_impl);
			if (result != OTFResultType::Success)
				return get_font_result(result);
			if (m_fonts.contains(std::string(new_obj->getName())))
				return FontResultType::AlreadyLoaded;

			new_fonts.emplace_back(new_obj);
		}

		for (auto& font : new_fonts)
			m_fonts.emplace(font->getName(), font);

		return FontResultType::Success;
	} else if (ext == ".ttf") {
		OTFResult result;
		OTFFont   font;

		result = otf_load_ttf(font, data.data(), data.size());

		if (result != OTFResultType::Success)
			return get_font_result(result);

		auto new_obj = obj<Font>(new Font);
		if (!new_obj)
			return FontResultType::AllocationFailed;

		new_obj->m_impl       = new priv::FontImpl();
		new_obj->m_impl->font = std::move(font);

		init_font_impl(*new_obj->m_impl, this);

		result = try_load_glyph_ranges(*new_obj->m_impl);
		if (result != OTFResultType::Success)
			return get_font_result(result);

		m_fonts.emplace(new_obj->getName(), new_obj);

		return FontResultType::Success;
	} else {
		return FontResultType::UnsupportedFormat;
	}
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
			return existing_range.contains(range);
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
