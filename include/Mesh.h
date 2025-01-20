#pragma once 

#include "Vertex.h"

// Mesh representation for a quad.
class Quad {
public:
	MeshTransform generateQuad(glm::vec3 position);

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	uint32_t vertexCount = 0, indexCount = 0;
};

// Mesh represenation for the base shape of a grass blade before tessellation.
class BaseBladeShape {
public:
	MeshTransform generateShape();

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	uint32_t vertexCount = 0, indexCount = 0;
};
