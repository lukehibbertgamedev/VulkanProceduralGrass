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
#include <chrono>

// TODO: Move to its own header (callbacks.h).
static void frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

// TODO: Move to its own header (timer.h).
class Timer {
public:
    Timer() {
        lastTime = std::chrono::high_resolution_clock::now();
    }

    float getDeltaTime() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        return deltaTime.count(); // Return delta time in seconds
    }

private:
    std::chrono::high_resolution_clock::time_point lastTime;
};

int main() {

    // Create an empty application structure.
    VulkanApplication vkApp = {};

    // Create a timer instance to obtain delta time for the application loop, auto initialises timer.lastTime.
    Timer timer;

    // Initialize glfw for window creation.
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("failed to initialize glfw!");
    }

    // Create the glfw window and its associated context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan procedural grass rendering", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("failed to create glfw window!");
    }

    // Link the application structure to the window.
    glfwSetWindowUserPointer(window, &vkApp);

    // Link functionality to when glfw detects its framebuffer has been resized (window resizing).
    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);

    // FIXME: May not be needed since timer has its own lastTime.
    // Begin the timer to retrieve delta time throughout the application loop.
    vkApp.lastTime = glfwGetTime();

    // TODO: Put in its own static function in its own header (extensionsupport.h).
    // Enumerate and output supported Vulkan extensions.
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
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create instance.");   

    ret = vkApp.createDebugMessenger();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create debug messenger.");

    ret = vkApp.createGlfwSurface();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create surface.");

    ret = vkApp.createPhysicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create physical device.");

    ret = vkApp.createLogicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create logical device.");

    ret = vkApp.createSwapchain();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create swapchain.");

    ret = vkApp.createSwapchainImageViews();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create swapchain image views.");

    ret = vkApp.createRenderPass();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create render pass.");

    ret = vkApp.createDescriptorSetLayouts();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create descriptor layout.");

    ret = vkApp.createPipelines();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create model | grass pipeline.");

    ret = vkApp.createFrameBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create frame buffers.");

    ret = vkApp.createCommandPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create command pool.");

    vkApp.createMeshObjects();

    vkApp.populateBladeInstanceBuffer(); // SSBO data.

    ret = vkApp.createVertexBuffer();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create vertex buffer.");

    ret = vkApp.createIndexBuffer();  
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create index buffer.");

    ret = vkApp.createShaderStorageBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create shader storage buffer.");

    ret = vkApp.createUniformBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create uniform buffer.");

    ret = vkApp.createDescriptorPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create descriptor pool.");

    ret = vkApp.createDescriptorSets();  
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create descriptor sets."); 

    vkApp.copyCPUBladeInstanceBufferToHostVisibleMemory(); 

    ret = vkApp.createCommandBuffer(); 
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create command buffer.");

    ret = vkApp.createSynchronizationObjects();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create semaphores | fences.");

    ret = vkApp.createImGuiImplementation();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create imgui implementation.");

    std::cout << "\nSuccessful initialization.\n";

    // Main application loop:
    while (!glfwWindowShouldClose(window)) {
        
        // Process any window events, calls any associated callbacks.
        glfwPollEvents();

        // Create new ImGui frames via its backend and context.
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Prepare ImGui draw data, anything within the ImGui:: namspace this doesn't touch the GPU or graphics API at all.
        vkApp.prepareImGuiDrawData();

        // Prepare the ImGui data for rendering so you can call GetDrawData().
        ImGui::Render();        
        
        // Record and execute commands through a compute and graphics pipeline.
        vkApp.render();

        // TODO: Remove vkApp.time variables as timer should do this for you.
        // We want to animate the particle system using the last frames time to get smooth, frame-rate independent animation
        double currentTime = glfwGetTime();
        vkApp.lastFrameTime = (currentTime - vkApp.lastTime) * 1000.0;
        vkApp.lastTime = currentTime;
        vkApp.deltaTime = timer.getDeltaTime();
    }

    // All operations in vkApp.render() are asynchronous, ensure all operations/commands have completed before termination.
    vkDeviceWaitIdle(vkApp.m_LogicalDevice);

    // Destroy the ImGui backends and context.
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean up/termination of allocated Vulkan objects (in the reverse order to initialization).
    vkApp.cleanupApplication(window);

    return 0;
}
