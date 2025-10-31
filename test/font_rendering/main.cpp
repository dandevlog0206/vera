#include <vera/vera.h>
#include <thread>

struct Vertex
{
	vr::float2 pos;
	vr::float4 color;
	float      attr;

	VERA_VERTEX_DESCRIPTOR_BEGIN(Vertex)
		VERA_VERTEX_ATTRIBUTE(0, pos),
		VERA_VERTEX_ATTRIBUTE(1, color),
		VERA_VERTEX_ATTRIBUTE(2, attr),
	VERA_VERTEX_DESCRIPTOR_END
};

int main(int argc, const char* argv[])
{
	vr::RenderDoc::init();

	vr::ContextCreateInfo context_info = {
		.enableValidation = true,
		.enableDebugUtils = true
	};

	auto context = vr::Context::create(context_info);
	auto device  = vr::Device::create(context);

	auto font_manager = vr::FontManager::create();
	font_manager->addCodeRange(vr::UnicodeRange::HangulJamo);
	font_manager->addCodeRange(vr::UnicodeRange::HangulSyllables);
	font_manager->load("C:\\Windows\\Fonts\\consola.ttf");
	font_manager->load("C:\\Windows\\Fonts\\batang.ttc");

	auto font = font_manager->getFonts()[1];

	vr::FontAtlasCreateInfo atlas_info = {
		.font        = font,
		.type        = vr::AtlasType::SDF,
		.sdfFontSize = 48,
		.atlasWidth  = 4096,
		.atlasHeight = 4096,
		.padding     = 5
	};

	vr::RenderDoc::startFrameCapture();

	auto atlas = vr::FontAtlas::create(device, atlas_info);

	// vr::basic_range<vr::GlyphID> glyph_range = { (uint32_t)atoi(argv[1]), (uint32_t)atoi(argv[2]) };
	vr::basic_range<vr::GlyphID> glyph_range = { 0, font->getGlyphCount() };
	// vr::basic_range<vr::GlyphID> glyph_range = { 0, 39680 };
	// vr::basic_range<vr::GlyphID> glyph_range = { 0, 100 };

	atlas->loadGlyphRange(glyph_range, 32);

	vr::RenderDoc::endFrameCapture();
	vr::RenderDoc::destroy();

	device->waitIdle();

	return 0;
}