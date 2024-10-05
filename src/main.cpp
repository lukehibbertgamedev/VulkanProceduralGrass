#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <imgui.h>

#include <VulkanApplication.h>

#include <iostream>
#include <cstdio>
#include <stdio.h>
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

    
    VkResult ret = vkApp.createInstance();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad instance.");
    vkApp.createDebugMessenger();

    ret = vkApp.createGlfwSurface(window);
    if (ret != VK_SUCCESS) throw std::runtime_error("bad surface.");

    ret = vkApp.createPhysicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad physical device.");

    ret = vkApp.createLogicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad logical device.");

    ret = vkApp.createSwapchain(window);
    if (ret != VK_SUCCESS) throw std::runtime_error("bad swapchain.");

    ret = vkApp.createSwapchainImageViews();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad swapchain image views.");

    std::cout << "\nSuccessful initialization.\n";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    for (auto imageView : vkApp.swapChainImageViews) {
        vkDestroyImageView(vkApp.m_LogicalDevice, imageView, nullptr);
    }

    vkDestroyPipelineLayout(vkApp.m_LogicalDevice, vkApp.pipelineLayout, nullptr);

    vkDestroySwapchainKHR(vkApp.m_LogicalDevice, vkApp.swapChain, nullptr);
    vkDestroyDevice(vkApp.m_LogicalDevice, nullptr);
    vkDestroySurfaceKHR(vkApp.m_VkInstance, vkApp.m_SurfaceKHR, nullptr);
    
    if (kEnableValidationLayers) {
        vkApp.destroyDebugUtilsMessengerEXT(vkApp.m_VkInstance, vkApp.m_DebugMessenger, nullptr);
    }

    vkDestroyInstance(vkApp.m_VkInstance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
