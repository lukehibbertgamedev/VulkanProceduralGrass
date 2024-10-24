#include "Sphere.h"

MeshInstance Sphere::generateFlatSphere(glm::vec3 position, float radius, int sectorCount, int stackCount, int up)
{
	// https://www.songho.ca/opengl/gl_sphere.html

	this->radius = radius;
	this->sectorCount = sectorCount;
	this->stackCount = stackCount;
	this->up = up;

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
		}
	}

	if (this->up != 3) {
		changeUpAxis(3, this->up);
	}

	MeshInstance meshData = {};
	meshData.position = position;
	meshData.scale = glm::vec3(radius);

	vertexCount = vertices.size();
	indexCount = indices.size();

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