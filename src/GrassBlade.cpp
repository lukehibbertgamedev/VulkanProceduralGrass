#include "GrassBlade.h"

#include <Utility.h>

void GrassBlade::initialise()
{
	glm::vec3 p0 = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f); // Z-axis is up when using Vulkan.
	float height = GRASS_MIN_HEIGHT + (Utils::getRandomFloat() * (GRASS_MAX_HEIGHT - GRASS_MIN_HEIGHT));

	p0AndWidth = glm::vec4(p0, GRASS_MIN_WIDTH + (Utils::getRandomFloat() * (GRASS_MAX_WIDTH - GRASS_MIN_WIDTH)));
	p1AndHeight = glm::vec4(glm::vec3(p0 + up * height), height);
	p2AndDirection = glm::vec4(glm::vec3(p0 + up * height), 0.f);
	upAndStiffness = glm::vec4(up, 0.f);
}

void GrassBlade::updatePackedVec4s()
{
	glm::vec3 p0 = glm::vec3(p0AndWidth.x, p0AndWidth.y, p0AndWidth.z);
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f); // Z-axis is up when using Vulkan.
	float height = GRASS_MIN_HEIGHT + (Utils::getRandomFloat() * (GRASS_MAX_HEIGHT - GRASS_MIN_HEIGHT));
	float width = GRASS_MIN_WIDTH + (Utils::getRandomFloat() * (GRASS_MAX_WIDTH - GRASS_MIN_WIDTH));
	float direction = 0.0f + (Utils::getRandomFloat() * (360.0f - 0.0f));



	p0AndWidth = glm::vec4(glm::vec3(p0AndWidth.x, p0AndWidth.y, p0AndWidth.z), width);
	p1AndHeight = glm::vec4(glm::vec3(p0 + up * height), height);
	p2AndDirection = glm::vec4(glm::vec3(p0 + up * height), direction);
	upAndStiffness = glm::vec4(up, 0.f);
}
