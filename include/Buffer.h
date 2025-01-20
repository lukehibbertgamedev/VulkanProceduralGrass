#pragma once

// ===============================================================================================================================================================================

// Buffer structures used as GPU resources.

// ===============================================================================================================================================================================

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp> 

// ===============================================================================================================================================================================

// Camera matrices & model matrix, bound to 2 pipelines where the grass pipeline ignores model matrix.
struct CameraUniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

// Output value to determine the number of grass blades to draw.
struct NumBladesBufferObject {
	alignas(4) uint32_t numVisible;
};

// For use in the compute shader.
struct PushConstantsObject {
	alignas(4) uint32_t totalNumBlades;
	alignas(4) float elapsed;
};