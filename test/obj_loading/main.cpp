#include <vera/vera.h>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;

static ostream& operator<<(ostream& os, const vr::float3& v)
{
	os << "<" << v.x << ", " << v.y << ", " << v.z << ">\n";
	return os;
}

struct Vertex3D
{
	vr::float3 pos;
	vr::float3 normal;
	vr::float2 uv;

	VERA_VERTEX_DESCRIPTOR_BEGIN(Vertex3D)
		VERA_VERTEX_ATTRIBUTE(0, pos),
		VERA_VERTEX_ATTRIBUTE(1, normal),
		VERA_VERTEX_ATTRIBUTE(2, uv)
	VERA_VERTEX_DESCRIPTOR_END
};

class MyApp
{
public:
	MyApp() :
		m_window(1080, 720, "OBJ Loading Demo")
	{
		m_window.UserPtr = this;
		m_window.registerEventHandler(eventHandler);

		m_context = vr::Context::create(vr::ContextCreateInfo{
			.enableValidation = VERA_IS_DEBUG,
			.enableDebugUtils = VERA_IS_DEBUG
		});

		m_device = vr::Device::create(m_context, vr::DeviceCreateInfo{
			.deviceID = m_context->findDeviceByType(vr::DeviceType::Discrete)
		});

		m_render_ctx = vr::RenderContext::create(m_device);

		m_swapchain = vr::Swapchain::create(m_device, m_window, vr::SwapchainCreateInfo{
			.presentMode = vr::PresentMode::Immediate
		});

		vr::ModelLoader loader("resource/viking room/viking_room.obj");
		vr::Image       image("resource/viking room/viking_room.png");

		m_pass = std::make_unique<vr::GraphicsPass>(m_device, vr::GraphicsPassCreateInfo{
			.vertexShader   = vr::Shader::create(m_device, "shader/obj_loading.vert.glsl.spv"),
			.fragmentShader = vr::Shader::create(m_device, "shader/obj_loading.frag.glsl.spv"),
			.vertexInput    = VERA_REFLECT_VERTEX(Vertex3D),
			.vertexCount    = static_cast<uint32_t>(loader.vertices.size()),
			.depthFormat    = vr::DepthFormat::D32Float
		});

		auto* map = reinterpret_cast<Vertex3D*>(m_pass->getVertexBuffer()->getDeviceMemory()->map());

		for (size_t i = 0; i < loader.vertices.size(); ++i) {
			map[i].pos    = loader.vertices[i];
			map[i].normal = loader.normals[i];
			map[i].uv     = loader.uvs[i];
		}

		auto texture = vr::Texture::create(m_device,
			vr::TextureCreateInfo{
				.format = image.format(),
				.width  = image.width(),
				.height = image.height()
			});

		texture->upload(image);

		m_pass->getShaderParameter()["sTexture"] = texture;

		m_camera = std::make_unique<vr::Flycam>(vr::float3(1.f, 0.f, 1.f), vr::float3(0.f, 0.f, 0.f));

		m_window.Focussed = true;
	}

	static void eventHandler(vr::os::Window& window, const vr::os::WindowEvent& e)
	{
		MyApp& app = *reinterpret_cast<MyApp*>(window.UserPtr.get());

		app.m_camera->updateEvent(e, app.m_timer.dt());

		switch (e.type()) {
		case vr::os::WindowEventType::Resize:
		case vr::os::WindowEventType::Move:
			app.drawFrame();
			break;
		}
	}

	void run()
	{
		while (!m_window.needClose()) {
			m_window.handleEvent();
			m_camera->updateAsync(m_timer.dt());

			drawFrame();
			m_timer.update();
			this_thread::sleep_for(3ms);
		}

		m_device->waitIdle();
	}

	void drawFrame()
	{
		if (m_swapchain->isOccluded()) return;
	
		auto proj = vr::perspective(vr::to_radian(45.f), m_window.getAspect(), 0.1f, 100.f);
		auto view = m_camera->getMatrix();

		auto& param = m_pass->getShaderParameter();
		param["pc"]["mat"] = proj * view;

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
	std::unique_ptr<vr::Camera>       m_camera;
	vr::Timer                         m_timer;
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
