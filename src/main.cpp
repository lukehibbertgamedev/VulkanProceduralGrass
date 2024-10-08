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


static void frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

int main() {

    VulkanApplication vkApp = {};

    // Initialize glfw for window creation.
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("failed to initialize glfw!");
    }

    // Create the window.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan procedural grass rendering", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("failed to create glfw window!");
    }
    glfwSetWindowUserPointer(window, &vkApp);
    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);

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


    vkApp.linkWindowToVulkan(window);

    VkResult ret = vkApp.createInstance();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad instance.");   

    ret = vkApp.createDebugMessenger();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad debug messenger.");

    ret = vkApp.createGlfwSurface();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad surface.");

    ret = vkApp.createPhysicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad physical device.");

    ret = vkApp.createLogicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad logical device.");

    ret = vkApp.createSwapchain();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad swapchain.");

    ret = vkApp.createSwapchainImageViews();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad swapchain image views.");

    ret = vkApp.createRenderPass();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad render pass.");

    ret = vkApp.createDescriptorSetLayout();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad descriptor layout.");

    ret = vkApp.createGraphicsPipeline();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad graphics pipeline.");

    ret = vkApp.createCommandPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad command pool.");

    ret = vkApp.createColourResources();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad colour resources.");

    ret = vkApp.createDepthResources();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad depth resources.");

    ret = vkApp.createFrameBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad frame buffers.");

    ret = vkApp.createTextureImage();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad texture image.");

    ret = vkApp.createTextureImageView();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad texture image view.");

    ret = vkApp.createTextureSampler();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad texture sampler.");

    ret = vkApp.createVertexBuffer();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad vertex buffer.");

    ret = vkApp.createIndexBuffer(); 
    if (ret != VK_SUCCESS) throw std::runtime_error("bad index buffer.");

    ret = vkApp.createUniformBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad uniform buffer.");

    ret = vkApp.createDescriptorPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad descriptor pool.");

    ret = vkApp.createDescriptorSets(); 
    if (ret != VK_SUCCESS) throw std::runtime_error("bad descriptor sets.");

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
    vkApp.cleanupApplication(window);

    return 0;
}
