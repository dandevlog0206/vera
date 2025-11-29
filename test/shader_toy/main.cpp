#include <vera/vera.h>
#include <chrono>
#include <thread>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;

class MyApp
{
public:
	MyApp() :
		m_mouse_pos(0.f, 0.f),
		m_time(0.f),
		m_pause(false)
	{
		m_context = vr::Context::create(vr::ContextCreateInfo{
			.enableValidation = VERA_IS_DEBUG,
			.enableDebugUtils = VERA_IS_DEBUG
		});

		m_device = vr::Device::create(m_context, vr::DeviceCreateInfo{
			.deviceID = m_context->findDeviceByType(vr::DeviceType::Discrete)
		});

		m_window.create(1080, 720, "Shader Toy Example");
		m_window.UserPtr = this;
		m_window.registerEventHandler(eventHandler);

		auto root     = openShaderToyJsonFile();
		auto first_id = root.front().at("id").get<uint32_t>();

		if (!loadShaderToyExample(root, first_id))
			throw vr::Exception("failed to load initial shader toy example: {}", first_id);

		m_render_ctx = vr::RenderContext::create(m_device);

		m_swapchain = vr::Swapchain::create(m_device, m_window, vr::SwapchainCreateInfo{
			.presentMode = vr::PresentMode::Immediate
		});

		m_window.Focussed = true;
	}

	static void eventHandler(vr::os::Window& window, const vr::os::WindowEvent& e)
	{
		MyApp& app = *reinterpret_cast<MyApp*>(window.UserPtr.get());

		switch (e.type()) {
			break;
		case vr::os::WindowEventType::Resize:
		case vr::os::WindowEventType::Move:
			app.drawFrame();
			break;
		case vr::os::WindowEventType::KeyDown: {
			auto& args = e.get_if<vr::os::KeyboardArgs>();

			if (args.key == vr::os::Key::P)
				app.m_pause = !app.m_pause;

			if (auto id = vr::os::Keyboard::getNumber(args.key); id != -1) {
				auto root = app.openShaderToyJsonFile();
				app.loadShaderToyExample(root, static_cast<uint32_t>(id));
			}

		} break;
		case vr::os::WindowEventType::MouseWheel: {
			auto& args = e.get_if<vr::os::MouseWheelArgs>();
		} break;
		}

		app.m_camera->updateEvent(e, app.m_timer.dt());
	}

	void updateAsync(float dt)
	{
		m_camera->updateAsync(dt);
		m_time += m_pause ? 0.f : dt;

		if (vr::os::Mouse::isDown(vr::os::Mouse::LButton)) {
			auto pos = vr::os::Mouse::getPosition(m_window);

			m_mouse_pos = vr::float2(
				static_cast<float>(pos.x),
				static_cast<float>(pos.y));
		}
	}

