#pragma once

#include "../core/intrusive_ptr.h"
#include "../os/keyboard.h"

VERA_NAMESPACE_BEGIN
VERA_OS_NAMESPACE_BEGIN

class Window;

VERA_OS_NAMESPACE_END

class Context;

enum class RenderDocAPIVersion VERA_ENUM
{
	Version_1_0_0 = 10000,
	Version_1_0_1 = 10001,
	Version_1_0_2 = 10002,
	Version_1_1_0 = 10100,
	Version_1_1_1 = 10101,
	Version_1_1_2 = 10102,
	Version_1_2_0 = 10200,
	Version_1_3_0 = 10300,
	Version_1_4_0 = 10400,
	Version_1_4_1 = 10401,
	Version_1_4_2 = 10402,
	Version_1_5_0 = 10500,
	Version_1_6_0 = 10600,
};

enum class RenderDocCaptureOption VERA_ENUM
{
	// Allow the application to enable vsync
	//
	// Default - enabled
	//
	// 1 - The application can enable or disable vsync at will
	// 0 - vsync is force disabled
	AllowVSync = 0,

	// Allow the application to enable fullscreen
	//
	// Default - enabled
	//
	// 1 - The application can enable or disable fullscreen at will
	// 0 - fullscreen is force disabled
	AllowFullscreen = 1,

	// Record API debugging events and messages
	//
	// Default - disabled
	//
	// 1 - Enable built-in API debugging features and records the results into
	//     the capture, which is matched up with events on replay
	// 0 - no API debugging is forcibly enabled
	APIValidation = 2,

	// Capture CPU callstacks for API events
	//
	// Default - disabled
	//
	// 1 - Enables capturing of callstacks
	// 0 - no callstacks are captured
	CaptureCallstacks = 3,

	// When capturing CPU callstacks, only capture them from actions.
	// This option does nothing without the above option being enabled
	//
	// Default - disabled
	//
	// 1 - Only captures callstacks for actions.
	//     Ignored if CaptureCallstacks is disabled
	// 0 - Callstacks, if enabled, are captured for every event.
	CaptureCallstacksOnlyDraws = 4,
	CaptureCallstacksOnlyActions = 4,

	// Specify a delay in seconds to wait for a debugger to attach, after
	// creating or injecting into a process, before continuing to allow it to run.
	//
	// 0 indicates no delay, and the process will run immediately after injection
	//
	// Default - 0 seconds
	//
	DelayForDebugger = 5,

	// Verify buffer access. This includes checking the memory returned by a Map() call to
	// detect any out-of-bounds modification, as well as initialising buffers with undefined contents
	// to a marker value to catch use of uninitialised memory.
	//
	// NOTE: This option is only valid for OpenGL and D3D11. Explicit APIs such as D3D12 and Vulkan do
	// not do the same kind of interception & checking and undefined contents are really undefined.
	//
	// Default - disabled
	//
	// 1 - Verify buffer access
	// 0 - No verification is performed, and overwriting bounds may cause crashes or corruption in
	//     RenderDoc.
	VerifyBufferAccess = 6,

	// The old name for VerifyBufferAccess was VerifyMapWrites.
	// This option now controls the filling of uninitialised buffers with 0xdddddddd which was
	// previously always enabled
	VerifyMapWrites = VerifyBufferAccess,

	// Hooks any system API calls that create child processes, and injects
	// RenderDoc into them recursively with the same options.
	//
	// Default - disabled
	//
	// 1 - Hooks into spawned child processes
	// 0 - Child processes are not hooked by RenderDoc
	HookIntoChildren = 7,

	// By default RenderDoc only includes resources in the final capture necessary
	// for that frame, this allows you to override that behaviour.
	//
	// Default - disabled
	//
	// 1 - all live resources at the time of capture are included in the capture
	//     and available for inspection
	// 0 - only the resources referenced by the captured frame are included
	RefAllResources = 8,

