#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Dynamic camera

struct CameraDataDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { 40.0f, -20.0f, 5.0f };
	const float pitch = 0.0f;//81.0f;
	const float yaw = 0.0f;// 136.f;
	const float fov = 45.0f; // Degrees.
};

struct CameraGoodPhotoDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { -2.0f, 8.0f, 3.0f };
	const float pitch = 83.0f;
	const float yaw = -69.0f;
	const float fov = 45.0f; // Degrees.
};

struct SideCameraDataDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { 40.0f, -100.0f, 5.0f };
	const float pitch = 81.0f;
	const float yaw = 42.0f;
	const float fov = 45.0f; // Degrees.
};

struct TopCameraDataDefaults {
	const glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
	const glm::vec3 position = { 17.0f, -46.0f, 100.0f };
	const float pitch = 11.0f;
	const float yaw = 134.0f;
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
	float farPlane = 1000.f;

	glm::mat4 getViewMatrix() const;
	glm::mat4 getRotationMatrix() const;
	float getFOV() const;

	void update();
	void setFront();
	void setSide();
	void setTop(); 
	void setGoodPhoto();
};
