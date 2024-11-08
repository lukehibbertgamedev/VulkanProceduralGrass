#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 up, float fov, float aspect, float nearPlane, float farPlane)
	: position(position), front(glm::normalize(target-position)), up(up), right(normalize(glm::cross(front, up))), fov(fov), aspect(aspect), nearPlane(nearPlane), farPlane(farPlane)
{
	// update view update proj.
}

void Camera::initialise()
{
}

void Camera::processInput(int key, float elapsed)
{
}
