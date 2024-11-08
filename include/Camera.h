#pragma once 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Dynamic camera

class Camera {
public:
	Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float fov, float aspect, float nearPlane, float farPlane);

	void initialise();
	void processInput(int key, float elapsed);

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	float yaw = 0.0f;
	float pitch = 0.0f;

	float moveSpeed = 3.0f;

	float fov;
	float aspect;
	float nearPlane;
	float farPlane;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};