	// **NOTE**: As of RenderDoc v1.1 this option has been deprecated. Setting or
	// getting it will be ignored, to allow compatibility with older versions.
	// In v1.1 the option acts as if it's always enabled.
	//
	// By default RenderDoc skips saving initial states for resources where the
	// previous contents don't appear to be used, assuming that writes before
	// reads indicate previous contents aren't used.
	//
	// Default - disabled
	//
	// 1 - initial contents at the start of each captured frame are saved, even if
	//     they are later overwritten or cleared before being used.
	// 0 - unless a read is detected, initial contents will not be saved and will
	//     appear as black or empty data.
	SaveAllInitials = 9,

	// In APIs that allow for the recording of command lists to be replayed later,
	// RenderDoc may choose to not capture command lists before a frame capture is
	// triggered, to reduce overheads. This means any command lists recorded once
	// and replayed many times will not be available and may cause a failure to
	// capture.
	//
	// NOTE: This is only true for APIs where multithreading is difficult or
	// discouraged. Newer APIs like Vulkan and D3D12 will ignore this option
	// and always capture all command lists since the API is heavily oriented
	// around it and the overheads have been reduced by API design.
	//
	// 1 - All command lists are captured from the start of the application
	// 0 - Command lists are only captured if their recording begins during
	//     the period when a frame capture is in progress.
	CaptureAllCmdLists = 10,

	// Mute API debugging output when the API validation mode option is enabled
	//
	// Default - enabled
	//
	// 1 - Mute any API debug messages from being displayed or passed through
	// 0 - API debugging is displayed as normal
	DebugOutputMute = 11,

	// Option to allow vendor extensions to be used even when they may be
	// incompatible with RenderDoc and cause corrupted replays or crashes.
	//
	// Default - inactive
	//
	// No values are documented, this option should only be used when absolutely
	// necessary as directed by a RenderDoc developer.
	AllowUnsupportedVendorExtensions = 12,

	// Define a soft memory limit which some APIs may aim to keep overhead under where
	// possible. Anything above this limit will where possible be saved directly to disk during
	// capture.
	// This will cause increased disk space use (which may cause a capture to fail if disk space is
	// exhausted) as well as slower capture times.
	//
	// Not all memory allocations may be deferred like this so it is not a guarantee of a memory
	// limit.
	//
	// Units are in MBs, suggested values would range from 200MB to 1000MB.
	//
	// Default - 0 Megabytes
	SoftMemoryLimit = 13,
};

class RenderDoc
{
public:
	static bool init(RenderDocAPIVersion version = RenderDocAPIVersion::Version_1_6_0);
	static void destroy();

	static void getApiVersion(int* major, int* minor, int* patch);

	static void setCaptureOptionU32(RenderDocCaptureOption option, uint32_t val);
	static void setCaptureOptionF32(RenderDocCaptureOption option, float val);

	static uint32_t getCaptureOptionU32(RenderDocCaptureOption option);
	static float getCaptureOptionF32(RenderDocCaptureOption option);

	static void setFocusToggleKeys(os::Key* keys, int num);
	static void setCaptureKeys(os::Key* keys, int num);

	static uint32_t getOverlayBits();
	static void maskOverlayBits(uint32_t and_mask, uint32_t or_mask);

	static void removeHooks();
	static void unloadCrashHandler();

	static void setCaptureFilePathTemplate(const char* pathtemplate);
	static const char* getCaptureFilePathTemplate();

	static uint32_t getNumCaptures();
	static bool getCapture(uint32_t idx, char* filename, uint32_t* pathlength, uint64_t* timestamp);

	static void triggerCapture();
	static void triggerMultiFrameCapture(uint32_t numFrames);

	static bool isTargetControlConnected();
	static bool launchReplayUI(uint32_t connectTargetControl, const char* cmdline);

	static void setActiveWindow(cref<Context> context = nullptr, const os::Window* wndHandle = nullptr);

	static void startFrameCapture(cref<Context> context = nullptr, const os::Window* wndHandle = nullptr);
	static bool isFrameCapturing();
	static bool endFrameCapture(cref<Context> context = nullptr, const os::Window* wndHandle = nullptr);
	static bool discardFrameCapture(cref<Context> context = nullptr, const os::Window* wndHandle = nullptr);

	static void setCaptureFileComments(const char* filePath, const char* comments);
	static void showReplayUI();
	static void setCaptureTitle(const char* title);
};

VERA_NAMESPACE_END
