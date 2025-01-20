#pragma once 

static constexpr bool kEnableValidationLayers = false; // Used for debugging during development.
const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };
static constexpr bool kEnableImGuiDemoWindow = true;
static constexpr int kMaxFramesInFlight = 2;

// Number of frames to wait for until the application monitors frame timings.
static constexpr int kWaitFrames = 3000u;

// Number of frames to capture the timing for.
static constexpr int kMonitorFrames = 10000u;

// Ground plane bounds definitions (Z is up)
#define MEADOW_SCALE_X 120
#define MEADOW_SCALE_Y 120
#define MEADOW_SCALE_Z 1	