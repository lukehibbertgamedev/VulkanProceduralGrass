#pragma once

#include <glm/glm.hpp> 

// 413812	- unity value
// 1072145	- unity value
// 1535599	- unity value

constexpr uint32_t MAX_BLADES = 2 << 18; 
constexpr float GRASS_MIN_WIDTH = 0.050f;
constexpr float GRASS_MAX_WIDTH = 0.100f;
constexpr float GRASS_MIN_HEIGHT = 0.45f;
constexpr float GRASS_MAX_HEIGHT = 1.0f;

// Packed data for use per-grass blade.
struct GrassBladeInstanceData {
	glm::vec4 p0_width = glm::vec4(0.0f);
	glm::vec4 p1_height = glm::vec4(0.0f);
	glm::vec4 p2_direction = glm::vec4(0.0f);
	glm::vec4 up_stiffness = glm::vec4(0.0f);
};

// A class representing one blade of grass using the Bézier representation.
class GrassBlade {
public:
	// Sets up packed 4x vec4 values.
	void updatePackedData(); 

	// Internal grass data.
	// All grass members can be defined in four packed vector4s where xyz represents a vector3 and w represents a float.
	glm::vec4 p0AndWidth		= glm::vec4(0.0f); 
	glm::vec4 p1AndHeight		= glm::vec4(0.0f); 
	glm::vec4 p2AndDirection	= glm::vec4(0.0f); 
	glm::vec4 upAndStiffness	= glm::vec4(0.0f); 
};
