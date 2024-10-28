#include "GrassBlade.h"

//Blade::Blade()
//{
//}
//
//VkVertexInputBindingDescription Blade::getBindingDescription()
//{
//    VkVertexInputBindingDescription bindingDescription = {};
//    bindingDescription.binding = 0;
//    bindingDescription.stride = sizeof(Blade);
//    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//    return bindingDescription;
//}
//
//std::array<VkVertexInputAttributeDescription, 4> Blade::getAttributeDescription()
//{
//    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
//
//    // Position and direction.
//    attributeDescriptions[0].binding = 0;
//    attributeDescriptions[0].location = 0;
//    attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//    attributeDescriptions[0].offset = offsetof(Blade, positionAndDirection);
//
//    // Bezier point and height.
//    attributeDescriptions[1].binding = 0;
//    attributeDescriptions[1].location = 1;
//    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//    attributeDescriptions[1].offset = offsetof(Blade, bezierPointAndHeight);
//
//    // Physical model guide and width.
//    attributeDescriptions[2].binding = 0;
//    attributeDescriptions[2].location = 2;
//    attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//    attributeDescriptions[2].offset = offsetof(Blade, physicalModelGuideAndWidth);
//
//    // Up vector and stiffness coefficient.
//    attributeDescriptions[3].binding = 0;
//    attributeDescriptions[3].location = 3;
//    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//    attributeDescriptions[3].offset = offsetof(Blade, upVectorAndStiffnessCoefficient);
//
//    return attributeDescriptions;
//}


//glm::vec3 Blade::calculatePositionAlongQuadraticBezierCurve(float t)
//{
//	return bezier::lerp(bezier::lerp(p0, p1, t), bezier::lerp(p1, p2, t), t); 
//}
//
//Blade::Blade()
//{
//	p0 = position;
//	p1 = p0 + glm::vec3(0.0f, 0.0f, height);
//	p2 = p1 + direction * height * lean;
//}

void Blade::initialise()
{
	float direction = 0.0f;
	glm::vec3 p0 = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 p1 = p0 + glm::vec3(0.0f, 0.0f, GRASS_HEIGHT);
	glm::vec3 p2 = p1 + direction * GRASS_HEIGHT * GRASS_LEAN;
	glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f); // Z-axis is up when using Vulkan.

	p0AndWidth = glm::vec4(p0, GRASS_WIDTH);  
	p1AndHeight = glm::vec4(p1, GRASS_HEIGHT);
	p2AndDirection = glm::vec4(p2, direction);
	upAndStiffness = glm::vec4(up, GRASS_STIFFNESS);
}

glm::vec3 Blade::calculatePositionAlongBezierCurve(float interpolationValue)
{
	glm::vec3 p0p1 = bezier::lerp(glm::vec3(p0AndWidth.x, p0AndWidth.y, p0AndWidth.z), glm::vec3(p1AndHeight.x, p1AndHeight.y, p1AndHeight.z), interpolationValue);
	glm::vec3 p1p2 = bezier::lerp(glm::vec3(p1AndHeight.x, p1AndHeight.y, p1AndHeight.z), glm::vec3(p2AndDirection.x, p2AndDirection.y, p2AndDirection.z), interpolationValue);
	return bezier::lerp(p0p1, p1p2, interpolationValue);
}

VkVertexInputBindingDescription Blade::getBindingDescription()
{
	// Configure how Vulkan understands the format of the blade's class data.
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Blade);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Blade::getAttributeDescription()
{
	// Configure per-vertex information.
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

	// Base control point position and width.
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Blade, p0AndWidth);

	// Top control point position and height.
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Blade, p1AndHeight);

	// Tip control point position and direction angle.
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Blade, p2AndDirection);

	// Up vector and stiffness coefficient.
	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Blade, upAndStiffness);

	return attributeDescriptions;
}