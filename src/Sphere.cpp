#include "Sphere.h"

MeshInstance Sphere::generateFlatSphere(glm::vec3 position, float radius, int sectorCount, int stackCount, int up)
{
	// https://www.songho.ca/opengl/gl_sphere.html

	this->radius = radius;
	this->sectorCount = sectorCount;
	this->stackCount = stackCount;
	this->up = up;

	//vertices.clear();

	const float PI = acos(-1.0f);
	
	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;
		float xy = radius * cosf(stackAngle);
		float z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;

			float vx = xy * cosf(sectorAngle);
			float vy = xy * sinf(sectorAngle);
			float vz = z;
			float s = (float)j / sectorCount;
			float t = (float)i / stackCount;
			addVertex(glm::vec3(vx, vy, vz), glm::vec2(s, t), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}

			// store indices for lines
			// vertical lines for all stacks, k1 => k2
			lineIndices.push_back(k1);
			lineIndices.push_back(k2);
			if (i != 0)  // horizontal lines except 1st stack, k1 => k+1
			{
				lineIndices.push_back(k1);
				lineIndices.push_back(k1 + 1);
			}
		}
	}

	if (this->up != 3) {
		changeUpAxis(3, this->up);
	}

	MeshInstance meshData = {};
	meshData.position = position;
	meshData.scale = glm::vec3(radius);
	return meshData;
}

void Sphere::addVertex(glm::vec3 pos, glm::vec2 tex, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
{
	Vertex vertex; 
	vertex.pos = pos; 
	vertex.texCoord = tex;
	vertex.color = color;
	vertices.push_back(vertex); 
}

void Sphere::addIndex(uint16_t triIndex1, uint16_t triIndex2, uint16_t triIndex3)
{
	indices.push_back(triIndex1);
	indices.push_back(triIndex2);
	indices.push_back(triIndex3);
}

void Sphere::addLineIndex(uint16_t index)
{
	lineIndices.push_back(index);
}

std::vector<float> Sphere::computeFaceNormal(glm::vec3 x, glm::vec3 y, glm::vec3 z)
{
	const float EPSILON = 0.000001f;

	std::vector<float> normal(3, 0.0f);     // default return value (0,0,0) 
	float nx, ny, nz; 

	float x1 = x.x, x2 = x.y, x3 = x.z; 
	float y1 = y.x, y2 = y.y, y3 = y.z; 
	float z1 = z.x, z2 = z.y, z3 = z.z; 

	// find 2 edge vectors: v1-v2, v1-v3
	float ex1 = x2 - x1;  
	float ey1 = y2 - y1;
	float ez1 = z2 - z1; 
	float ex2 = x3 - x1; 
	float ey2 = y3 - y1; 
	float ez2 = z3 - z1; 

	// cross product: e1 x e2
	nx = ey1 * ez2 - ez1 * ey2; 
	ny = ez1 * ex2 - ex1 * ez2; 
	nz = ex1 * ey2 - ey1 * ex2; 

	// normalize only if the length is > 0
	float length = sqrtf(nx * nx + ny * ny + nz * nz); 
	if (length > EPSILON) 
	{
		// normalize
		float lengthInv = 1.0f / length;
		normal[0] = nx * lengthInv;
		normal[1] = ny * lengthInv;
		normal[2] = nz * lengthInv;
	}

	return normal;
}

void Sphere::changeUpAxis(int from, int to)
{
	// initial transform matrix cols
	float tx[] = { 1.0f, 0.0f, 0.0f };    // x-axis (left)
	float ty[] = { 0.0f, 1.0f, 0.0f };    // y-axis (up)
	float tz[] = { 0.0f, 0.0f, 1.0f };    // z-axis (forward)

	// X -> Y
	if (from == 1 && to == 2)
	{
		tx[0] = 0.0f; tx[1] = 1.0f;
		ty[0] = -1.0f; ty[1] = 0.0f;
	}
	// X -> Z
	else if (from == 1 && to == 3)
	{
		tx[0] = 0.0f; tx[2] = 1.0f;
		tz[0] = -1.0f; tz[2] = 0.0f;
	}
	// Y -> X
	else if (from == 2 && to == 1)
	{
		tx[0] = 0.0f; tx[1] = -1.0f;
		ty[0] = 1.0f; ty[1] = 0.0f;
	}
	// Y -> Z
	else if (from == 2 && to == 3)
	{
		ty[1] = 0.0f; ty[2] = 1.0f;
		tz[1] = -1.0f; tz[2] = 0.0f;
	}
	//  Z -> X
	else if (from == 3 && to == 1)
	{
		tx[0] = 0.0f; tx[2] = -1.0f;
		tz[0] = 1.0f; tz[2] = 0.0f;
	}
	// Z -> Y
	else
	{
		ty[1] = 0.0f; ty[2] = -1.0f;
		tz[1] = 1.0f; tz[2] = 0.0f;
	}

	std::size_t i, j;
	std::size_t count = vertices.size();
	float vx, vy, vz;
	for (i = 0, j = 0; i < count; i += 3, j += 8)
	{
		// transform vertices
		vx = vertices[i].pos.x;
		vy = vertices[i + 1].pos.y;
		vz = vertices[i + 2].pos.z;
		vertices[i].pos.x = tx[0] * vx + ty[0] * vy + tz[0] * vz;   // x 
		vertices[i + 1].pos.y = tx[1] * vx + ty[1] * vy + tz[1] * vz;   // y
		vertices[i + 2].pos.z = tx[2] * vx + ty[2] * vy + tz[2] * vz;   // z
	}
}