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


glm::vec3 Blade::calculatePositionAlongQuadraticBezierCurve(float t)
{
	return bezier::lerp(bezier::lerp(p0, p1, t), bezier::lerp(p1, p2, t), t); 
}

Blade::Blade()
{
	p0 = position;
	p1 = p0 + glm::vec3(0.0f, 0.0f, height);
	p2 = p1 + direction * height * lean;
}
