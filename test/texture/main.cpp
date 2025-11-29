#include <vera/vera.h>
#include <thread>

using namespace std;

struct Vertex
{
	vr::float2 pos;
	vr::float4 color;
	vr::float2 uv;

	VERA_VERTEX_DESCRIPTOR_BEGIN(Vertex)
		VERA_VERTEX_ATTRIBUTE(0, pos),
		VERA_VERTEX_ATTRIBUTE(1, color),
		VERA_VERTEX_ATTRIBUTE(2, uv),
	VERA_VERTEX_DESCRIPTOR_END
};

class MyApp
{
public:
	MyApp() :
		m_window(1080, 720, "Texture Demo")
	{
		m_window.UserPtr = this;
		m_window.registerEventHandler(eventHandler);

		m_context = vr::Context::create(vr::ContextCreateInfo{
			.enableValidation = true,
			.enableDebugUtils = true
		});

		m_device = vr::Device::create(m_context, vr::DeviceCreateInfo{
			.deviceID = m_context->findDeviceByType(vr::DeviceType::Discrete)
		});

		m_render_ctx = vr::RenderContext::create(m_device);

		m_swapchain = vr::Swapchain::create(m_device, m_window, vr::SwapchainCreateInfo{
			.presentMode = vr::PresentMode::Immediate
		});

		m_pass = vr::GraphicsPass::create(m_device, vr::GraphicsPassCreateInfo{
			.vertexShader    = vr::Shader::create(m_device, "spirv/default.vert.glsl.spv"),
			.fragmentShader  = vr::Shader::create(m_device, "spirv/default.frag.glsl.spv"),
			.vertexInput     = VERA_REFLECT_VERTEX(Vertex),
			.vertexCount     = 6
		});

		loadImages();

		m_window.Focussed = true;
	}

	static void eventHandler(vr::os::Window& window, const vr::os::WindowEvent& e)
	{
		MyApp& app = *reinterpret_cast<MyApp*>(window.UserPtr.get());

		switch (e.type()) {
		case vr::os::WindowEventType::Resize:
		case vr::os::WindowEventType::Move:
			app.drawFrame();
			break;
		}
	}

	int run()
	{
		while (!m_window.needClose()) {
			m_window.handleEvent();
			drawFrame();
		}

		m_device->waitIdle();

		return 0;
	}

	void drawFrame()
	{
		if (m_swapchain->isOccluded()) return;

		auto image    = m_swapchain->acquireNextImage();
		auto time     = m_timer.elapsed();
		auto viewport = vr::float2(
			static_cast<float>(image->width()),
			static_cast<float>(image->height()));

		auto root_var = m_pass->getRootVariable();
		root_var["pc"]["viewport"] = viewport;
		root_var["pc"]["time"]     = time;

		for (uint32_t i = 0; i < 10; ++i) {
			auto image_pos = vr::float2(
				cosf(time / 3.f + vr::to_radian(i * 36.f)) * 300.f,
				sinf(time / 3.f + vr::to_radian(i * 36.f)) * 300.f
			);
			
			auto mat = vr::Transform2D()
				.translate(viewport / 2.f)
				.translate(image_pos)
				.translate(-50, -50)
				.getMatrix();

			root_var["pc"]["transform"] = vr::to_col_major(mat);
			root_var["Texture"]         = m_textures[i];

			m_pass->execute(m_render_ctx, image);
		}

		m_render_ctx->submit();
		m_swapchain->present();
	}

private:
	void loadImages()
	{
		auto  root_var = m_pass->getRootVariable();
		auto  center   = vr::float2{ 1080 / 2.f, 720 / 2.f };

		for (uint32_t i = 0; i < 10; ++i) {
			auto path    = std::format("resource/test/{}.png", i);
			auto image   = vr::Image::loadFromFile(path);
			auto texture = vr::Texture::create(m_device, image);

			m_textures.push_back(vr::TextureView::create(std::move(texture)));
		}

		auto  vertex_memory = m_pass->getVertexBuffer()->getDeviceMemory();
		auto* map           = reinterpret_cast<Vertex*>(vertex_memory->map());

		auto v     = vr::rect<float>{ 0.f, 0.f, 100.f, 100.f };
		auto uv    = vr::rect<float>{ 0.f, 0.f, 1.f, 1.f };
		auto color = vr::Color(vr::Colors::White).unorm();

		map[0] = { v[0], color, uv[0] };
		map[1] = { v[1], color, uv[1] };
		map[2] = { v[2], color, uv[2] };
		map[3] = { v[0], color, uv[0] };
		map[4] = { v[2], color, uv[2] };
		map[5] = { v[3], color, uv[3] };
	}

private:
	using Textures = std::vector<vr::obj<vr::TextureView>>;

	vr::obj<vr::Context>       m_context;
	vr::obj<vr::Device>        m_device;
	vr::obj<vr::RenderContext> m_render_ctx;
	vr::obj<vr::Swapchain>     m_swapchain;
	vr::obj<vr::GraphicsPass>  m_pass;
	Textures                   m_textures;

	vr::os::Window             m_window;
	vr::Timer                  m_timer;
};

int main()
{
	try {
		MyApp app;
		app.run();
	} catch (const std::exception& e) {
		vr::Logger::exception(e.what());
	}

	return 0;
}