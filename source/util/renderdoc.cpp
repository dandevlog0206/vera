#include "../../include/vera/util/renderdoc.h"

#include "../../include/vera/core/assertion.h"
#include "../../include/vera/os/window.h"
#include "../../include/vera/util/static_vector.h"
#include "../impl/object_impl.h"
#include "renderdoc_app.h"

#ifdef _WIN32
#include <windows.h>
HMODULE              g_renderdoc_module = NULL;
RENDERDOC_API_1_6_0* g_rdoc_api         = NULL;
#endif

VERA_NAMESPACE_BEGIN

static RENDERDOC_InputButton get_renderdoc_input_button(os::Key key)
{
	switch (key) {
	case os::Key::A:           return eRENDERDOC_Key_A;
	case os::Key::B:           return eRENDERDOC_Key_B;
	case os::Key::C:           return eRENDERDOC_Key_C;
	case os::Key::D:           return eRENDERDOC_Key_D;
	case os::Key::E:           return eRENDERDOC_Key_E;
	case os::Key::F:           return eRENDERDOC_Key_F;
	case os::Key::G:           return eRENDERDOC_Key_G;
	case os::Key::H:           return eRENDERDOC_Key_H;
	case os::Key::I:           return eRENDERDOC_Key_I;
	case os::Key::J:           return eRENDERDOC_Key_J;
	case os::Key::K:           return eRENDERDOC_Key_K;
	case os::Key::L:           return eRENDERDOC_Key_L;
	case os::Key::M:           return eRENDERDOC_Key_M;
	case os::Key::N:           return eRENDERDOC_Key_N;
	case os::Key::O:           return eRENDERDOC_Key_O;
	case os::Key::P:           return eRENDERDOC_Key_P;
	case os::Key::Q:           return eRENDERDOC_Key_Q;
	case os::Key::R:           return eRENDERDOC_Key_R;
	case os::Key::S:           return eRENDERDOC_Key_S;
	case os::Key::T:           return eRENDERDOC_Key_T;
	case os::Key::U:           return eRENDERDOC_Key_U;
	case os::Key::V:           return eRENDERDOC_Key_V;
	case os::Key::W:           return eRENDERDOC_Key_W;
	case os::Key::X:           return eRENDERDOC_Key_X;
	case os::Key::Y:           return eRENDERDOC_Key_Y;
	case os::Key::Z:           return eRENDERDOC_Key_Z;
	case os::Key::Num0:        return eRENDERDOC_Key_0;
	case os::Key::Num1:        return eRENDERDOC_Key_1;
	case os::Key::Num2:        return eRENDERDOC_Key_2;
	case os::Key::Num3:        return eRENDERDOC_Key_3;
	case os::Key::Num4:        return eRENDERDOC_Key_4;
	case os::Key::Num5:        return eRENDERDOC_Key_5;
	case os::Key::Num6:        return eRENDERDOC_Key_6;
	case os::Key::Num7:        return eRENDERDOC_Key_7;
	case os::Key::Num8:        return eRENDERDOC_Key_8;
	case os::Key::Num9:        return eRENDERDOC_Key_9;
	case os::Key::Backspace:   return eRENDERDOC_Key_Backspace;
	case os::Key::Tab:         return eRENDERDOC_Key_Tab;
	case os::Key::F1:          return eRENDERDOC_Key_F1;
	case os::Key::F2:          return eRENDERDOC_Key_F2;
	case os::Key::F3:          return eRENDERDOC_Key_F3;
	case os::Key::F4:          return eRENDERDOC_Key_F4;
	case os::Key::F5:          return eRENDERDOC_Key_F5;
	case os::Key::F6:          return eRENDERDOC_Key_F6;
	case os::Key::F7:          return eRENDERDOC_Key_F7;
	case os::Key::F8:          return eRENDERDOC_Key_F8;
	case os::Key::F9:          return eRENDERDOC_Key_F9;
	case os::Key::F10:         return eRENDERDOC_Key_F10;
	case os::Key::F11:         return eRENDERDOC_Key_F11;
	case os::Key::F12:         return eRENDERDOC_Key_F12;
	case os::Key::PrintScreen: return eRENDERDOC_Key_PrtScrn;
	case os::Key::Pause:       return eRENDERDOC_Key_Pause;
	case os::Key::Insert:      return eRENDERDOC_Key_Insert;
	case os::Key::Home:        return eRENDERDOC_Key_Home;
	case os::Key::PageUp:      return eRENDERDOC_Key_PageUp;
	case os::Key::Delete:      return eRENDERDOC_Key_Delete;
	case os::Key::End:         return eRENDERDOC_Key_End;
	case os::Key::PageDown:    return eRENDERDOC_Key_PageDn;
	}
}

