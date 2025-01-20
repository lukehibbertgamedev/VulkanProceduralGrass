#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Dynamic camera

struct Defaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { -29.0f, 60.0f, 5.0f };
	const float pitch = 87.0f;
	const float yaw = -130.0f;
	const float fov = 45.0f; // Degrees.
};

class Camera {
public:
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f }; // Movement.
	glm::vec3 position = { 0.0f, -3.0f, 1.5f };

	glm::vec2 sensitivity = { 0.0f, 0.0f }; // Rotation.
	float pitch= -30.f;
	float yaw = 0.f;
	float fov = 45.0f; // Stored in degrees.
	float nearPlane = 0.1f;
	float farPlane = 1000.f;

	glm::mat4 getViewMatrix() const;
	glm::mat4 getRotationMatrix() const;
	float getFOV() const;

	void update();
	void setDefault();
};
