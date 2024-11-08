#include "Camera.h"
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

glm::mat4 Camera::getViewMatrix()
{
	glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), position);
	glm::mat4 cameraRotation = getRotationMatrix();
	return glm::inverse(cameraTranslation * cameraRotation);
}

glm::mat4 Camera::getRotationMatrix()
{
	glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3{ 1.f, 0.f, 0.f });
	glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3{ 0.f, -1.f, 0.f });
	return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

float Camera::getFOV()
{
    return fov;
}

void Camera::processGlfwKeyEvent(int key, int action)
{
    
}

void Camera::update()
{
    glm::mat4 cameraRotation = getRotationMatrix();
    position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.5f, 0.f));
}

void Camera::reset()
{
    position = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);
    pitch = 0.0f;
    yaw = 0.0f;
}
