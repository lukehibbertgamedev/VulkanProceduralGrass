#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <VulkanApplication.h>
#include <Camera.h>
#include <Timer.h>

#include <fstream>
#include <iterator>
#include <cstdio>
#include <stdio.h>
#include <vector>
#include <set>

// Global main camera.
static Camera globalCamera;

// Determine action to take on window/framebuffer resize.
static void frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

// Determine action to take on keyboard inputs.
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    const float moveSpeed = 0.1f;

    if (action == GLFW_PRESS) {
        switch (key) {
        default: break;

        case GLFW_KEY_W: globalCamera.velocity.z = -moveSpeed; break;
        case GLFW_KEY_S: globalCamera.velocity.z = moveSpeed; break;

        case GLFW_KEY_A: globalCamera.velocity.x = -moveSpeed; break;
        case GLFW_KEY_D: globalCamera.velocity.x = moveSpeed; break;

        case GLFW_KEY_E: globalCamera.velocity.y = -moveSpeed; break;
        case GLFW_KEY_Q: globalCamera.velocity.y = moveSpeed; break;

        case GLFW_KEY_L: globalCamera.fov += 1.0f; break;
        case GLFW_KEY_J: globalCamera.fov -= 1.0f; break;

        case GLFW_KEY_UP: globalCamera.sensitivity.x = .075f; break;
        case GLFW_KEY_DOWN: globalCamera.sensitivity.x = -.075f; break; //pitch
        case GLFW_KEY_LEFT: globalCamera.sensitivity.y = .075f; break;
        case GLFW_KEY_RIGHT: globalCamera.sensitivity.y = -.075f; break; // yawe

        case GLFW_KEY_R: globalCamera.setFront(); break;
        case GLFW_KEY_T: globalCamera.setSide(); break;
        case GLFW_KEY_Y: globalCamera.setTop(); break;
        case GLFW_KEY_U: globalCamera.setGoodPhoto(); break;
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

int main() {

    // Create an empty application structure.
    VulkanApplication vkApp = {};

    // Create a timer instance to obtain delta time for the application loop, auto initialises timer.lastTime.
    Timer timer = {};

    // Initialize glfw for window creation.
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("failed to initialize glfw!");
    }

    // Create the glfw window and its associated context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Vulkan procedural grass rendering", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("failed to create glfw window!");
    }

    // Link the application structure to the window.
    glfwSetWindowUserPointer(window, &vkApp);

    // Link functionality to determine key presses, mostly for the dynamic camera.
    glfwSetKeyCallback(window, keyCallback);

    // Link functionality to when glfw detects its framebuffer has been resized (window resizing).
    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);

    // Begin the timer to retrieve delta time throughout the application loop.
    vkApp.lastTime = glfwGetTime();

    // Vulkan application initialization:
    vkApp.linkWindowToVulkan(window);
    vkApp.linkCameraToVulkan(&globalCamera);
    VkResult ret = vkApp.initialiseApplication();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not initialise application.");

    // For use in monitoring frame time.
    int frameNum = 0;
    int wait = 1000;
    int count = 10000;
    std::vector<double> timeInMs; 
    timeInMs.reserve(count);
    bool writtenToFile = false;

    // Main application loop:
    while (!glfwWindowShouldClose(window)) {
        
        // Begin calculating frame time.
        auto t0 = std::chrono::high_resolution_clock::now(); 

        // Internal application timings.
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

        // Prepare ImGui draw data, this does not affect the graphics in any way, the drawing is done later.
        vkApp.prepareImGuiDrawData();

        // Update camera data.
        globalCamera.update();

        // Prepare the ImGui data for rendering so you can call GetDrawData().
        ImGui::Render();        
        
        // Record and execute commands through a compute and graphics pipeline.
        vkApp.render();

        // End and monitor frame time.
        auto t1 = std::chrono::high_resolution_clock::now();
        if (frameNum >= wait) { // Wait for n frames for scene to populate.
            auto t = t1 - t0;
            double tc = std::chrono::duration_cast<std::chrono::duration<double>>(t).count();
            double time = (tc * 1000);
            if (timeInMs.size() < count) { // Insert new frame timing if not at max count.
                timeInMs.push_back(time);
            }
            if (timeInMs.size() >= count && !writtenToFile) { // If at max count, write those timings to file.
                std::string fileName = "../assets/performance_timings/VulkanFrameTimings_";
                fileName += std::to_string(MAX_BLADES);
                fileName += ".txt"; 
                std::ofstream file(fileName);
                std::ostream_iterator<double> it(file, "\n");
                std::copy(std::begin(timeInMs), std::end(timeInMs), it); 
                writtenToFile = true;
            }
        }
        frameNum++;
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