bool RenderDoc::init(RenderDocAPIVersion version)
{
#if defined(_WIN32)
	VERA_ASSERT_MSG(g_renderdoc_module == NULL, "RenderDoc already initialized");

	if (HMODULE g_renderdoc_module = GetModuleHandleA("renderdoc.dll")) {
		pRENDERDOC_GetAPI RENDERDOC_GetAPI =
			(pRENDERDOC_GetAPI)GetProcAddress(g_renderdoc_module, "RENDERDOC_GetAPI");
		int ret = RENDERDOC_GetAPI(static_cast<RENDERDOC_Version>(version), (void**)&g_rdoc_api);
		
		return ret == 1;
	}
#endif

	return false;
}

void RenderDoc::destroy()
{
#if defined(_WIN32)
	// FreeLibrary(g_renderdoc_module);
	g_renderdoc_module = NULL;
	g_rdoc_api         = NULL;
#endif
}

void RenderDoc::getApiVersion(int* major, int* minor, int* patch)
{
	if (g_rdoc_api) g_rdoc_api->GetAPIVersion(major, minor, patch);
}

void RenderDoc::setCaptureOptionU32(RenderDocCaptureOption option, uint32_t val)
{
	if (g_rdoc_api) g_rdoc_api->SetCaptureOptionU32(static_cast<RENDERDOC_CaptureOption>(option), val);
}

void RenderDoc::setCaptureOptionF32(RenderDocCaptureOption option, float val)
{
	if (g_rdoc_api) g_rdoc_api->SetCaptureOptionF32(static_cast<RENDERDOC_CaptureOption>(option), val);
}

uint32_t RenderDoc::getCaptureOptionU32(RenderDocCaptureOption option)
{
	if (!g_rdoc_api) return 0;
	return g_rdoc_api->GetCaptureOptionU32(static_cast<RENDERDOC_CaptureOption>(option));
}

float RenderDoc::getCaptureOptionF32(RenderDocCaptureOption option)
{
	if (!g_rdoc_api) return 0.f;
	return g_rdoc_api->GetCaptureOptionF32(static_cast<RENDERDOC_CaptureOption>(option));
}

void RenderDoc::setFocusToggleKeys(os::Key* keys, int num)
{
	if (!g_rdoc_api) return;

	static_vector<RENDERDOC_InputButton, 128> buttons;

	for (int i = 0; i < num; i++)
		buttons.push_back(get_renderdoc_input_button(keys[i]));

	g_rdoc_api->SetFocusToggleKeys(buttons.data(), buttons.size());
}

void RenderDoc::setCaptureKeys(os::Key* keys, int num)
{
	if (!g_rdoc_api) return;

	static_vector<RENDERDOC_InputButton, 128> buttons;

	for (int i = 0; i < num; i++)
		buttons.push_back(get_renderdoc_input_button(keys[i]));

	g_rdoc_api->SetCaptureKeys(buttons.data(), buttons.size());
}

uint32_t RenderDoc::getOverlayBits()
{
	if (!g_rdoc_api) return 0;
	return g_rdoc_api->GetOverlayBits();
}

void RenderDoc::maskOverlayBits(uint32_t and_mask, uint32_t or_mask)
{
	if (g_rdoc_api) g_rdoc_api->MaskOverlayBits(and_mask, or_mask);
}

void RenderDoc::removeHooks()
{
	if (g_rdoc_api) g_rdoc_api->RemoveHooks();
}

void RenderDoc::unloadCrashHandler()
{
	if (g_rdoc_api) g_rdoc_api->UnloadCrashHandler();
}

