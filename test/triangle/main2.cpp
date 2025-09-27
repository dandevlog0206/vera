#include <vera/vera.h>

using namespace std;

int main()
{
	bool exit = false;

	vr::os::Window window(1080, 720, "Triangle Demo");

	auto device     = vr::Device::create(vr::Context::create());
	auto render_ctx = vr::RenderContext::create(device);
	auto swapchain  = vr::Swapchain::create(render_ctx, window);

	vr::GraphicsPass pass(device, vr::GraphicsPassCreateInfo{
		.vertexShader   = vr::Shader::create(device, "shaders/triangle_minimal.vert.glsl.spv"),
		.fragmentShader = vr::Shader::create(device, "shaders/triangle.frag.glsl.spv"),
		.vertexCount = 3
	});

	
	while (!exit) {
		vr::os::WindowEvent e;
		while (window.pollEvent(e)) {
			if (e.type() == vr::os::EventType::Close) {
				exit = true;
			}
		}

		if (swapchain->isOccluded()) continue;
		
		pass.execute(render_ctx, swapchain->acquireNextImage());

		render_ctx->submit();
		swapchain->present();
	}

	return 0;
}