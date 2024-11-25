#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Dynamic camera

struct CameraDataDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { 0.0f, -3.5f, 1.5f };
	const float pitch = 81.0f;
	const float yaw = 0.f;
	const float fov = 45.0f; // Degrees.
};

struct SideCameraDataDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { -2.5f, 0.3f, 0.35f };
	const float pitch = 91.0f;
	const float yaw = -90.0f;
	const float fov = 45.0f; // Degrees.
};

struct TopCameraDataDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { 0.0f, 0.18f, 5.1f };
	const float pitch = 4.0f;
	const float yaw = 0.0f;
	const float fov = 45.0f; // Degrees.
};

class Camera {
public:
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f }; // camera movement speed.
	glm::vec3 position = { 0.0f, -3.0f, 1.5f };

	glm::vec2 sensitivity = { 0.0f, 0.0f }; // camera rotation speed.
	float pitch= -30.f;
	float yaw = 0.f;
	float fov = 45.0f; // Degrees.
	float nearPlane = 0.1f;
	float farPlane = 100.f;

	glm::mat4 getViewMatrix() const;
	glm::mat4 getRotationMatrix() const;
	float getFOV() const;

	void update();
	void setFront();
	void setSide();
	void setTop(); 
};


/*
* View frustum culling class
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <array>
#include <math.h>
class Frustum {
public:
	enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };
	std::array<glm::vec4, 6> planes;

	void update(glm::mat4 matrix)
	{
		planes[LEFT].x = matrix[0].w + matrix[0].x;
		planes[LEFT].y = matrix[1].w + matrix[1].x;
		planes[LEFT].z = matrix[2].w + matrix[2].x;
		planes[LEFT].w = matrix[3].w + matrix[3].x;

		planes[RIGHT].x = matrix[0].w - matrix[0].x;
		planes[RIGHT].y = matrix[1].w - matrix[1].x;
		planes[RIGHT].z = matrix[2].w - matrix[2].x;
		planes[RIGHT].w = matrix[3].w - matrix[3].x;

		planes[TOP].x = matrix[0].w - matrix[0].y;
		planes[TOP].y = matrix[1].w - matrix[1].y;
		planes[TOP].z = matrix[2].w - matrix[2].y;
		planes[TOP].w = matrix[3].w - matrix[3].y;

		planes[BOTTOM].x = matrix[0].w + matrix[0].y;
		planes[BOTTOM].y = matrix[1].w + matrix[1].y;
		planes[BOTTOM].z = matrix[2].w + matrix[2].y;
		planes[BOTTOM].w = matrix[3].w + matrix[3].y;

		planes[BACK].x = matrix[0].w + matrix[0].z;
		planes[BACK].y = matrix[1].w + matrix[1].z;
		planes[BACK].z = matrix[2].w + matrix[2].z;
		planes[BACK].w = matrix[3].w + matrix[3].z;

		planes[FRONT].x = matrix[0].w - matrix[0].z;
		planes[FRONT].y = matrix[1].w - matrix[1].z;
		planes[FRONT].z = matrix[2].w - matrix[2].z;
		planes[FRONT].w = matrix[3].w - matrix[3].z;

		for (auto i = 0; i < planes.size(); i++)
		{
			float length = sqrtf(planes[i].x * planes[i].x + planes[i].y * planes[i].y + planes[i].z * planes[i].z);
			planes[i] /= length;
		}
	}

	bool checkSphere(glm::vec3 pos, float radius)
	{
		for (auto i = 0; i < planes.size(); i++)
		{
			if ((planes[i].x * pos.x) + (planes[i].y * pos.y) + (planes[i].z * pos.z) + planes[i].w <= -radius)
			{
				return false;
			}
		}
		return true;
	}
};