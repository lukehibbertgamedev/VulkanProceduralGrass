#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Dynamic camera

class Camera {
public:

	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 position = { 0.0f, -3.0f, 1.5f };

	float pitch= -30.f;
	float yaw = 0.f;
	float fov = 45.0f; // Degrees.
	float nearPlane = 0.1f;
	float farPlane = 100.f;

	glm::mat4 getViewMatrix();
	glm::mat4 getRotationMatrix();
	float getFOV();

	void processGlfwKeyEvent(int key, int action);

	void update();
	void reset();
};