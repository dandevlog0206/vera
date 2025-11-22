#include <vera/vera.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

int main()
{
	vr::os::Window window(1080, 720, "Minimal Triangle Demo");

	auto device    = vr::Device::create(vr::Context::create());
	auto ctx       = vr::RenderContext::create(device);
	auto swapchain = vr::Swapchain::create(device, window);

	auto vert_shader = vr::Shader::create(device, "spirv/test.vert.glsl.spv");
	auto geom_shader = vr::Shader::create(device, "spirv/test.geom.glsl.spv");
	auto frag_shader = vr::Shader::create(device, "spirv/test.frag.glsl.spv");

	auto reflection = vr::ProgramReflection::create(
		device,
		{
			vr::ShaderReflection::create(device, vert_shader),
			vr::ShaderReflection::create(device, geom_shader),
			vr::ShaderReflection::create(device, frag_shader)
		});

	auto pass_info = vr::GraphicsPassCreateInfo{
		.vertexShader   = vr::Shader::create(device, "spirv/triangle_minimal.vert.glsl.spv"),
		.fragmentShader = vr::Shader::create(device, "spirv/triangle_minimal.frag.glsl.spv"),
		.vertexCount    = 3
	};

	auto pass = vr::GraphicsPass::create(device, pass_info);

	while (!window.needClose()) {
		window.handleEvent();

		pass->execute(ctx, swapchain->acquireNextImage());

		ctx->submit();
		swapchain->present();
	}

	return 0;
}
