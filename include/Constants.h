#pragma once 

// ===============================================================================================================================================================================

// A collection of constant data and macro definitions.

// ===============================================================================================================================================================================

static constexpr bool kEnableValidationLayers = false; // Used for debugging during development.
const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };

// ===============================================================================================================================================================================

// Number of frames that are either being written to or presented during the application.
static constexpr int kMaxFramesInFlight = 2;

// Number of frames to wait for until the application monitors frame timings.
static constexpr int kWaitFrames = 3000u;

// Number of frames to capture the timing for.
static constexpr int kMonitorFrames = 10000u;

// ===============================================================================================================================================================================

// Ground plane bounds definitions (Z is up)
#define MEADOW_SCALE_X 120
#define MEADOW_SCALE_Y 120
#define MEADOW_SCALE_Z 1	

// ===============================================================================================================================================================================

#define APP_VERSION_MAJOR 1
#define APP_VERSION_MINOR 0

// ===============================================================================================================================================================================

// Grass blade related constants.
constexpr uint32_t kMaxBlades = 2 << 21;
constexpr float kGrassMinWidth = 0.050f;
constexpr float kGrassMaxWidth = 0.100f;
constexpr float kGrassMinHeight = 0.45f;
constexpr float kGrassMaxHeight = 1.0f;