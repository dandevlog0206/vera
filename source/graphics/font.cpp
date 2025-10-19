#include "../../include/vera/graphics/font.h"

#include "../../include/vera/core/assertion.h"
#include "detail/open_type.h"
#include <fstream>
#include <filesystem>
#include <vector>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

struct FontData
{

};

VERA_PRIV_NAMESPACE_END

static FontResult get_font_result(OTFResult result)
{
	const char* msg = result.what().data();

	switch (result.result()) {
	case OTFResultType::Success:
		return { FontResultType::Success, msg };
	case OTFResultType::InvalidID:
		return { FontResultType::InvalidID, msg };
	case OTFResultType::InvalidFormat:
		return { FontResultType::InvalidFormat, msg };
	}

	VERA_ASSERT_MSG(false, "invalid otf result type");
	return {};
}

Font::Font() VERA_NOEXCEPT
	: m_data(nullptr) {}

Font::Font(std::string_view path) VERA_NOEXCEPT
	: m_data(nullptr)
{
	load(path);
}

Font::Font(const Font& other) VERA_NOEXCEPT
	: m_data(other.m_data) {}

Font::Font(Font&& other) VERA_NOEXCEPT
	: m_data(other.m_data)
{
	other.m_data = nullptr;
}

Font::~Font() VERA_NOEXCEPT
{
	delete m_data;
}

Font& Font::operator=(const Font& other) VERA_NOEXCEPT
{
	if (this != &other)
	{
		delete m_data;
		m_data = other.m_data;
	}
	return *this;
}

Font& Font::operator=(Font&& other) VERA_NOEXCEPT
{
	if (this != &other)
	{
		delete m_data;
		m_data = other.m_data;
		other.m_data = nullptr;
	}
	return *this;
}

FontResult Font::load(std::string_view path) VERA_NOEXCEPT
{
	std::string ext = std::filesystem::path(path).extension().string();

	std::ifstream file(path.data(), std::ios::binary | std::ios::ate);
	
	if (!file.is_open())
		return FontResultType::FailedToOpenFile;

	std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(data.data()), data.size());
	file.close();

	if (ext == ".ttc") {
		OTFFontCollection font_collection;

		OTFResult result = otf_load_ttc(font_collection, data.data(), data.size());

		return get_font_result(result);
	} else if (ext == ".ttf") {
		OTFFont font;

		OTFResult result = otf_load_ttf(font, data.data(), data.size());

		return get_font_result(result);
	} else if (ext == ".otf") {
		return FontResultType::UnsupportedFormat;
	} else {
		return FontResultType::UnsupportedFormat;
	}
}

FontResult Font::getLastResult() const VERA_NOEXCEPT
{
	return m_result;
}

VERA_NAMESPACE_END
