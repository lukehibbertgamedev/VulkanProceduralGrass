#pragma once

// ===============================================================================================================================================================================

// Basic vertex and transform data for the terrain rendering.

// ===============================================================================================================================================================================

#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>  

#include <array>        

// ===============================================================================================================================================================================

struct MeshTransform {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f); 
};

// Note: Whatever variables are here, must match the IN parameters for the vertex shader.
struct Vertex {
	glm::vec3 pos;		
	glm::vec4 color;	
	glm::vec2 uv;		

	// Describes how vertex data is grouped in memory.
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	// Defines attribute layouts within a vertex (in order of definition).
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() { 
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 pos
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // vec4 color
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // vec2 uv
		attributeDescriptions[2].offset = offsetof(Vertex, uv);

		return attributeDescriptions;
	}
};