	void run()
	{
		while (!m_window.needClose()) {
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

		auto root_var = m_pass->getRootVariable();
		root_var["pc"]["resolution"] = vr::float2(width, height);
		root_var["pc"]["time"]       = m_time;
		root_var["pc"]["timeDelta"]  = m_timer.dt();
		root_var["pc"]["frameRate"]  = m_timer.framerate();
		root_var["pc"]["frame"]      = m_render_ctx->getCurrentFrame().frameID;
		root_var["pc"]["mouse"]      = m_mouse_pos;
		root_var["pc"]["position"]   = m_camera->getPosition();
		root_var["pc"]["direction"]  = m_camera->getDirection();
		root_var["pc"]["up"]         = m_camera->getUp();
		root_var["pc"]["scale"]      = 1.f;

		m_pass->execute(m_render_ctx, image);

		m_render_ctx->submit();
		m_swapchain->present();
	}

	bool loadShaderToyExample(const nlohmann::json& root, uint32_t example_id)
	{
		vr::Logger::info(std::format("loading shader toy example {}", example_id));

		try {
			const auto& json = getShaderToyJson(root, example_id);
			std::string vert_path = json.at("vert_path");
			std::string frag_path = json.at("frag_path");
			std::string title     = json.at("title");
			std::string author    = json.at("author");

			if (vert_path.empty())
				throw vr::Exception("missing vertex shader path");
			if (frag_path.empty())
				throw vr::Exception("missing fragment shader path");
			if (title.empty())
				throw vr::Exception("missing example title");

			m_window.Title = std::format("Shader Toy Example - {} by {}", title, author);

			if (json.contains("camera"))
				loadShaderToyCamera(json["camera"]);

			vr::GraphicsPassCreateInfo pass_info = {
				.vertexShader   = vr::Shader::create(m_device, vert_path),
				.fragmentShader = vr::Shader::create(m_device, frag_path),
				.vertexCount    = 6
			};
			
			auto new_pass = vr::GraphicsPass::create(m_device, pass_info);
			
			if (json.contains("channels")) {
				auto root_var = new_pass->getRootVariable();
				loadShaderToyChannels(json["channels"], root_var);
			}
			
			m_device->waitIdle();

			m_pass = std::move(new_pass);
		} catch (const std::exception& e) {
			vr::Logger::error(std::format("failed to load shader toy example {}: {}", example_id, e.what()));
			return false;
		}

		m_mouse_pos = vr::float2(0.f, 0.f);
		m_time      = 0.f;
		m_pause     = false;

		return true;
	}

private:
	nlohmann::json openShaderToyJsonFile()
	{
		std::string   json_path = "resource/shader_toy/shader_toy.json";
		std::ifstream json_file(json_path);
		
		if (!json_file.is_open())
			throw vr::Exception("failed to open shader toy json file: {}", json_path);

		auto json = nlohmann::json::parse(json_file);

		if (json.empty())
			throw vr::Exception("shader toy json file is empty: {}", json_path);

		return json;
	}

	const nlohmann::json& getShaderToyJson(const nlohmann::json& json, uint32_t example_id)
	{
		for (const auto& item : json)
			if (item.at("id").get<uint32_t>() == example_id)
				return item;

		throw vr::Exception("shader toy example not found: {}", example_id);
	}

	void loadShaderToyCamera(nlohmann::json camera)
	{
		std::string type = camera.at("type");

		if (type == "flycam") {
			vr::float3 pos   = parseJsonFloat3(camera.at("position"));
			vr::float3 look  = parseJsonFloat3(camera.at("lookat"));

			auto new_camera = std::make_unique<vr::Flycam>(pos, look);

			if (camera.contains("speed"))
				new_camera->setVelocity(camera.at("speed"));

			m_camera = std::move(new_camera);
		} else if (type == "arcball") {
			vr::float3 pos      = parseJsonFloat3(camera.at("position"));
			vr::float3 look     = parseJsonFloat3(camera.at("lookat"));

			auto new_camera = std::make_unique<vr::Arcball>(pos, look);
			
			if (camera.contains("min_distance"))
				new_camera->setMinDistance(camera.at("min_distance").get<float>());
			if (camera.contains("max_distance"))
				new_camera->setMaxDistance(camera.at("max_distance").get<float>());
			if (camera.contains("min_longitude"))
				new_camera->setMinLongitude(parseJsonAngle(camera.at("min_longitude")));
			if (camera.contains("max_longitude"))
				new_camera->setMaxLongitude(parseJsonAngle(camera.at("max_longitude")));

			m_camera = std::move(new_camera);
		} else {
			throw vr::Exception("invalid camera");
		}
	}

	void loadShaderToyChannels(nlohmann::json channels, vr::ShaderVariable& root_var)
	{
		for (const auto& channel : channels) {
			std::string type = channel.at("type");
			std::string name = channel.at("name");

			if (type == "texture") {
				auto image      = vr::Image::loadFromFile(channel["path"]);
				auto texture    = vr::Texture::create(m_device, vr::TextureCreateInfo{
					.format = image.format(),
					.width  = image.width(),
					.height = image.height()
				});
				
				vr::ImageEdit::flip(image, image, vr::ImageFlipFlagBits::Vertical);
				
				texture->upload(image);

				root_var[name] = vr::TextureView::create(texture);
			} else {
				throw vr::Exception("invalid channel type");
			}
		}
	}

	vr::float3 parseJsonFloat3(const nlohmann::json& json)
	{
		return vr::float3{
			json[0].get<float>(),
			json[1].get<float>(),
			json[2].get<float>()
		};
	}

	vr::radian parseJsonAngle(const nlohmann::json& json)
	{
		std::string str = json.get<std::string>();

		if (str.ends_with("rad"))
			return vr::radian(std::stof(str.substr(0, str.size() - 3)));
		else if (str.ends_with("deg"))
			return vr::radian(vr::to_radian(std::stof(str.substr(0, str.size() - 3))));
		else
			throw vr::Exception("invalid angle format: {}", str);
	}

private:
	vr::obj<vr::Context>        m_context;
	vr::obj<vr::Device>         m_device;
	vr::obj<vr::RenderContext>  m_render_ctx;
	vr::obj<vr::Swapchain>      m_swapchain;
	vr::obj<vr::GraphicsPass>   m_pass;

	vr::os::Window              m_window;
	vr::Timer                   m_timer;
	std::unique_ptr<vr::Camera> m_camera;
	vr::float2                  m_mouse_pos;
	float                       m_time;
	bool                        m_pause;
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
