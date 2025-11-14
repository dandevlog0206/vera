#include <vera/vera.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <fstream>
#include <chrono>
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>

using namespace std;

std::stringstream ss;

int test_main()
{
	vr::os::Window window(1080, 720, "Minimal Triangle Demo");

	vr::ContextCreateInfo ctx_info = {
		.enableValidation = true,
		.enableDebugUtils = true
	};

	auto device    = vr::Device::create(vr::Context::create(ctx_info));
	auto ctx       = vr::RenderContext::create(device);
	auto swapchain = vr::Swapchain::create(device, window);

	//auto vert_shader = vr::Shader::create(device, "spirv/test.vert.glsl.spv");
	//auto geom_shader = vr::Shader::create(device, "spirv/test.geom.glsl.spv");
	//auto frag_shader = vr::Shader::create(device, "spirv/test.frag.glsl.spv");

	//auto begin = std::chrono::high_resolution_clock::now();

	//// auto reflection = vr::ShaderReflection::create({ vert_shader, frag_shader });

	//auto end = std::chrono::high_resolution_clock::now();

	//std::cout << "Shader reflection took " <<
	//	std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() <<
	//	" ms" << std::endl;

	ss << 1;

	vr::GraphicsPass pass(device, vr::GraphicsPassCreateInfo{
		.vertexShader   = vr::Shader::create(device, "spirv/triangle_minimal.vert.glsl.spv"),
		.fragmentShader = vr::Shader::create(device, "spirv/triangle_minimal.frag.glsl.spv"),
		.vertexCount    = 3
	});

	while (!window.needClose()) {
		window.handleEvent();
		
		pass.execute(ctx, swapchain->acquireNextImage());


		try {
			ctx->submit();
			swapchain->present();
		} catch (...) {
			GFSDK_Aftermath_CrashDump_Status status;
			
			GFSDK_Aftermath_GetCrashDumpStatus(&status);

			auto tStart = std::chrono::steady_clock::now();
			auto tElapsed = std::chrono::milliseconds::zero();

			// Loop while Aftermath crash dump data collection has not finished or
			// the application is still processing the crash dump data.
			while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
				   status != GFSDK_Aftermath_CrashDump_Status_Finished &&
				   tElapsed.count() < 50)
			{
				// Sleep a couple of milliseconds and poll the status again.
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				GFSDK_Aftermath_GetCrashDumpStatus(&status);

				tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tStart);
			}

			throw;
		}
	}

	return 0;
}

// Static callback wrapper for OnShaderDebugInfoLookup
void ShaderDebugInfoLookupCallback(
	const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier,
	PFN_GFSDK_Aftermath_SetData setShaderDebugInfo,
	void* pUserData)
{
}

// Static callback wrapper for OnShaderLookup
void ShaderLookupCallback(
	const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash,
	PFN_GFSDK_Aftermath_SetData setShaderBinary,
	void* pUserData)
{
}

// Static callback wrapper for OnShaderSourceDebugInfoLookup
void ShaderSourceDebugInfoLookupCallback(
	const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName,
	PFN_GFSDK_Aftermath_SetData setShaderBinary,
	void* pUserData)
{
}

// Handler for shader debug information lookup callbacks.
// This is used by the JSON decoder for mapping shader instruction
// addresses to IL lines or source lines.
void OnShaderDebugInfoLookup(
	const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier,
	PFN_GFSDK_Aftermath_SetData setShaderDebugInfo)
{
}

// Handler for shader lookup callbacks.
// This is used by the JSON decoder for mapping shader instruction
// addresses to IL lines or source lines.
// NOTE: If the application loads stripped shader binaries, Aftermath
// will require access to both the stripped and the non-stripped
// shader binaries.
void OnShaderLookup(
	const GFSDK_Aftermath_ShaderBinaryHash& shaderHash,
	PFN_GFSDK_Aftermath_SetData setShaderBinary)
{
}

// Handler for shader source debug info lookup callbacks.
// This is used by the JSON decoder for mapping shader instruction addresses to
// source lines if the shaders used by the application were compiled with
// separate debug info data files.
void OnShaderSourceDebugInfoLookup(
	const GFSDK_Aftermath_ShaderDebugName& shaderDebugName,
	PFN_GFSDK_Aftermath_SetData setShaderBinary)
{
}

void GpuCrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData)
{
	std::ofstream ofs("GpuCrashDump.nv-gpudmp", std::ios::binary);
	ofs.write((const char*)pGpuCrashDump, gpuCrashDumpSize);
	ofs.close();

	GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};

	GFSDK_Aftermath_GpuCrashDump_CreateDecoder(
		GFSDK_Aftermath_Version_API,
		pGpuCrashDump,
		gpuCrashDumpSize,
		&decoder);

	// Query the size of the required results buffer
	uint32_t jsonSize = 0;
	GFSDK_Aftermath_Result result = GFSDK_Aftermath_GpuCrashDump_GenerateJSON(
		decoder,
		GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,                                            // The flags controlling what information to include in the JSON.
		GFSDK_Aftermath_GpuCrashDumpFormatterFlags_CONDENSED_OUTPUT, // Generate condensed output, i.e., omit all unnecessary whitespace.
		ShaderDebugInfoLookupCallback,                               // Callback function invoked to find shader debug information data.
		ShaderLookupCallback,                                        // Callback function invoked to find shader binary data by shader hash.
		nullptr,                         // Callback function invoked to find shader source debug data by shader DebugName.
		nullptr,                                      // User data that will be provided to the above callback functions.
		&jsonSize);                                                  // Result of the call: size in bytes of the generated JSON data.

	if (GFSDK_Aftermath_SUCCEED(result) && result != GFSDK_Aftermath_Result_NotAvailable)
	{
		// Allocate buffer for results.
		std::vector<char> json(jsonSize);

		// Query the generated JSON data taht si cached inside the decoder object.
		result = GFSDK_Aftermath_GpuCrashDump_GetJSON(
			decoder,
			json.size(),
			json.data());
		if (GFSDK_Aftermath_SUCCEED(result))
		{
			cout << "JSON: " << json.data();
		}
	}
}

// Static wrapper for the shader debug information handler. See the 'Handling Shader Debug Information callbacks' section for details.
void ShaderDebugInfoCallback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData)
{

}

// Static wrapper for the GPU crash dump description handler. See the 'Handling GPU Crash Dump Description Callbacks' section for details.
void CrashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* pUserData)
{
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, "Hello Nsight Aftermath");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion, "v1.0");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined, "This is a GPU crash dump example");
	addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 1, "Engine State: Rendering");
}

 // Static wrapper for the resolve marker handler. See the 'Handling Marker Resolve Callbacks' section for details.
void ResolveMarkerCallback(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize)
{

}

int main()
{
	// Enable GPU crash dumps and register callbacks.
	GFSDK_Aftermath_EnableGpuCrashDumps(
		GFSDK_Aftermath_Version_API,
		GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
		GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default,   // Default behavior.
		GpuCrashDumpCallback,                               // Register callback for GPU crash dumps.
		ShaderDebugInfoCallback,                            // Register callback for shader debug information.
		CrashDumpDescriptionCallback,                       // Register callback for GPU crash dump description.
		ResolveMarkerCallback,                              // Register callback for marker resolution (R495 or later NVIDIA graphics driver).
		nullptr);                            // Set the GpuCrashTracker object as user data passed back by the above callbacks.

	//test_main();
	//return 0;

	try {
		test_main();
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}