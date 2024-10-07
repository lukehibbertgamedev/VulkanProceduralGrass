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

    // Initialize glfw for window creation.
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("failed to initialize glfw!");
    }

    // Create the window.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan procedural grass rendering", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("failed to create glfw window!");
    }

    // Display supported extensions 
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

    // Vulkan application initialization:

    VulkanApplication vkApp = {}; 

    VkResult ret = vkApp.createInstance();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad instance.");
   
    ret = vkApp.createDebugMessenger();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad debug messenger.");

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

    ret = vkApp.createRenderPass();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad render pass.");

    ret = vkApp.createGraphicsPipeline();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad graphics pipeline.");

    ret = vkApp.createFrameBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad frame buffers.");

    ret = vkApp.createCommandPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad command pool.");

    ret = vkApp.createCommandBuffer();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad command buffer.");

    ret = vkApp.createSynchronizationObjects();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad semaphores | fences.");

    std::cout << "\nSuccessful initialization.\n";


    // Main update & render loop:

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        vkApp.render();
    }

    // All operations in vkApp.render() are asynchronous, ensure all operations have completed before termination.
    vkDeviceWaitIdle(vkApp.m_LogicalDevice);

    // Clean up/termination of allocated Vulkan objects (in the reverse order to initialization):

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vkApp.m_LogicalDevice, vkApp.renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vkApp.m_LogicalDevice, vkApp.imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vkApp.m_LogicalDevice, vkApp.inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(vkApp.m_LogicalDevice, vkApp.commandPool, nullptr);

    for (auto framebuffer : vkApp.swapChainFramebuffers) {
        vkDestroyFramebuffer(vkApp.m_LogicalDevice, framebuffer, nullptr);
    }

    for (auto imageView : vkApp.swapChainImageViews) {
        vkDestroyImageView(vkApp.m_LogicalDevice, imageView, nullptr);
    }

    vkDestroyPipeline(vkApp.m_LogicalDevice, vkApp.graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vkApp.m_LogicalDevice, vkApp.pipelineLayout, nullptr);

    vkDestroyRenderPass(vkApp.m_LogicalDevice, vkApp.renderPass, nullptr);

    vkDestroySwapchainKHR(vkApp.m_LogicalDevice, vkApp.swapChain, nullptr);

    vkDestroyDevice(vkApp.m_LogicalDevice, nullptr);

    vkDestroySurfaceKHR(vkApp.m_VkInstance, vkApp.m_SurfaceKHR, nullptr);
    
    if (kEnableValidationLayers) {
        vkApp.destroyDebugUtilsMessengerEXT(vkApp.m_VkInstance, vkApp.m_DebugMessenger, nullptr);
    }

    vkDestroyInstance(vkApp.m_VkInstance, nullptr);

    // Glfw clean up/termination (ensure the window is terminated before glfw to avoid a memory leak).

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
