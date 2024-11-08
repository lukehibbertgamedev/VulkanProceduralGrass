#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Dynamic camera

struct CameraDataDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { 0.0f, -3.0f, 1.5f };
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
	glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	glm::vec3 position = { 0.0f, -3.0f, 1.5f };

	float pitch= -30.f;
	float yaw = 0.f;
	float fov = 45.0f; // Degrees.
	float nearPlane = 0.1f;
	float farPlane = 100.f;

	glm::mat4 getViewMatrix() const;
	glm::mat4 getRotationMatrix() const;
	float getFOV() const;

	void update();
	void reset();
	void setSide();
	void setTop(); 
};