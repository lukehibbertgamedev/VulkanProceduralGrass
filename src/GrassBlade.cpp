#include "GrassBlade.h"

// ===============================================================================================================================================================================

#include <Utility.h>
#include <Constants.h>

// ===============================================================================================================================================================================

void GrassBlade::updatePackedData()
{
	glm::vec3 p0 = glm::vec3(p0AndWidth.x, p0AndWidth.y, p0AndWidth.z);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f); // Z-axis is up.
	float height = kGrassMinHeight + (Utils::getRandomFloat() * (kGrassMaxHeight - kGrassMinHeight));
	float width = kGrassMinWidth + (Utils::getRandomFloat() * (kGrassMaxWidth - kGrassMinWidth));
	float direction = 0.0f + (Utils::getRandomFloat() * (360.0f - 0.0f));

	p0AndWidth = glm::vec4(glm::vec3(p0AndWidth.x, p0AndWidth.y, p0AndWidth.z), width);
	p1AndHeight = glm::vec4(glm::vec3(p0 + up * height), height);
	p2AndDirection = glm::vec4(glm::vec3(p0 + up * height), direction);
	upAndStiffness = glm::vec4(up, 0.f);
}
