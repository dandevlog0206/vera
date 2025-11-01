#include <vera/vera.h>

#include <vera/vera.h>
#include <chrono>
#include <thread>
#include <fstream>

using namespace std;

static vr::basic_range<vr::GlyphID> get_cmdline_glyph_range()
{
	if (__argc >= 3) {
		return { (uint32_t)atoi(__argv[1]), (uint32_t)atoi(__argv[2]) };
	} else {
		return { 0, 100 };
	}
}

class MyApp
{
public:
	MyApp() :
		m_time(0.f),
		m_pause(false),
		m_exit(false)
	{
		m_context = vr::Context::create(vr::ContextCreateInfo{
			.enableValidation = VERA_IS_DEBUG,
			.enableDebugUtils = VERA_IS_DEBUG
		});

		m_device = vr::Device::create(m_context, vr::DeviceCreateInfo{
			.deviceID = m_context->findDeviceByType(vr::DeviceType::Discrete)
		});

		auto font_manager = vr::FontManager::create();
		font_manager->addCodeRange(vr::UnicodeRange::HangulJamo);
		font_manager->addCodeRange(vr::UnicodeRange::HangulSyllables);
		font_manager->loadFont("C:\\Windows\\Fonts\\consola.ttf");
		font_manager->loadFont("C:\\Windows\\Fonts\\batang.ttc");

		vr::FontAtlasCreateInfo atlas_info = {
			.font        = font_manager->getFonts()[1],
			.type        = vr::AtlasType::SDF,
			.sdfFontSize = 48,
			.atlasWidth  = 4096,
			.atlasHeight = 4096,
			.padding     = 5
		};

		m_font_atlas = vr::FontAtlas::create(m_device, atlas_info);

		// vr::basic_range<vr::GlyphID> glyph_range = get_cmdline_glyph_range();
		// vr::basic_range<vr::GlyphID> glyph_range = {0, font->getGlyphCount()};
		vr::basic_range<vr::GlyphID> glyph_range = { 0, 39680 };
		// vr::basic_range<vr::GlyphID> glyph_range = { 0, 100 };
		
		vr::RenderDoc::startFrameCapture();

		m_font_atlas->loadGlyphRange(glyph_range, 32);
		m_device->waitIdle();

		vr::RenderDoc::endFrameCapture();

		return;

		m_window.create(1080, 720, "Font Rendering Example");
		m_window.UserPtr = this;
		m_window.registerEventHandler(eventHandler);

		m_render_ctx = vr::RenderContext::create(m_device);

		m_swapchain = vr::Swapchain::create(m_device, m_window, vr::SwapchainCreateInfo{
			.presentMode = vr::PresentMode::Immediate
		});

		m_pass = std::make_unique<vr::GraphicsPass>(m_device, vr::GraphicsPassCreateInfo{
			.vertexShader   = vr::Shader::create(m_device, "shader/font_rendering.vert.glsl.spv"),
			.fragmentShader = vr::Shader::create(m_device, "shader/font_rendering.frag.glsl.spv"),
			.vertexCount = 6
		});

		auto fly_cam = std::make_unique<vr::Flycam>(vr::float3(0.f, 100.f, 0.f), vr::float3(10.f, 10.f, 0.f));
		fly_cam->setVelocity(10.f);
		m_camera = std::move(fly_cam);

		m_window.Focussed = true;
	}

	static void eventHandler(vr::os::Window& window, const vr::os::WindowEvent& e)
	{
		MyApp& app = *reinterpret_cast<MyApp*>(window.UserPtr.get());

		switch (e.type()) {
		case vr::os::WindowEventType::Close:
			app.m_exit = true;
			break;
		case vr::os::WindowEventType::Resize:
		case vr::os::WindowEventType::Move:
			app.drawFrame();
			break;
		}

		app.m_camera->updateEvent(e, app.m_timer.dt());
	}

	void updateAsync(float dt)
	{
		m_camera->updateAsync(dt);
		m_time += m_pause ? 0.f : dt;
	}

	void run()
	{
		while (!m_exit) {
			updateAsync(m_timer.dt());
			m_window.handleEvent();
			m_timer.update();

			drawFrame();
			this_thread::sleep_for(3ms);
		}

		m_device->waitIdle();
	}

	void drawFrame()
	{
		auto image = m_swapchain->acquireNextImage();

		float width  = static_cast<float>(image->width());
		float height = static_cast<float>(image->height());

		auto& params = m_pass->getShaderParameter();
		params["pc"]["resolution"] = vr::float2(width, height);
		params["pc"]["position"]   = m_camera->getPosition();
		params["pc"]["direction"]  = m_camera->getDirection();
		params["pc"]["up"]         = m_camera->getUp();
		params["atlasTexture"]     = m_font_atlas->getTextureView(32, 0);

		m_pass->execute(m_render_ctx, image);

		m_render_ctx->submit();
		m_swapchain->present();
	}

private:
	vr::obj<vr::Context>              m_context;
	vr::obj<vr::Device>               m_device;
	vr::obj<vr::RenderContext>        m_render_ctx;
	vr::obj<vr::Swapchain>            m_swapchain;
	vr::obj<vr::FontAtlas>            m_font_atlas;
	std::unique_ptr<vr::GraphicsPass> m_pass;

	vr::os::Window                    m_window;
	vr::Timer                         m_timer;
	std::unique_ptr<vr::Camera>       m_camera;
	float                             m_time;
	bool                              m_pause;
	bool                              m_exit;
};

int main()
{
	vr::RenderDoc::init();

	try {
		MyApp app;
		// app.run();
	} catch (const std::exception& e) {
		vr::Logger::exception(e.what());
	}

	vr::RenderDoc::destroy();

	return 0;
}
