#pragma once

// ===============================================================================================================================================================================

// Class data for the grass blade representation using packed data for a quadratic B�zier curve.

// ===============================================================================================================================================================================

#include <glm/glm.hpp> 

// ===============================================================================================================================================================================

// Packed data for use per-grass blade.
struct GrassBladeInstanceData {
	glm::vec4 p0_width = glm::vec4(0.0f);
	glm::vec4 p1_height = glm::vec4(0.0f);
	glm::vec4 p2_direction = glm::vec4(0.0f);
	glm::vec4 up_stiffness = glm::vec4(0.0f);
};

// A class representing one blade of grass using the B�zier representation.
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
