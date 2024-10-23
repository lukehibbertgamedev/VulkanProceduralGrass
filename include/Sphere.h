#pragma once 

#include "Vertex.h"

class Sphere {

public:

	void generateFlatSphere(float radius, int sectors, int stacks, int up);
	void generateUVSphere(int radius, int latitudes, int longitudes);
	std::vector<Vertex> vertices; 
	std::vector<uint16_t> indices;
	std::vector<uint16_t> lineIndices;

private:
	void addVertex(glm::vec3 pos, glm::vec2 tex, glm::vec4 color /*= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)*/);
	void addIndex(uint16_t triIndex1, uint16_t triIndex2, uint16_t triIndex3);
	void addLineIndex(uint16_t index);

	std::vector<float> computeFaceNormal(glm::vec3 x, glm::vec3 y, glm::vec3 z);
	void changeUpAxis(int from, int to); 


	float radius;
	int sectorCount;
	int stackCount;
	int up;
};