#include <vera/vera.h>
#include <chrono>
#include <thread>

using namespace std;

static float elapsed_s()
{
	using namespace std::chrono;
	
	using clock_t = high_resolution_clock;

	static auto s_clock_begin = clock_t::now();

	return duration_cast<microseconds>(clock_t::now() - s_clock_begin).count() / 1e6f;
}

class MyApp
{
public:
	MyApp() :
		m_window(1080, 720, "Triangle Demo"),
		m_exit(false)
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

		m_swapchain = vr::Swapchain::create(m_render_ctx, m_window, vr::SwapchainCreateInfo{
			.presentMode = vr::PresentMode::Immediate
		});

		m_pass = std::make_unique<vr::GraphicsPass>(m_device, vr::GraphicsPassCreateInfo{
			.vertexShader   = vr::Shader::create(m_device, "shaders/triangle.vert.glsl.spv"),
			.fragmentShader = vr::Shader::create(m_device, "shaders/triangle.frag.glsl.spv"),
			.vertexCount    = 3
		});

		m_window.Focussed = true;
	}

	static void eventHandler(vr::os::Window& window, const vr::os::WindowEvent& e)
	{
		MyApp& app = *reinterpret_cast<MyApp*>(window.UserPtr.get());

		switch (e.type()) {
		case vr::os::EventType::Close:
			app.m_exit = true;
			break;
		case vr::os::EventType::Resize:
		case vr::os::EventType::Move:
			app.drawFrame();
			break;
		}
	}

	void run()
	{
		while (!m_exit) {
			m_window.handleEvent();
			drawFrame();
			this_thread::sleep_for(30ms);
		}

		m_device->waitIdle();
	}

	void drawFrame()
	{
		if (m_swapchain->isOccluded()) return;

		float time   = elapsed_s();
		auto& params = m_pass->getShaderParameter();
		params["pc"]["scale"]     = std::abs(sinf(time) + 2) / 2;
		params["pc"]["colors"][0] = vr::Colormaps::turbo(abs(fmodf(0.5f * time + 0.05f, 1.9f) - 1.f)).unorm();
		params["pc"]["colors"][1] = vr::Colormaps::turbo(abs(fmodf(0.5f * time + 0.71666f, 1.9f) - 1.f)).unorm();
		params["pc"]["colors"][2] = vr::Colormaps::turbo(abs(fmodf(0.5f * time + 1.38333f, 1.9f) - 1.f)).unorm();

		m_pass->execute(m_render_ctx, m_swapchain->acquireNextImage());

		m_render_ctx->submit();
		m_swapchain->present();
	}

private:
	vr::obj<vr::Context>              m_context;
	vr::obj<vr::Device>               m_device;
	vr::obj<vr::RenderContext>        m_render_ctx;
	vr::obj<vr::Swapchain>            m_swapchain;
	std::unique_ptr<vr::GraphicsPass> m_pass;

	vr::os::Window                    m_window;

	bool                              m_exit;
};

int main()
{
	MyApp app;

	try {
		app.run();
	} catch (const std::exception& e) {
		vr::Logger::exception(e.what());
	}

	return 0;
}
