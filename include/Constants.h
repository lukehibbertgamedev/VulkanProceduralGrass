#pragma once 

static constexpr bool kEnableValidationLayers = true;
const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };
static constexpr bool kEnableImGuiDemoWindow = true;
static constexpr int kMaxFramesInFlight = 2;

// Ground plane bounds definitions (Z is up)
#define MEADOW_SCALE_X 120
#define MEADOW_SCALE_Y 120
#define MEADOW_SCALE_Z 1	