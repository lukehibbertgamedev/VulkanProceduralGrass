#include "Sphere.h"

void Sphere::generateFlatSphere(float radius, int sectorCount, int stackCount, int up)
{
	this->radius = radius;
	this->sectorCount = sectorCount;
	this->stackCount = stackCount;
	this->up = up;

	const float PI = acos(-1.0f);

	// temp
	struct TempVertex { float x, y, z, s, t; };
	std::vector<TempVertex> tmpVertices;
	
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
			TempVertex tmpVertex;
			tmpVertex.x = xy * cosf(sectorAngle);
			tmpVertex.y = xy * sinf(sectorAngle);
			tmpVertex.z = z;
			tmpVertex.s = (float)j / sectorCount;
			tmpVertex.t = (float)i / stackCount;
			tmpVertices.push_back(tmpVertex);
		}
	}

	vertices.clear();
	TempVertex v1, v2, v3, v4;
	std::vector<float> n;

	int i, j, k, vi1, vi2;
	int index = 0;

	for (int i = 0; i < stackCount; ++i) {
		vi1 = i * (sectorCount + 1);
		vi2 = (i + 1) * (sectorCount + 1);

		for (int j = 0; j < sectorCount; ++j)
		{
			v1 = tmpVertices[vi1];
			v2 = tmpVertices[vi2];
			v3 = tmpVertices[vi1 + 1]; 
			v4 = tmpVertices[vi2 + 1];

			if (i == 0) {
				addVertex(glm::vec3(v1.x, v1.y, v1.z), glm::vec2(v1.s, v1.t));  
				addVertex(glm::vec3(v2.x, v2.y, v2.z), glm::vec2(v2.s, v2.t)); 
				addVertex(glm::vec3(v4.x, v4.y, v4.z), glm::vec2(v4.s, v4.t)); 

				n = computeFaceNormal(glm::vec3(v1.x, v1.y, v1.z), glm::vec3(v2.x, v2.y, v2.z), glm::vec3(v4.x, v4.y, v4.z));

				for (int k = 0; k < 3; ++k)
				{
					// add normals
				}

				addIndex(index, index + 1, index + 2);
				addLineIndex(index);
				addLineIndex(index + 1);
				index += 3; // per tri
			}

			else if (i == (stackCount - 1)) {
				addVertex(glm::vec3(v1.x, v1.y, v1.z), glm::vec2(v1.s, v1.t));
				addVertex(glm::vec3(v2.x, v2.y, v2.z), glm::vec2(v2.s, v2.t));
				addVertex(glm::vec3(v3.x, v3.y, v3.z), glm::vec2(v3.s, v3.t));

				n = computeFaceNormal(glm::vec3(v1.x, v1.y, v1.z), glm::vec3(v2.x, v2.y, v2.z), glm::vec3(v3.x, v3.y, v3.z)); 

				for (int k = 0; k < 3; ++k)
				{
					// add normals
				}

				addIndex(index, index + 1, index + 2);
				addLineIndex(index);
				addLineIndex(index + 1);
				addLineIndex(index);
				addLineIndex(index + 2);
				index += 3; // per tri
			}

			else { // 2 tris for others
				addVertex(glm::vec3(v1.x, v1.y, v1.z), glm::vec2(v1.s, v1.t));
				addVertex(glm::vec3(v2.x, v2.y, v2.z), glm::vec2(v2.s, v2.t));
				addVertex(glm::vec3(v3.x, v3.y, v3.z), glm::vec2(v3.s, v3.t));
				addVertex(glm::vec3(v4.x, v4.y, v4.z), glm::vec2(v4.s, v4.t));

				n = computeFaceNormal(glm::vec3(v1.x, v1.y, v1.z), glm::vec3(v2.x, v2.y, v2.z), glm::vec3(v3.x, v3.y, v3.z));

				for (int k = 0; k < 3; ++k)
				{
					// add normals
				}

				addIndex(index, index + 1, index + 2);
				addIndex(index + 2, index + 1, index + 3);

				addLineIndex(index);
				addLineIndex(index + 1);
				addLineIndex(index);
				addLineIndex(index + 2);

				index += 4;
			}
		}
	}

	if (this->up != 3) {
		changeUpAxis(3, this->up);
	}

	// make all white
	for (int i = 0; i < vertices.size(); ++i) {
		vertices[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	}
}

void Sphere::addVertex(glm::vec3 pos, glm::vec2 tex)
{
	Vertex vertex;
	vertex.pos = pos;
	vertex.texCoord = tex;

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