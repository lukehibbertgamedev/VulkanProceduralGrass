#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <imgui.h>

#include <VulkanApplication.h>

#include <iostream>
#include <vector>
#include <set>


int main() {
    
    VulkanApplication vkApp = {};

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("failed to initialize glfw!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan procedural grass rendering", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("failed to create glfw window!");
    }

    uint32_t extensionCount = 0;
    VkExtensionProperties props = {};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported:\n";
    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
    for (auto& extension : extensionProperties) {
        std::cout << "- " << extension.extensionName;
        std::cout << ".\tVersion: " << extension.specVersion << "\n";
    }


    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    if (kEnableValidationLayers) {
        vkApp.destroyDebugUtilsMessengerEXT(vkApp.m_VkInstance, vkApp.m_DebugMessenger, nullptr);
    }

    vkDestroyInstance(vkApp.m_VkInstance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
