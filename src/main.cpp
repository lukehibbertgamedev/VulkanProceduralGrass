#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <VulkanApplication.h>
#include <Camera.h>

#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <vector>
#include <set>
#include <chrono>

static Camera globalCamera;

// TODO: Move to its own header (callbacks.h).
static void frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        default: break;

        case GLFW_KEY_W: globalCamera.velocity.z = -.01f; break;
        case GLFW_KEY_S: globalCamera.velocity.z = .01f; break;

        case GLFW_KEY_A: globalCamera.velocity.x = -.01f; break;
        case GLFW_KEY_D: globalCamera.velocity.x = .01f; break;

        case GLFW_KEY_E: globalCamera.velocity.y = -.01f; break;
        case GLFW_KEY_Q: globalCamera.velocity.y = .01f; break;

        case GLFW_KEY_L: globalCamera.fov += 1.0f; break;
        case GLFW_KEY_J: globalCamera.fov -= 1.0f; break;

        case GLFW_KEY_UP: globalCamera.sensitivity.x = .05f; break;
        case GLFW_KEY_DOWN: globalCamera.sensitivity.x = -.05f; break; //pitch
        case GLFW_KEY_LEFT: globalCamera.sensitivity.y = .05f; break;
        case GLFW_KEY_RIGHT: globalCamera.sensitivity.y = -.05f; break; // yawe

        case GLFW_KEY_R: globalCamera.setFront(); break;
        case GLFW_KEY_T: globalCamera.setSide(); break;
        case GLFW_KEY_Y: globalCamera.setTop(); break;
        }
    }

    else if (action == GLFW_RELEASE) {
        switch (key) {
        default: break;

        case GLFW_KEY_W: globalCamera.velocity.z = 0.0f; break;
        case GLFW_KEY_S: globalCamera.velocity.z = 0.0f; break;
        case GLFW_KEY_A: globalCamera.velocity.x = 0.0f; break;
        case GLFW_KEY_D: globalCamera.velocity.x = 0.0f; break;
        case GLFW_KEY_E: globalCamera.velocity.y = 0.0f; break;
        case GLFW_KEY_Q: globalCamera.velocity.y = 0.0f; break;

        case GLFW_KEY_UP: globalCamera.sensitivity.x = 0.0f; break;
        case GLFW_KEY_DOWN: globalCamera.sensitivity.x = 0.0f; break; //pitch
        case GLFW_KEY_LEFT: globalCamera.sensitivity.y = 0.0f; break;
        case GLFW_KEY_RIGHT: globalCamera.sensitivity.y = 0.0f; break; // yawe
        }
    }
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

    // Link functoinality to determine key presses, mostly for the dynamic camera.
    glfwSetKeyCallback(window, keyCallback);

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

    vkApp.linkCameraToVulkan(&globalCamera);

    VkResult ret = vkApp.createDefaultCamera(); 
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create camera.");

    ret = vkApp.createInstance();
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

    ret = vkApp.createDepthResources();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create depth resources.");

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

    vkApp.createNumBladesBuffer();

    ret = vkApp.createDescriptorSets();  
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create descriptor sets."); 

    vkApp.createBladeInstanceStagingBuffer(); 

    ret = vkApp.createCommandBuffers(); 
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create command buffer.");

    ret = vkApp.createSynchronizationObjects();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create semaphores | fences.");

    ret = vkApp.createImGuiImplementation();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create imgui implementation.");

    std::cout << "\nSuccessful initialization.\n";

    // Main application loop:
    while (!glfwWindowShouldClose(window)) {
        
        // TODO: Remove vkApp.time variables as timer should do this for you.
        // We want to animate the particle system using the last frames time to get smooth, frame-rate independent animation
        double currentTime = glfwGetTime();
        vkApp.lastFrameTime = (currentTime - vkApp.lastTime) * 1000.0;
        vkApp.lastTime = currentTime;
        vkApp.deltaTime = timer.getDeltaTime();

        // Process any window events, calls any associated callbacks (including camera.processKey).
        glfwPollEvents();

        // Create new ImGui frames via its backend and context.
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Prepare ImGui draw data, anything within the ImGui:: namspace this doesn't touch the GPU or graphics API at all.
        vkApp.prepareImGuiDrawData();

        // Update camera data.
        globalCamera.update();

        // Prepare the ImGui data for rendering so you can call GetDrawData().
        ImGui::Render();        
        
        // Record and execute commands through a compute and graphics pipeline.
        vkApp.render();
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
