#pragma once 

#include "Vertex.h"

class Sphere {

public:

	MeshInstance generateFlatSphere(glm::vec3 position, float radius, int sectors, int stacks, int up);
	uint32_t vertexCount = 0, indexCount = 0;

	std::vector<Vertex> vertices; 
	std::vector<uint16_t> indices;
private:
	void addVertex(glm::vec3 pos, glm::vec2 tex, glm::vec4 color /*= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)*/);
	void changeUpAxis(int from, int to); 

	float radius;
	int sectorCount;
	int stackCount;
	int up;
};