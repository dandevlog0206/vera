#pragma once

#include "../core/texture.h"
#include "../core/command_buffer_sync.h"
#include "../util/rect_packer.h"
#include "font.h"
#include <unordered_map>

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

class FontAtlasGlobalResource;
class FontAtlasResource;
struct GlyphPage;

VERA_PRIV_NAMESPACE_END

enum class AtlasType VERA_ENUM
{
	HardMask,
	SoftMask,
	SDF,
	PSDF,
	MSDF,
	MTSDF
};

struct PackedGlyph
{
	GlyphID  glyphID;
	uint32_t px;
	uint32_t layer;
	AABB2D   rect;
};

struct FontAtlasCreateInfo
{
	ref<Font>     font                  = {};
	AtlasType     type                  = AtlasType::SDF;
	PackingMethod packingMethod         = PackingMethod::Shelf;
	uint32_t      sdfFontSize           = 0;                    // zero will use the font's default size
	uint32_t      sdfPadding            = 5;
	uint32_t      atlasWidth            = 2048;
	uint32_t      atlasHeight           = 2048;
	uint32_t      padding               = 2;
	bool          hasOverlappingContour = false;
};

class FontAtlas : public ManagedObject
{
	FontAtlas() VERA_NOEXCEPT = default;
public:
	VERA_NODISCARD static obj<FontAtlas> create(obj<Device> device, const FontAtlasCreateInfo& info = {});
	~FontAtlas() VERA_NOEXCEPT;

	VERA_NODISCARD ref<Font> getFont() const VERA_NOEXCEPT;
	VERA_NODISCARD const_ref<Texture> getTexture(uint32_t px, uint32_t layer) const VERA_NOEXCEPT;
	VERA_NODISCARD uint32_t getTextureCount(uint32_t px) const VERA_NOEXCEPT;
	VERA_NODISCARD extent2d getTextureSize() const VERA_NOEXCEPT;

	CommandBufferSync loadGlyphRange(const basic_range<GlyphID>& range, uint32_t px);
	CommandBufferSync loadCodeRange(const CodeRange& range, uint32_t px);

	VERA_NODISCARD const PackedGlyph& getGlyph(char32_t codepoint, uint32_t px);

	VERA_NODISCARD AtlasType getAtlasType() const VERA_NOEXCEPT;

private:
	obj<Device>                                   m_device;
	std::unique_ptr<priv::FontAtlasResource>      m_resource;
	std::unordered_map<uint32_t, priv::GlyphPage> m_pages;
	FontAtlasCreateInfo                           m_info;
};

VERA_NAMESPACE_END
