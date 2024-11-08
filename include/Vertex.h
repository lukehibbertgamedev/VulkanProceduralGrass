#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3
#include <array>        // For std::array

#include <../PortableVulkanSDK1.3/include/vulkan/vulkan.h>

struct MeshTransform {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f); 
};

// Note: Whatever variables are here, must match the IN parameters for the vertex shader.
struct Vertex {
	glm::vec3 pos;		// layout(location = 0) in vec3
	glm::vec4 color;	// layout(location = 1) in vec4

	// Describes how vertex data is grouped in memory.
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	// Defines attribute layouts within a vertex (in order of definition).
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() { 
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 pos
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4 color
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}
};