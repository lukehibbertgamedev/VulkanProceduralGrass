#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>


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
    vkApp.lastTime = glfwGetTime();

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

    ret = vkApp.createComputeDescriptorSetLayout();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad compute descriptor layout.");

    //ret = vkApp.createDescriptorSetLayout();
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad descriptor layout.");

    ret = vkApp.createGraphicsPipeline();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad graphics pipeline.");

    ret = vkApp.createComputePipeline();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad compute pipeline.");

    ret = vkApp.createFrameBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad frame buffers.");

    ret = vkApp.createCommandPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad command pool.");

    //ret = vkApp.createColourResources();
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad colour resources.");

    //ret = vkApp.createDepthResources();
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad depth resources.");    

    //ret = vkApp.createTextureImage();
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad texture image.");

    //ret = vkApp.createTextureImageView();
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad texture image view.");

    //ret = vkApp.createTextureSampler();
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad texture sampler.");

    //ret = vkApp.createVertexBuffer();
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad vertex buffer.");

    //ret = vkApp.createIndexBuffer(); 
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad index buffer.");

    ret = vkApp.createShaderStorageBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad shader storage buffer.");

    ret = vkApp.createUniformBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad uniform buffer.");

    ret = vkApp.createDescriptorPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad descriptor pool.");

    ret = vkApp.createComputeDescriptorSets();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad compute descriptor sets.");

    //ret = vkApp.createDescriptorSets(); 
    //if (ret != VK_SUCCESS) throw std::runtime_error("bad descriptor sets.");

    ret = vkApp.createCommandBuffer();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad command buffer.");

    ret = vkApp.createComputeCommandBuffer();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad compute command buffer.");

    ret = vkApp.createSynchronizationObjects();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad semaphores | fences.");

    ret = vkApp.createImGuiImplementation();
    if (ret != VK_SUCCESS) throw std::runtime_error("bad imgui implementation.");

    std::cout << "\nSuccessful initialization.\n";


    // Main update & render loop:

    while (!glfwWindowShouldClose(window)) {
        
        glfwPollEvents();


        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        vkApp.prepareImGuiDrawData();

        ImGui::Render();        
        vkApp.render();

        // We want to animate the particle system using the last frames time to get smooth,
        // frame-rate independent animation
        double currentTime = glfwGetTime();
        vkApp.lastFrameTime = (currentTime - vkApp.lastTime) * 1000.0;
        vkApp.lastTime = currentTime;
    }

    // All operations in vkApp.render() are asynchronous, ensure all operations have completed before termination.
    vkDeviceWaitIdle(vkApp.m_LogicalDevice);

    // Clean up/termination of allocated Vulkan objects (in the reverse order to initialization):
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkApp.cleanupApplication(window);

    return 0;
}
