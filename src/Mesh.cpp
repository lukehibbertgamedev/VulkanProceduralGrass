#include "Mesh.h"

MeshTransform Quad::generateQuad(glm::vec3 position)
{
	vertices = {
		{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f },  { 0.0f, 1.0f }},	// Vertex 0 - bottom left
		{{1.0f, -1.0f, 0.0f},  {1.0f, 1.0f, 1.0f, 1.0f },  { 1.0f, 1.0f }},	// Vertex 1 - bottom right
		{{1.0f, 1.0f, 0.0f},   {1.0f, 1.0f, 1.0f, 1.0f },  { 1.0f, 0.0f }},	// Vertex 2 - top right
		{{-1.0f, 1.0f, 0.0f},  {1.0f, 1.0f, 1.0f, 1.0f },  { 0.0f, 0.0f }}	// Vertex 3 - top left
	};
	 
	indices = {	
		0, 1, 2, 
		2, 3, 0 
	}; 

	MeshTransform meshData = {}; 
	meshData.position = position; 
	meshData.scale = glm::vec3(1.0f);

	vertexCount = vertices.size();
	indexCount = indices.size();

	return meshData;
}

MeshTransform BaseBladeShape::generateShape()
{
	vertices = {
		{ {-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f},	{ 0.0f, 1.0f }},	// Vertex 0 - bottom left
		{ {0.5f, -0.5f, 0.0f},	{1.0f, 0.0f, 0.0f, 1.0f},	{ 1.0f, 1.0f }},	// Vertex 1 - bottom right
		{ {0.5f, 0.5f, 0.0f},	{0.0f, 1.0f, 0.0f, 1.0f} ,	{ 1.0f, 0.0f }},	// Vertex 2 - top right
		{ {-0.5f, 0.5f, 0.0f},	{0.0f, 0.0f, 1.0f, 1.0f},	{ 0.0f, 0.0f }}		// Vertex 3 - top left
	};

	indices = { 
		0, 1, 2,
		2, 3, 0
	};

	MeshTransform meshData = {};
	meshData.position = glm::vec3(0.0f);
	meshData.scale = glm::vec3(1.0f);

	vertexCount = vertices.size();
	indexCount = indices.size();

	return meshData;
}
