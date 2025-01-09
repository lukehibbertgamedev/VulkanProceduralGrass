#pragma once

#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3

constexpr uint32_t MAX_BLADES = 2<<17; // Test 18, 19, 20, 21, 22 without culling and 22 with culling.
// 413812	- unity value
// 1072145	- unity value

#define GRASS_MIN_WIDTH 0.050f	// Will modify the shader values.
#define GRASS_MAX_WIDTH 0.100f	// Will modify the shader values.

#define GRASS_MIN_HEIGHT 0.45f	// Will modify the shader values.
#define GRASS_MAX_HEIGHT 1.0f	// Will modify the shader values.


struct GrassBladeInstanceData {

	glm::vec4 p0_width = glm::vec4(0.0f);
	glm::vec4 p1_height = glm::vec4(0.0f);
	glm::vec4 p2_direction = glm::vec4(0.0f);
	glm::vec4 up_stiffness = glm::vec4(0.0f);
};

// A class representing ONE blade of grass using the Bézier representation.
class GrassBlade {
public:

	// Sets up packed 4 v4 values.
	void initialise(); 

	// Call this when changing the values of any packed vec4s, this will recalculate the defaults.
	void updatePackedVec4s(); 

	// All grass members can be defined in four packed vector4s where xyz represents a vector3 and w represents a float.
	glm::vec4 p0AndWidth		= glm::vec4(0.0f); 
	glm::vec4 p1AndHeight		= glm::vec4(0.0f); 
	glm::vec4 p2AndDirection	= glm::vec4(0.0f); 
	glm::vec4 upAndStiffness	= glm::vec4(0.0f); 
};
