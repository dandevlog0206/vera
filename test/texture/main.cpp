#include <vera/vera.h>
#include <chrono>
#include <thread>

using namespace std;

random_device                    rd;
mt19937                          rnd(rd());
uniform_real_distribution<float> pos_dist(-1.f, 1.f);
uniform_real_distribution<float> color_dist(0.f, 1.f);

struct Vertex
{
	vr::float2 pos;
	vr::float4 color;
	vr::float2 uv;

	VERA_VERTEX_DESCRIPTOR_BEGIN(Vertex)
		VERA_VERTEX_ATTRIBUTE(0, pos),
		VERA_VERTEX_ATTRIBUTE(1, color),
		VERA_VERTEX_ATTRIBUTE(1, uv),
	VERA_VERTEX_DESCRIPTOR_END
};

static vr::float2 get_random_pos()
{
	return { pos_dist(rnd), pos_dist(rnd) };
}

static vr::float4 get_random_color()
{
	return { color_dist(rnd), color_dist(rnd), color_dist(rnd), 1.f };
}

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
		m_window(1080, 720, "Texture Demo"),
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
			.vertexShader    = vr::Shader::create(m_device, "shaders/default.vert.glsl.spv"),
			.fragmentShader  = vr::Shader::create(m_device, "shaders/default.frag.glsl.spv"),
			.vertexInput     = VERA_REFLECT_VERTEX(Vertex),
			.vertexCount     = 36
		});

		auto image  = vr::Image::loadFromFile("resource/vulkan.png");
		
		vr::ImageSampler sampler(vr::ImageSamplerCreateInfo{
			.filter                  = vr::ImageSamplerFilter::Linear,
			.addressModeU            = vr::ImageSamplerAddressMode::ClampToBorder,
			.addressModeV            = vr::ImageSamplerAddressMode::ClampToBorder,
			.borderColor             = { 1.f, 0.f, 1.f, 1.f },
			.unnormalizedCoordinates = false,
		});

		//image = vr::ImageEdit::createMask(image, 0, 0, 0.f, 0.99f);
		//image = vr::ImageEdit::blit(image, sampler, vr::ImageBlitInfo{
		//	.dstWidth  = image.width(),
		//	.dstHeight = image.height(),
		//	.uv0       = { -2.f, -2.f },
		//	.uv1       = { 2.f, -2.f },
		//	.uv2       = { 2.f, 2.f },
		//	.uv3       = { -2.f, 2.f }
		//});

		m_texture = vr::Texture::create(m_device, vr::TextureCreateInfo{
			.format = image.format(),
			.width  = image.width(),
			.height = image.height()
		});

		m_texture->upload(image);

		m_pass->getShaderParameter()["sTexture"] = m_texture;

		auto  vertex_memory = m_pass->getVertexBuffer()->getDeviceMemory();
		auto* map           = reinterpret_cast<Vertex*>(vertex_memory->map());
		auto  aspect        = image.width() / image.height();

		for (uint32_t i = 0; i < 36; i += 6) {
			vr::float2 v0  = { 0, 0};
			vr::float2 v1  = { 500, 0 };
			vr::float2 v2  = { 500, 500 * aspect };
			vr::float2 v3  = { 0, 500 * aspect };
			vr::float2 uv0 = { 0.0, 0.0 };
			vr::float2 uv1 = { 1.0, 0.0 };
			vr::float2 uv2 = { 1.0, 1.0 };
			vr::float2 uv3 = { 0.0, 1.0 };

			auto offset = 500.f * get_random_pos();
			auto color  = vr::Color(vr::Colors::White).unorm();

			map[i + 0].pos   = v0 + offset;
			map[i + 0].color = color;
			map[i + 0].uv    = uv0;
			map[i + 1].pos   = v1 + offset;
			map[i + 1].color = color;
			map[i + 1].uv    = uv1;
			map[i + 2].pos   = v2 + offset;
			map[i + 2].color = color;
			map[i + 2].uv    = uv2;
			map[i + 3].pos   = v0 + offset;
			map[i + 3].color = color;
			map[i + 3].uv    = uv0;
			map[i + 4].pos   = v2 + offset;
			map[i + 4].color = color;
			map[i + 4].uv    = uv2;
			map[i + 5].pos   = v3 + offset;
			map[i + 5].color = color;
			map[i + 5].uv    = uv3;
		}

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
	}

	int run()
	{
		while (!m_exit) {
			m_window.handleEvent();
			drawFrame();
			this_thread::sleep_for(3ms);
		}

		m_device->waitIdle();

		return 0;
	}

	void drawFrame()
	{
		if (m_swapchain->isOccluded()) return;

		auto  image = m_swapchain->acquireNextImage();
		float time  = elapsed_s();

		auto mat = vr::Transform2D().translate(1080 / 2, 720 / 2).rotate(time).translate(-250, -250).getMatrix();
		//auto mat = vr::Transform2D().translate(sinf(time), sinf(time)).getMatrix();
		//auto mat = vr::Transform2D().getMatrix();

		auto& params = m_pass->getShaderParameter();
		params["pc"]["viewport"]  = vr::float2(image->width(), image->height());
		params["pc"]["time"]      = time; // for block variable
		params["pc"]["transform"] = mat;
		params["pc"]["colors"][0] = vr::Colormaps::turbo(abs(fmodf(0.5f * time + 0.05f, 1.9f) - 1.f)).unorm();
		params["pc"]["colors"][1] = vr::Colormaps::turbo(abs(fmodf(0.5f * time + 0.71666f, 1.9f) - 1.f)).unorm();
		params["pc"]["colors"][2] = vr::Colormaps::turbo(abs(fmodf(0.5f * time + 1.38333f, 1.9f) - 1.f)).unorm();

		m_pass->execute(m_render_ctx, image);

		m_render_ctx->submit();
		m_swapchain->present();
	}

private:
	vr::obj<vr::Context>              m_context;
	vr::obj<vr::Device>               m_device;
	vr::obj<vr::RenderContext>        m_render_ctx;
	vr::obj<vr::Swapchain>            m_swapchain;
	vr::obj<vr::Texture>              m_texture;
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