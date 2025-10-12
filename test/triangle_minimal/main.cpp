#include <vera/vera.h>

int main()
{
	vr::os::Window window(1080, 720, "Triangle Demo");

	auto device    = vr::Device::create(vr::Context::create());
	auto ctx       = vr::RenderContext::create(device);
	auto swapchain = vr::Swapchain::create(device, window);

	auto vert_shader = vr::Shader::create(device, "shader/test1.vert.glsl.spv");
	auto frag_shader = vr::Shader::create(device, "shader/test1.frag.glsl.spv");

	std::vector<vr::const_ref<vr::Shader>> shaders = { vert_shader, frag_shader };

	auto layout = vr::PipelineLayout::create(device, shaders);

	vr::GraphicsPass pass(device, vr::GraphicsPassCreateInfo{
		.vertexShader   = vr::Shader::create(device, "shader/triangle_minimal.vert.glsl.spv"),
		.fragmentShader = vr::Shader::create(device, "shader/triangle.frag.glsl.spv"),
		.vertexCount    = 3
	});

	while (!window.needClose()) {
		window.handleEvent();
		
		pass.execute(ctx, swapchain->acquireNextImage());

		ctx->submit();
		swapchain->present();
	}

	return 0;
}