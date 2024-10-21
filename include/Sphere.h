#pragma once 

#include "Vertex.h"

class Sphere {

public:

	void generateFlatSphere(float radius, int sectors, int stacks, int up);
	std::vector<Vertex> vertices; 
	std::vector<uint16_t> indices;

private:
	void addVertex(glm::vec3 pos, glm::vec2 tex);
	void addIndex(uint16_t triIndex1, uint16_t triIndex2, uint16_t triIndex3);
	void addLineIndex(uint16_t index);

	std::vector<float> computeFaceNormal(glm::vec3 x, glm::vec3 y, glm::vec3 z);
	void changeUpAxis(int from, int to); 

	std::vector<uint16_t> lineIndices;

	float radius;
	int sectorCount;
	int stackCount;
	int up;
};