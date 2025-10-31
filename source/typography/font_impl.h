#pragma once

#include "../../include/vera/core/intrusive_ptr.h"
#include "../../include/vera/typography/font.h"
#include "open_type.h"

VERA_NAMESPACE_BEGIN

class FontManager;

VERA_PRIV_NAMESPACE_BEGIN

struct FontImpl
{
	std::string               fontName;
	OTFFont                   font;

	ref<FontManager>          manager;
};

VERA_PRIV_NAMESPACE_END

static FontResult get_font_result(OTFResult result)
{
	const char* msg = result.what().data();

	switch (result.result()) {
	case OTFResultType::Success:
		return { FontResultType::Success, msg };
	case OTFResultType::MissingTable:
	case OTFResultType::InvalidID:
	case OTFResultType::InvalidFormat:
	case OTFResultType::InvalidMagic:
	case OTFResultType::InvalidSize:
	case OTFResultType::InvalidIndex:
	case OTFResultType::InvalidRange:
		return { FontResultType::InvalidFormat, msg };
	case OTFResultType::Unsupported:
		return { FontResultType::UnsupportedFeature, msg };
	case OTFResultType::MissingCodepoint:
		return { FontResultType::MissingCodepoint, msg };
	case OTFResultType::MissingGlyph:
		return { FontResultType::MissingGlyph, msg };
	case OTFResultType::OutOfBounds:
		return { FontResultType::OutOfBounds, msg };
	}

	VERA_ASSERT_MSG(false, "invalid otf result type");
	return {};
}

static void init_font_impl(priv::FontImpl& impl, ref<FontManager> manager)
{
	for (const auto& entry : impl.font.nameTable.names) {
		const auto& name = entry.names[static_cast<size_t>(OTFNameID::FullFontName)];

		if (!name.empty()) {
			impl.fontName = name;
			break;
		}
	}

	impl.manager = manager;
}

VERA_NAMESPACE_END