void RenderDoc::setCaptureFilePathTemplate(const char* pathtemplate)
{
	if (g_rdoc_api) g_rdoc_api->SetCaptureFilePathTemplate(pathtemplate);
}

const char* RenderDoc::getCaptureFilePathTemplate()
{
	if (!g_rdoc_api) nullptr;
	return g_rdoc_api->GetCaptureFilePathTemplate();
}

uint32_t RenderDoc::getNumCaptures()
{
	if (!g_rdoc_api) return 0;
	return g_rdoc_api->GetNumCaptures();
}

bool RenderDoc::getCapture(uint32_t idx, char* filename, uint32_t* pathlength, uint64_t* timestamp)
{
	if (!g_rdoc_api) return false;
	return g_rdoc_api->GetCapture(idx, filename, pathlength, timestamp);
}

void RenderDoc::triggerCapture()
{
	if (g_rdoc_api) g_rdoc_api->TriggerCapture();
}

bool RenderDoc::isTargetControlConnected()
{
	if (!g_rdoc_api) return false;
	return g_rdoc_api->IsTargetControlConnected();
}

bool RenderDoc::launchReplayUI(uint32_t connectTargetControl, const char* cmdline)
{
	if (g_rdoc_api) return false;
	return g_rdoc_api->LaunchReplayUI(connectTargetControl, cmdline);
}

void RenderDoc::setActiveWindow(const_ref<Context> context, const os::Window* wndHandle)
{
	if (!g_rdoc_api) return;

	void* device_handle = nullptr;
	void* window_handle = nullptr;

	if (context) {
		VkInstance vk_instance = get_vk_instance(context);
		device_handle = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(vk_instance);
	}
	if (wndHandle)
		window_handle = wndHandle->getNativeHandle();

	g_rdoc_api->SetActiveWindow(device_handle, window_handle);
}

void RenderDoc::startFrameCapture(const_ref<Context> context, const os::Window* wndHandle)
{
	if (!g_rdoc_api) return;

	void* device_handle = nullptr;
	void* window_handle = nullptr;

	if (context) {
		VkInstance vk_instance = get_vk_instance(context);
		device_handle = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(vk_instance);
	}
	if (wndHandle)
		window_handle = wndHandle->getNativeHandle();

	g_rdoc_api->StartFrameCapture(device_handle, window_handle);
}

bool RenderDoc::isFrameCapturing()
{
	if (!g_rdoc_api) return false;
	return g_rdoc_api->IsFrameCapturing();
}

bool RenderDoc::endFrameCapture(const_ref<Context> context, const os::Window* wndHandle)
{
	if (!g_rdoc_api) return false;

	void* device_handle = nullptr;
	void* window_handle = nullptr;

	if (context) {
		VkInstance vk_instance = get_vk_instance(context);
		device_handle = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(vk_instance);
	}
	if (wndHandle)
		window_handle = wndHandle->getNativeHandle();

	return g_rdoc_api->EndFrameCapture(device_handle, window_handle);
}

void RenderDoc::triggerMultiFrameCapture(uint32_t numFrames)
{
	if (g_rdoc_api) g_rdoc_api->TriggerMultiFrameCapture(numFrames);
}

void RenderDoc::setCaptureFileComments(const char* filePath, const char* comments)
{
	if (g_rdoc_api) g_rdoc_api->SetCaptureFileComments(filePath, comments);
}

bool RenderDoc::discardFrameCapture(const_ref<Context> context, const os::Window* wndHandle)
{
	if (!g_rdoc_api) return false;
	
	void* device_handle = nullptr;
	void* window_handle = nullptr;

	if (context) {
		VkInstance vk_instance = get_vk_instance(context);
		device_handle = RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(vk_instance);
	}
	if (wndHandle)
		window_handle = wndHandle->getNativeHandle();

	return g_rdoc_api->DiscardFrameCapture(device_handle, window_handle);
}

void RenderDoc::showReplayUI()
{
	if (g_rdoc_api) g_rdoc_api->ShowReplayUI();
}

void RenderDoc::setCaptureTitle(const char* title)
{
	if (g_rdoc_api) g_rdoc_api->SetCaptureTitle(title);
}

VERA_NAMESPACE_END