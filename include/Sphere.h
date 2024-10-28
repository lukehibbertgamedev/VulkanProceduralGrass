#pragma once 

#include "Vertex.h"

// Mesh representation for a quad.
class Quad {
public:
	MeshInstance generateQuad(glm::vec3 position);

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
	uint32_t vertexCount = 0, indexCount = 0;
};

// Mesh representation for a UV sphere.
class Sphere {
public:
	MeshInstance generateFlatSphere(glm::vec3 position, float radius, int sectors, int stacks, int up);

	std::vector<Vertex> vertices; 
	std::vector<uint16_t> indices;
	uint32_t vertexCount = 0, indexCount = 0;
private:
	void addVertex(glm::vec3 pos, glm::vec2 tex, glm::vec4 color /*= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)*/);
	void changeUpAxis(int from, int to); 

	float radius;
	int sectorCount;
	int stackCount;
	int up;
};