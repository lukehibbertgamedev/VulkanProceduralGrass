#include "Camera.h"
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

glm::mat4 Camera::getViewMatrix() const
{
	glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), position);
	glm::mat4 cameraRotation = getRotationMatrix();
	return glm::inverse(cameraTranslation * cameraRotation);
}

glm::mat4 Camera::getRotationMatrix() const 
{
	glm::quat pitchRotation = glm::angleAxis(glm::radians(pitch), glm::vec3{ 1.f, 0.f, 0.f });
	glm::quat yawRotation = glm::angleAxis(glm::radians(yaw), glm::vec3{ 0.f, 0.f, 1.f });
	return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

float Camera::getFOV() const
{
    return fov;
}

void Camera::update()
{
    glm::mat4 cameraRotation = getRotationMatrix();
    position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.5f, 0.f));
}

void Camera::reset()
{
    CameraDataDefaults defaults;

    position = defaults.position;
    velocity = defaults.velocity;
    pitch = defaults.pitch;
    yaw = defaults.yaw;
    fov = defaults.fov;
}

void Camera::sideSide()
{
    SideCameraDataDefaults defaults;

    position = defaults.position;
    velocity = defaults.velocity;
    pitch = defaults.pitch;
    yaw = defaults.yaw;
    fov = defaults.fov;
}
