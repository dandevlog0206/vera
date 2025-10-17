#include <vera/vera.h>
#include <thread>
#include <random>
#include <iostream>

#define RESOURCE_PATH "resource/angry bot/"

using namespace std;

static vr::float2 rnd_float2()
{
	static std::minstd_rand rng(std::random_device{}());
	static std::uniform_real_distribution<float> dist(0.f, 1.f);

	return vr::float2(dist(rng), dist(rng));
}

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

void map_mesh_data(vr::GraphicsPass* pass, vr::ref<vr::scene::MeshAttribute> mesh_attr)
{
	auto* vtx_map = reinterpret_cast<Vertex3D*>(pass->getVertexBuffer()->getDeviceMemory()->map());
	auto* idx_map = reinterpret_cast<uint32_t*>(pass->getIndexBuffer()->getDeviceMemory()->map());

	auto& vertices = mesh_attr->getVertices();
	auto& indicies = mesh_attr->getIndices();

	for (size_t i = 0; i < vertices.size(); ++i) {
		vtx_map[i].pos    = vertices[i];
		vtx_map[i].normal = vr::float3();
		vtx_map[i].uv     = rnd_float2();
	}

	for (size_t i = 0; i < indicies.size(); ++i)
		idx_map[i] = static_cast<uint32_t>(indicies[i]);
}

class MyApp
{
public:
	MyApp() :
		m_window(1080, 720, "Angry Bot Demo")
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

		vr::Image image("resource/lsm.png");
		vr::AssetLoader loader;

		// loader.loadScene(RESOURCE_PATH"Models/Player/Player.fbx");
		loader.loadModel(RESOURCE_PATH"Models/BreakDance.fbx");

		auto model_node = loader.getScene()->getRootNode()->getChild("model");

		vr::ref<vr::scene::MeshAttribute> mesh_attr0 = model_node->getAttribute(0);
		vr::ref<vr::scene::MeshAttribute> mesh_attr1 = model_node->getAttribute(1);

		m_pass0 = std::make_unique<vr::GraphicsPass>(m_device, vr::GraphicsPassCreateInfo{
			.vertexShader   = vr::Shader::create(m_device, "shader/obj_loading.vert.glsl.spv"),
			.fragmentShader = vr::Shader::create(m_device, "shader/obj_loading.frag.glsl.spv"),
			.vertexInput    = VERA_REFLECT_VERTEX(Vertex3D),
			.vertexCount    = static_cast<uint32_t>(mesh_attr0->getVertices().size()),
			.indexType      = vr::IndexType::UInt32,
			.indexCount     = static_cast<uint32_t>(mesh_attr0->getIndices().size()),
			.depthFormat    = vr::DepthFormat::D32Float
		});

		m_pass1 = std::make_unique<vr::GraphicsPass>(m_device, vr::GraphicsPassCreateInfo{
			.vertexShader   = vr::Shader::create(m_device, "shader/obj_loading.vert.glsl.spv"),
			.fragmentShader = vr::Shader::create(m_device, "shader/obj_loading.frag.glsl.spv"),
			.vertexInput    = VERA_REFLECT_VERTEX(Vertex3D),
			.vertexCount    = static_cast<uint32_t>(mesh_attr1->getVertices().size()),
			.indexType      = vr::IndexType::UInt32,
			.indexCount     = static_cast<uint32_t>(mesh_attr1->getIndices().size()),
			.depthFormat    = vr::DepthFormat::D32Float
		});

		map_mesh_data(m_pass0.get(), mesh_attr0);
		map_mesh_data(m_pass1.get(), mesh_attr1);

		auto texture = vr::Texture::create(m_device, vr::TextureCreateInfo{
			.format = image.format(),
			.width  = image.width(),
			.height = image.height()
		});

		texture->upload(image);

		m_pass0->getShaderParameter()["sTexture"] = texture;
		m_pass1->getShaderParameter()["sTexture"] = texture;

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
	
		auto image = m_swapchain->acquireNextImage();

		auto proj = vr::perspective(vr::to_radian(45.f), m_window.getAspect(), 0.1f, 100.f);
		auto view = m_camera->getMatrix();

		m_pass0->getShaderParameter()["pc"]["mat"] = proj * view;
		m_pass1->getShaderParameter()["pc"]["mat"] = proj * view;

		m_pass0->execute(m_render_ctx, image);
		m_pass1->execute(m_render_ctx, image);

		m_render_ctx->submit();
		m_swapchain->present();
	}

private:
	vr::obj<vr::Context>              m_context;
	vr::obj<vr::Device>               m_device;
	vr::obj<vr::RenderContext>        m_render_ctx;
	vr::obj<vr::Swapchain>            m_swapchain;
	std::unique_ptr<vr::GraphicsPass> m_pass0;
	std::unique_ptr<vr::GraphicsPass> m_pass1;

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
