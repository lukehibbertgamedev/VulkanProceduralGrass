#include "VulkanApplication.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <chrono>

#include <stdexcept>
#include <iostream>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream> // Necssary for std::ifstream
#include <random>
#include <glm/gtx/quaternion.hpp>

#include "Utility.h"

// Todo: Wrap in ifdef vk debug
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl << std::endl;
    /*std::cout << "Debug Messenger\n";
    std::cout << "Type: ";
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) std::cout << "General\n ";
    else {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
                std::cout << "Performance | Validation\n ";
            }
            else {
                std::cout << "Performance\n ";
            }
        }
        else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
            std::cout << "Validation\n ";
        }
    }
    std::cout << "Severity: ";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) std::cout << "Verbose\n ";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) std::cout << "Info\n ";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) std::cout << "Warning\n ";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) std::cout << "Error\n ";
    if (pCallbackData->objectCount > 0) {
        std::cout << "Number of objects: " << pCallbackData->objectCount << "\n";
        for (uint32_t object = 0u; object < pCallbackData->objectCount; ++object)  {
            if (pCallbackData->pObjects[object].pObjectName != nullptr && strlen(pCallbackData->pObjects[object].pObjectName) > 0) {                
                std::cout << "\tObject: " << object  << " - Type ID " << pCallbackData->pObjects[object].objectType 
                    << ", Handle " << (void*)(pCallbackData->pObjects[object].objectHandle) << ", Name " << pCallbackData->pObjects[object].pObjectName << "\n";
            }
            else {
                std::cout << "\tObject: " << object << " - Type ID " << pCallbackData->pObjects[object].objectType
                    << ", Handle " << (void*)(pCallbackData->pObjects[object].objectHandle) << "\n";
            }
        }
    }

    if (pCallbackData->cmdBufLabelCount > 0) {
        std::cout << "Number of command buffer labels: " << pCallbackData->cmdBufLabelCount << "\n";
        for (uint32_t cmd_buf_label = 0u; cmd_buf_label < pCallbackData->cmdBufLabelCount; ++cmd_buf_label) {
            std::cout << "\tLabel: " << cmd_buf_label << " - " << pCallbackData->pCmdBufLabels[cmd_buf_label].pLabelName
                << " { " << pCallbackData->pCmdBufLabels[cmd_buf_label].color[0] << ", " << pCallbackData->pCmdBufLabels[cmd_buf_label].color[1]
                << ", " << pCallbackData->pCmdBufLabels[cmd_buf_label].color[2] << ", " << pCallbackData->pCmdBufLabels[cmd_buf_label].color[3] << " }\n";
        }
    }
    std::cout << "Message ID Number: " << pCallbackData->messageIdNumber << "\n";
    std::cout << "Message ID Name: " << pCallbackData->pMessageIdName << "\n";
    std::cout << "Message: " << pCallbackData->pMessage << "\n";*/

    return VK_FALSE;
}

static bool checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : kValidationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    return true;
}

static std::vector<const char*> getGlfwRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (kEnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        extensions.emplace_back(glfwExtensions[i]);
    }
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    return extensions;
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

static const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkShaderModule VulkanApplication::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
        return VkShaderModule{};
    }
    return shaderModule;
}

void VulkanApplication::render()
{
    //VkSubmitInfo submitInfo{};
    //submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Compute submission        
    //vkWaitForFences(m_LogicalDevice, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    //
    //
    //vkResetFences(m_LogicalDevice, 1, &computeInFlightFences[currentFrame]);
    //
    //vkResetCommandBuffer(computeCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    //recordComputeCommandBuffer(computeCommandBuffers[currentFrame]);
    //
    //submitInfo.commandBufferCount = 1;
    //submitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
    //submitInfo.signalSemaphoreCount = 1;
    //submitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];
    //
    //if (vkQueueSubmit(computeQueue, 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to submit compute command buffer!");
    //};

    // Graphics submission
    //vkWaitForFences(m_LogicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX); // Wait for compute to finish.

    vkWaitForFences(m_LogicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX); // Wait for compute to finish.

    uint32_t imageIndex;
    VkResult ret = vkAcquireNextImageKHR(m_LogicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (ret == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(currentFrame);
    // updateBladeInstanceBuffer ???

    vkResetFences(m_LogicalDevice, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    //VkSemaphore waitSemaphores[] = { /*computeFinishedSemaphores[currentFrame],*/ imageAvailableSemaphores[currentFrame] };
    //VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    ret = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

    if (ret != VK_SUCCESS) {
        int a = 5;
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
    //presentInfo.pResults = nullptr; // Optional

    ret = vkQueuePresentKHR(presentQueue, &presentInfo); // Send all image data to the screen, this will render this frame.

    if (ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapchain();
    }
    else if (ret != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // Frame complete, increment frame and wrap if it goes beyond max frames in flight.
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    frameCount++;
}

void VulkanApplication::updateUniformBuffer(uint32_t currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    glm::vec3 cameraPosition = glm::vec3(0.0f, -3.0f, 1.5f);
    glm::vec3 lookTowardsPoint = glm::vec3(0.0f, 0.0f, 0.5f);
    glm::vec3 worldUpVector = glm::vec3(0.0f, 0.0f, 1.0f); // Z is the natural UP vector.
    glm::mat4 view = glm::lookAtRH(cameraPosition, lookTowardsPoint, worldUpVector); 

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
    proj[1][1] *= -1; // Invert the y-axis due to differing coordinate systems.

    // Calculate correct rotation matrix for the plane to be flat ground.
    glm::mat4 rotationMatrixX = glm::rotate(glm::mat4(1.0f), glm::radians(groundPlane.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationMatrixY = glm::rotate(glm::mat4(1.0f), glm::radians(groundPlane.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationMatrixZ = glm::rotate(glm::mat4(1.0f), glm::radians(groundPlane.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotationMatrix = rotationMatrixX * rotationMatrixY * rotationMatrixZ;

    // Calculate the model, view, and projection matrix used by the vertex shader.
    CameraUniformBufferObject ubo = {};
    ubo.model = glm::translate(glm::mat4(1.0f), groundPlane.position) *  rotationMatrix * glm::scale(glm::mat4(1.0f), groundPlane.scale);
    ubo.view = view;
    ubo.proj = proj;

    // Copy the contents of the ubo structure into a mapped memory location effectively updating the uniform buffer with the most recent data.
    void* data;
    vkMapMemory(m_LogicalDevice, uniformBufferMemory, 0, sizeof(ubo), 0, &data); // Validation error: attempting to map memory on already mapped memory.
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_LogicalDevice, uniformBufferMemory);
}

VkResult VulkanApplication::createInstance() {   

    auto extensions = getGlfwRequiredExtensions();

    if (kEnableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    // Set up instance information structures:
    
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Procedural Grass Demo";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = (kEnableValidationLayers) ? static_cast<uint32_t>(kValidationLayers.size()) : 0;
    createInfo.ppEnabledLayerNames = (kEnableValidationLayers) ? kValidationLayers.data() : nullptr;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (kEnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
        createInfo.ppEnabledLayerNames = kValidationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    // Create Vulkan instance and write to class member.
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_VkInstance);
    if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createDebugMessenger()
{
    // If the validation layers aren't enabled, we don't want to return an error but m_DebugMessenger will be VK_NULL_HANDLE.
    if (!kEnableValidationLayers) return VK_SUCCESS;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS; 
}

VkResult VulkanApplication::createPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
    if (deviceCount <= 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());
    for (const auto& device : devices) {
        if (checkPhysicalDeviceSuitability(device)) {
            m_PhysicalDevice = device;
            //msaaSamples = getMaxUsableMSAASampleCount();
            break;
        }
    }

    // Print out details of all physical devices.
    for (int i = 0; i < deviceCount; ++i) {
        VkPhysicalDeviceProperties deviceProperties = {};
        memset(&deviceProperties, 0, sizeof(deviceProperties));
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

        driverData.name = deviceProperties.deviceName;
        driverData.version = deviceProperties.driverVersion;
        driverData.versionMajor = (deviceProperties.driverVersion >> 22) & 0X3FF;
        driverData.versionMinor = (deviceProperties.driverVersion >> 12) & 0X3FF;
        driverData.versionPatch = deviceProperties.driverVersion & 0X3FF;

        driverData.deviceID = deviceProperties.deviceID;
        driverData.deviceType = deviceProperties.deviceType;
        driverData.apiMajor = (deviceProperties.apiVersion >> 22) & 0X3FF;
        driverData.apiMinor = (deviceProperties.apiVersion >> 12) & 0X3FF;
        driverData.apiPatch = deviceProperties.apiVersion & 0X3FF;
        driverData.deviceCount = deviceCount;

        std::cout << "\nDevice name: " << deviceProperties.deviceName << "\n";
        std::cout << "Device version: " << deviceProperties.driverVersion << "\n";
        std::cout << "Device version: " << driverData.versionMajor << "." << driverData.versionMinor << "." << driverData.versionPatch << "\n";
        std::cout << "Device ID: " << deviceProperties.deviceID << "\n";
        std::cout << "Device type: " << deviceProperties.deviceType << "\n";
        auto apiVersionMajor = (deviceProperties.apiVersion >> 22) & 0X3FF;
        auto apiVersionMinor = (deviceProperties.apiVersion >> 12) & 0X3FF;
        auto apiVersionPatch = deviceProperties.apiVersion & 0X3FF;
        std::cout << "API version: " << apiVersionMajor << "." << apiVersionMinor << "." << apiVersionPatch << "\n";

        // Enumerate and print relevant device limitations.
        VkPhysicalDeviceLimits limits = deviceProperties.limits;
        std::cout << "\nVkPhysicalDeviceLimits:\n";
        std::cout << "- maxComputeSharedMemorySize: " << limits.maxComputeSharedMemorySize << std::endl;
        std::cout << "- maxComputeWorkGroupInvocations: " << limits.maxComputeWorkGroupInvocations << std::endl;
        std::cout << "- maxComputeWorkGroupCount: " << limits.maxComputeWorkGroupCount[0] << " " << limits.maxComputeWorkGroupCount[1] << " " << limits.maxComputeWorkGroupCount[2] << std::endl;
        std::cout << "- maxComputeWorkGroupSize: " << limits.maxComputeWorkGroupSize[0] << " " << limits.maxComputeWorkGroupSize[1] << " " << limits.maxComputeWorkGroupSize[2] << std::endl;
        std::cout << "- maxBoundDescriptorSets: " << limits.maxBoundDescriptorSets << std::endl;
        std::cout << "- maxDescriptorSetStorageBuffers: " << limits.maxDescriptorSetStorageBuffers << std::endl;
        std::cout << "- maxDescriptorSetUniformBuffers: " << limits.maxDescriptorSetUniformBuffers << std::endl;
        std::cout << "- maxStorageBufferRange: " << limits.maxStorageBufferRange << std::endl;
        std::cout << "- maxUniformBufferRange: " << limits.maxUniformBufferRange << std::endl;
        std::cout << "- minStorageBufferOffsetAlignment: " << limits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << "- minUniformBufferOffsetAlignment: " << limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << "- maxPushConstantsSize: " << limits.maxPushConstantsSize << std::endl;
        std::cout << "- maxMemoryAllocationCount: " << limits.maxMemoryAllocationCount << std::endl;
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createLogicalDevice()
{
    float queuePriority = 1.0f;

    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { 
        indices.graphicsAndComputeFamily.value(), 
        indices.presentFamily.value() 
    };

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // TODO: Come back here and see what device features we want.
    //VkPhysicalDeviceLimits - has MANY stats that may be useful to know.

    VkPhysicalDeviceFeatures deviceFeatures = {};
    //deviceFeatures.shaderStorageBufferArrayDynamicIndexing = VK_TRUE;
    deviceFeatures.tessellationShader = VK_TRUE; // Enable Vulkan to be able to link and execute tessellation shaders.
    deviceFeatures.fillModeNonSolid = VK_TRUE; // Enable Vulkan to use VK_POLYGON_MODE_POINT or LINE.

    //deviceFeatures.samplerAnisotropy = VK_TRUE;
    //deviceFeatures.sampleRateShading = VK_TRUE; // Enable sample shading feature for the device.
    //deviceFeatures.geometryShader = VK_TRUE;

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (kEnableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(kValidationLayers.size());
        createInfo.ppEnabledLayerNames = kValidationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    
    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkGetDeviceQueue(m_LogicalDevice, indices.graphicsAndComputeFamily.value(), 0, &graphicsQueue); 
    vkGetDeviceQueue(m_LogicalDevice, indices.graphicsAndComputeFamily.value(), 0, &computeQueue); 
    vkGetDeviceQueue(m_LogicalDevice, indices.presentFamily.value(), 0, &presentQueue);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createGlfwSurface()
{
    if (glfwCreateWindowSurface(m_VkInstance, window, nullptr, &m_SurfaceKHR) != VK_SUCCESS) {
        throw std::runtime_error("failed to create glfw window surface!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createSwapchain()
{
    SwapChainSupportDetails swapChainSupport = checkSwapchainSupport(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_SurfaceKHR;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsAndComputeFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkGetSwapchainImagesKHR(m_LogicalDevice, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_LogicalDevice, swapChain, &imageCount, swapChainImages.data());
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    return VK_SUCCESS;
}

VkResult VulkanApplication::recreateSwapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_LogicalDevice);

    cleanupSwapchain();

    VkResult ret = createSwapchain();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad swapchain.");
        return ret;
    }

    ret = createSwapchainImageViews();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad swapchain image views.");
        return ret; 
    }

    ret = createFrameBuffers(); 
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad frame buffers.");
        return ret; 
    }

    return ret;
}

VkResult VulkanApplication::createSwapchainImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createComputeDescriptorSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].pImmutableSamplers = nullptr;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].pImmutableSamplers = nullptr;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].pImmutableSamplers = nullptr;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &layoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute descriptor set layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createDescriptorSetLayouts()
{
    // Define the type of descriptor/shader resources you will want.

    //
    // Create a descriptor set layout for objects/models.
    //

    VkResult ret = VK_SUCCESS;

    ret = createModelDescriptorSetLayout();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create model descriptor set layout.");
        return ret;
    }

    //
    // Create a descriptor set layout for grass objects as a data buffer.
    //

    ret = createGrassDescriptorSetLayout();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create grass descriptor set layout.");
        return ret;
    }

    return ret;
}

VkResult VulkanApplication::createPipelines()
{
    VkResult ret = VK_SUCCESS;
     
    ret = createMeshPipeline();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create model pipeline.");
        return ret;
    }

    ret = createGrassPipeline();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create grass pipeline.");
        return ret;
    }

    return ret;
}

VkResult VulkanApplication::createGraphicsPipeline()
{
    //// Standard vertex and fragment shader modules.
    //auto vertexShaderCode = readFile("../shaders/basicShader.vert.spv");
    //auto pixelShaderCode = readFile("../shaders/basicShader.frag.spv");
    //VkShaderModule vertShaderModule = createShaderModule(vertexShaderCode);
    //VkShaderModule fragShaderModule = createShaderModule(pixelShaderCode);

    //// Configure how the vertex shader stage executes within the pipeline.
    //VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    //vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    //vertShaderStageInfo.module = vertShaderModule;
    //vertShaderStageInfo.pName = "main";

    //// Configure how the pixel/fragment shader stage executes within the pipeline.
    //VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    //fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    //fragShaderStageInfo.module = fragShaderModule;
    //fragShaderStageInfo.pName = "main";

    //// See more for tessellation: https://docs.vulkan.org/spec/latest/chapters/tessellation.html

    //// It may be worthwhile to create a new pipeline specifically for the grass itself, since it used compute and tessellation but other geometry does not.

    //// Tessellation Shader set up. Tessellation consists of 3 pipeline stages:
    //// 1. tessellation control shader (transforms control points of a patch and produces per-patch data).
    //// 2. fixed-function tessellator (generates multiple primitives corresponding to a tessellation of the patch in parameter space).
    //// 3. tessellation evaluation shader (transforms the vertices of a tessellated patch).
    //
    //// Tessellator is enabled when the pipeline contains both a control and evaluation shader.

    ////// Tessellation shader modules.
    ////auto tessellationControlShaderCode = readFile("../shaders/basicShader.tesc.spv");
    ////auto tessellationEvaluationShaderCode = readFile("../shaders/basicShader.tese.spv");
    ////VkShaderModule tessellationControlShaderModule = createShaderModule(tessellationControlShaderCode);
    ////VkShaderModule tessellationEvaluationShaderModule = createShaderModule(tessellationEvaluationShaderCode);     

    //// Configure how the tessellation control shader stage executes within the pipeline.
    ////VkPipelineShaderStageCreateInfo tessellationControlShaderStageInfo = {}; 
    ////tessellationControlShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ////tessellationControlShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    ////tessellationControlShaderStageInfo.module = tessellationControlShaderModule;
    ////tessellationControlShaderStageInfo.pName = "main";

    //// Configure how the tessellation evaluation shader stage executes within the pipeline.
    ////VkPipelineShaderStageCreateInfo tessellationEvaluationShaderStageInfo = {};
    ////tessellationEvaluationShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ////tessellationEvaluationShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    ////tessellationEvaluationShaderStageInfo.module = tessellationEvaluationShaderModule;
    ////tessellationEvaluationShaderStageInfo.pName = "main";

    //// Configure how the tessellation process performs its subdivisions.
    ////VkPipelineTessellationStateCreateInfo tessellationStateInfo = {};
    ////tessellationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    ////tessellationStateInfo.pNext = nullptr;
    ////tessellationStateInfo.flags = 0; // Must be 0, this is reserved by Vulkan for future use.
    ////tessellationStateInfo.patchControlPoints = 3; // I would imagine this matches the layout(vertices = 3) out; in the control shader.

    //// Link all configured shader stages into one variable, so the pipeline can connect to all of them.
    //VkPipelineShaderStageCreateInfo shaderStages[] = { 
    //    vertShaderStageInfo, 
    //    /*tessellationControlShaderStageInfo, 
    //    tessellationEvaluationShaderStageInfo, */
    //    fragShaderStageInfo
    //};

    //VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();

    //// Configure how vertex data is structured and passed from a vertex buffer into a graphics pipeline stage.
    //VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    //vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //vertexInputInfo.vertexBindingDescriptionCount = 1;
    //vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    //vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescriptions().size()); 
    //vertexInputInfo.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data();

    //// Specify how the vertices that are provided by the vertex shader are then assembled into primitives for rendering.
    //VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    //inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    //inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    //inputAssembly.primitiveRestartEnable = VK_FALSE;

    //// Specify the structures that may change at runtime.
    //std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    //VkPipelineDynamicStateCreateInfo dynamicState = {};
    //dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    //dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    //dynamicState.pDynamicStates = dynamicStates.data();

    //// Configures the viewports and scissors to determine the regions of the framebuffer to render to.
    //VkPipelineViewportStateCreateInfo viewportState{};
    //viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    //viewportState.viewportCount = 1;
    //viewportState.scissorCount = 1;

    //// Configures settings for how primitives are transformed into fragments/pixels.
    //VkPipelineRasterizationStateCreateInfo rasterizer = {};
    //rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //rasterizer.depthClampEnable = VK_FALSE;
    //rasterizer.rasterizerDiscardEnable = VK_FALSE;
    //rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //rasterizer.lineWidth = 1.0f;
    //rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT
    //rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //rasterizer.depthBiasEnable = VK_FALSE;

    //// Configure settings for how multisampling performs, reducing aliasing and jagged edges in the rendered image.
    //VkPipelineMultisampleStateCreateInfo multisampling = {};
    //multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    //multisampling.sampleShadingEnable = VK_FALSE; 
    //multisampling.rasterizationSamples = msaaSamples;
    //multisampling.minSampleShading = 0; // Min fraction for sample shading where closer to 1 is smooth. // 0.2f

    //// Configure depth and stencil testing where depth refers to whether a fragment should be discarded and stencil refers to complex masking.
    //VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    //depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //depthStencil.depthTestEnable = VK_TRUE;
    //depthStencil.depthWriteEnable = VK_TRUE;
    //depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    //depthStencil.depthBoundsTestEnable = VK_FALSE;
    //depthStencil.stencilTestEnable = VK_FALSE;

    //// Enables blending settings for colour attachments, allowing transparency and masking effects.
    //VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    //colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    //colorBlendAttachment.blendEnable = VK_FALSE;

    //// Configure how blending is applied for individual attachments and logic-based colour operations.
    //VkPipelineColorBlendStateCreateInfo colorBlending = {};
    //colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    //colorBlending.logicOpEnable = VK_FALSE;
    //colorBlending.attachmentCount = 1;
    //colorBlending.pAttachments = &colorBlendAttachment;

    //// Configure how Vulkan understands to bind resources to shaders, ensuring they can efficiently access required resources.
    //// Note that if you have more descriptor set layouts (currently only using uniform buffer objects) you would need to reference that here.
    //VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    //pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutInfo.setLayoutCount = 1;
    //pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    //// Create the layout/blueprint for how the graphics pipeline will be created.
    //if (vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create pipeline layout!");
    //    return VK_ERROR_INITIALIZATION_FAILED;
    //}

    //// Encapsulate all aspects of the pipeline layout, enabling a pipeline creation optimised for specific rendering tasks (in this case, regular graphics).
    //VkGraphicsPipelineCreateInfo pipelineInfo{};
    //pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //pipelineInfo.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]); // Get the size of shader stages array.
    //pipelineInfo.pStages = shaderStages;
    //pipelineInfo.pVertexInputState = &vertexInputInfo;
    //pipelineInfo.pInputAssemblyState = &inputAssembly;
    //pipelineInfo.pViewportState = &viewportState;
    //pipelineInfo.pRasterizationState = &rasterizer;
    //pipelineInfo.pMultisampleState = &multisampling;
    //pipelineInfo.pColorBlendState = &colorBlending;
    //pipelineInfo.pDynamicState = &dynamicState;
    ////pipelineInfo.pTessellationState = &tessellationStateInfo;
    //pipelineInfo.layout = pipelineLayout;
    //pipelineInfo.renderPass = renderPass;
    //pipelineInfo.subpass = 0;
    //pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    //if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create graphics pipeline!");
    //    return VK_ERROR_INITIALIZATION_FAILED;
    //}

    //// The pipeline retains a reference to previously created shader modules, so we no longer need local references.

    ///*vkDestroyShaderModule(m_LogicalDevice, tessellationEvaluationShaderModule, nullptr);
    //vkDestroyShaderModule(m_LogicalDevice, tessellationControlShaderModule, nullptr);*/
    //vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
    //vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createComputePipeline() 
{
    auto computeShaderCode = readFile("../shaders/basicCompute.comp.spv");

    VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

    if (vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = computePipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkDestroyShaderModule(m_LogicalDevice, computeShaderModule, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createMeshPipeline()
{
    // Read SPIR-V files.
    auto meshVertexShaderCode = readFile("../shaders/mesh.vert.spv"); 
    auto fragmentShaderCode = readFile("../shaders/basicShader.frag.spv");

    // Load shader modules.
    VkShaderModule meshVertexShaderModule = createShaderModule(meshVertexShaderCode); 
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    // Configure how the vertex shader will execute within the pipeline.
    VkPipelineShaderStageCreateInfo meshVertexShaderStageInfo = {}; 
    meshVertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
    meshVertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; 
    meshVertexShaderStageInfo.module = meshVertexShaderModule; 
    meshVertexShaderStageInfo.pName = "main"; 

    // Configure the same for the fragment shader.
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {}; 
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; 
    fragmentShaderStageInfo.module = fragmentShaderModule; 
    fragmentShaderStageInfo.pName = "main"; 

    // Connect all shader stages for this pipeline.
    VkPipelineShaderStageCreateInfo shaderStages[] = { meshVertexShaderStageInfo, fragmentShaderStageInfo };

    // Configure how vertex data is structured and passed from a vertex buffer into a pipeline stage.
    VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {}; 
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; 
    vertexInputInfo.vertexBindingDescriptionCount = 1; 
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescriptions().size()); 
    vertexInputInfo.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data();

    // Specify how the vertices that are provided by the vertex shader are then assembled into primitives for rendering.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {}; 
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; 
    inputAssembly.primitiveRestartEnable = VK_FALSE; 

    // Specify the structures that may change at runtime.
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data(); 

    // Configures the viewports and scissors to determine the regions of the framebuffer to render to.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Configures settings for how primitives are transformed into fragments/pixels.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Configure settings for how multisampling performs, reducing aliasing and jagged edges in the rendered image.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.minSampleShading = 0;

    // Enables blending settings for colour attachments, allowing transparency and masking effects.
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    // Configure how blending is applied for individual attachments and logic-based colour operations.
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Configure how Vulkan understands to bind resources to shaders, ensuring they can efficiently access required resources.
    // Note that if you have more descriptor set layouts (currently only using uniform buffer objects) you would need to reference that here.
    VkPipelineLayoutCreateInfo modelPipelineLayoutInfo = {};
    modelPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    modelPipelineLayoutInfo.setLayoutCount = 1;
    modelPipelineLayoutInfo.pSetLayouts = &modelDescriptorSetLayout;

    // Create the layout/blueprint for how the graphics pipeline will be created.
    if (vkCreatePipelineLayout(m_LogicalDevice, &modelPipelineLayoutInfo, nullptr, &modelPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Encapsulate all aspects of the pipeline layout, enabling a pipeline creation optimised for specific rendering tasks (in this case, regular graphics).
    VkGraphicsPipelineCreateInfo modelPipelineCreateInfo = {}; 
    modelPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; 
    modelPipelineCreateInfo.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]); // Get the size of shader stages array. 
    modelPipelineCreateInfo.pStages = shaderStages; 
    modelPipelineCreateInfo.pVertexInputState = &vertexInputInfo; 
    modelPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    modelPipelineCreateInfo.pViewportState = &viewportState; 
    modelPipelineCreateInfo.pRasterizationState = &rasterizer; 
    modelPipelineCreateInfo.pMultisampleState = &multisampling; 
    modelPipelineCreateInfo.pColorBlendState = &colorBlending; 
    modelPipelineCreateInfo.pDynamicState = &dynamicState;
    modelPipelineCreateInfo.layout = modelPipelineLayout;
    modelPipelineCreateInfo.renderPass = renderPass; 
    modelPipelineCreateInfo.subpass = 0; 
    modelPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; 

    if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &modelPipelineCreateInfo, nullptr, &modelPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!"); 
        return VK_ERROR_INITIALIZATION_FAILED; 
    }

    // The pipeline retains a reference to previously created shader modules, so we no longer need local references.
    vkDestroyShaderModule(m_LogicalDevice, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, meshVertexShaderModule, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createGrassPipeline()
{
    // Read SPIR-V files.
    auto grassVertexShaderCode = readFile("../shaders/grass.vert.spv");
    auto fragmentShaderCode = readFile("../shaders/basicShader.frag.spv");

    // Load shader modules.
    VkShaderModule grassVertexShaderModule = createShaderModule(grassVertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    // Configure how the vertex shader will execute within the pipeline.
    VkPipelineShaderStageCreateInfo grassVertexShaderStageInfo = {};
    grassVertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    grassVertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    grassVertexShaderStageInfo.module = grassVertexShaderModule;
    grassVertexShaderStageInfo.pName = "main";

    // Configure the same for the fragment shader.
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";

    // Connect all shader stages for this pipeline.
    VkPipelineShaderStageCreateInfo shaderStages[] = { grassVertexShaderStageInfo, fragmentShaderStageInfo };

    // Configure how vertex data is structured and passed from a vertex buffer into a pipeline stage.
    VkVertexInputBindingDescription bladeBindingDescription = Blade::getBindingDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1; 
    vertexInputInfo.pVertexBindingDescriptions = &bladeBindingDescription; 
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Blade::getAttributeDescription().size());
    vertexInputInfo.pVertexAttributeDescriptions = Blade::getAttributeDescription().data();
    
    // Specify how the vertices that are provided by the vertex shader are then assembled into primitives for rendering.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; // WARNING :: PROBABLY NOT HTIS 
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Specify the structures that may change at runtime.
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Configures the viewports and scissors to determine the regions of the framebuffer to render to.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    
    // Configures settings for how primitives are transformed into fragments/pixels.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_POINT; // WARNING :: Points for grass !!!
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; 
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Configure settings for how multisampling performs, reducing aliasing and jagged edges in the rendered image.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.minSampleShading = 0;
    
    // Enables blending settings for colour attachments, allowing transparency and masking effects.
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    // Configure how blending is applied for individual attachments and logic-based colour operations.
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Configure how Vulkan understands to bind resources to shaders, ensuring they can efficiently access required resources.
    // Note that if you have more descriptor set layouts (currently only using uniform buffer objects) you would need to reference that here.
    VkPipelineLayoutCreateInfo grassPipelineLayoutInfo = {};
    grassPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    grassPipelineLayoutInfo.setLayoutCount = 1;
    grassPipelineLayoutInfo.pSetLayouts = &grassDescriptorSetLayout;

    // Create the layout/blueprint for how the graphics pipeline will be created.
    if (vkCreatePipelineLayout(m_LogicalDevice, &grassPipelineLayoutInfo, nullptr, &grassPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Encapsulate all aspects of the pipeline layout, enabling a pipeline creation optimised for specific rendering tasks (in this case, regular graphics).
    VkGraphicsPipelineCreateInfo grassPipelineCreateInfo = {};
    grassPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    grassPipelineCreateInfo.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]); // Get the size of shader stages array. 
    grassPipelineCreateInfo.pStages = shaderStages;
    grassPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    grassPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    grassPipelineCreateInfo.pViewportState = &viewportState;
    grassPipelineCreateInfo.pRasterizationState = &rasterizer;
    grassPipelineCreateInfo.pMultisampleState = &multisampling;
    grassPipelineCreateInfo.pColorBlendState = &colorBlending;
    grassPipelineCreateInfo.pDynamicState = &dynamicState;
    grassPipelineCreateInfo.layout = grassPipelineLayout;
    grassPipelineCreateInfo.renderPass = renderPass;
    grassPipelineCreateInfo.subpass = 0;
    grassPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &grassPipelineCreateInfo, nullptr, &grassPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // The pipeline retains a reference to previously created shader modules, so we no longer need local references.
    vkDestroyShaderModule(m_LogicalDevice, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, grassVertexShaderModule, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createFrameBuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {

        VkImageView attachments[] = { swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_LogicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

    if (vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createShaderStorageBuffers()
{
    VkDeviceSize bufferSize = sizeof(BladeInstanceData) * MAX_BLADES;

    VkResult ret = createBuffer(
        bufferSize, 
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, // warning
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // warning
        bladeInstanceDataBuffer,
        bladeInstanceDataBufferMemory
    );

    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad buffer creation.");
        return ret;
    }

    // if u get a map memory validation error from here, you can remove this i think.
    vkMapMemory(m_LogicalDevice, bladeInstanceDataBufferMemory, 0, bufferSize, 0, &bladeInstanceDataBufferMapped);

    return ret;
}

VkResult VulkanApplication::createVertexBuffer()
{
    // Define a return code for potentially dangerous function calls to ensure they ran correctly.
    VkResult ret = VK_ERROR_INITIALIZATION_FAILED; 

    // for the quad.
    {
        // Calculate the total size of the vertex buffers that we will need.
        VkDeviceSize quadMeshRequiredBufferSize = sizeof(Vertex) * quadMesh.vertexCount;

        // Prepare staging buffer and its associated memory for holding the vertex data temporarily before it gets transferred to the GPU.
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        // Create the staging buffer used as a source to send/transfer buffer data.
        // Note: writes from the CPU are visible to the GPU without explicit flushing.
        ret = createBuffer(quadMeshRequiredBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Convert the staging buffer to a pointer to be accessed easier.
        void* data;
        vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, quadMeshRequiredBufferSize, 0, &data);

        // Copy the vertex data from the quad mesh into the staging buffer.
        memcpy(data, quadMesh.vertices.data(), (size_t)quadMeshRequiredBufferSize);

        // Releases the mapped memory so the GPU can safely access the written data.
        vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

        // Creates the vertex buffer on the GPU used as a destination to receive transfers from a source, and as a vertex buffer for drawing. 
        // Note: this memory is local to the device and not accessible by the host (CPU) directly (optimised for GPU access).
        ret = createBuffer(quadMeshRequiredBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadVertexBuffer, quadVertexBufferMemory);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Transfers the vertex data from the staging buffer to the vertex buffer.
        copyBuffer(stagingBuffer, quadVertexBuffer, quadMeshRequiredBufferSize);

        // Clean up the staging buffer and its associated allocated memory.
        vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);
    }

    return ret;
}

VkResult VulkanApplication::createIndexBuffer()
{
    // Define a return code for potentially dangerous function calls to ensure they ran correctly.
    VkResult ret = VK_ERROR_INITIALIZATION_FAILED;

    // for the quad.
    {
        // Calculate the total size of the index buffers that we will need.
        VkDeviceSize bufferSize = sizeof(uint16_t) * quadMesh.indexCount;

        // Prepare staging buffer and its associated memory for holding the index data temporarily before it gets transferred to the GPU.
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        // Create the staging buffer used as a source to send/transfer buffer data.
        // Note: writes from the CPU are visible to the GPU without explicit flushing.
        ret = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Convert the staging buffer to a pointer to be accessed easier.
        void* data; 
        vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data); 

        // Copy the index data from the sphere mesh into the staging buffer.
        memcpy(data, quadMesh.indices.data(), (size_t)bufferSize);

        // Releases the mapped memory so the GPU can safely access the written data.
        vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

        // Creates the index buffer on the GPU used as a destination to receive transfers from a source, and as a index buffer for drawing. 
        // Note: this memory is local to the device and not accessible by the host (CPU) directly (optimised for GPU access).
        ret = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadIndexBuffer, quadIndexBufferMemory);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Transfers the vertex data from the staging buffer to the vertex buffer.
        copyBuffer(stagingBuffer, quadIndexBuffer, bufferSize);

        // Clean up the staging buffer and its associated allocated memory.
        vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);
    }

    return ret;
}

VkResult VulkanApplication::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(CameraUniformBufferObject);

    VkResult ret = createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad buffer creation.");
        return ret;
    }

    return ret;
}

VkResult VulkanApplication::createDescriptorPool()
{ 
    // Pool sizes containing uniform buffer objects (UBO) and shader storage buffer objects (SSBO).
    // Also include one here for the dynamic storage buffer to be used for an arbitrary number of grass blade objects.

    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 2;

    if (vkCreateDescriptorPool(m_LogicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createDescriptorSets()
{
    VkResult ret = VK_SUCCESS; 

    ret = createModelDescriptorSets();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create model descriptor sets.");
        return ret;
    }

    ret = createGrassDescriptorSets();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create grass descriptor sets.");
        return ret;
    }

    return ret;
}

VkResult VulkanApplication::createCommandBuffer()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) { 
        throw std::runtime_error("failed to allocate command buffers!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

VkResult VulkanApplication::createSynchronizationObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics synchronization objects for a frame!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createImGuiImplementation()
{
    VkDescriptorPoolSize pool_sizes[] = { 
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } 
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkResult ret = vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr, &imguiDescriptorPool);
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad descriptor pool (imgui).");
        return ret;
    }
    
    ImGui::CreateContext();

    //ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true); // Set up the backend to hook ImGui, GLFW, and Vulkan altogether.

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_VkInstance;
    init_info.PhysicalDevice = m_PhysicalDevice;
    init_info.Device = m_LogicalDevice;
    init_info.Queue = graphicsQueue;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = msaaSamples;
    init_info.RenderPass = renderPass;
    init_info.Allocator = nullptr;
    //init_info.UseDynamicRendering = true;

    ImGui_ImplVulkan_Init(&init_info);

    return ret;
}

void VulkanApplication::createMeshObjects()
{
    // Construct a plane mesh, for the ground.
    MeshInstance _groundPlane = quadMesh.generateQuad(glm::vec3(0.0f, 0.0f, 0.0f));
    _groundPlane.position = glm::vec3(0.0f, -0.5f, 0.0f);
    _groundPlane.rotation = glm::vec3(0.0f, 90.0f, 0.0f);
    _groundPlane.scale = glm::vec3(MEADOW_SCALE_X, MEADOW_SCALE_Y, MEADOW_SCALE_Z); 
    groundPlane = _groundPlane;

    driverData.vertexCount += quadMesh.vertexCount;
}

void VulkanApplication::populateBladeInstanceBuffer()
{
    // TODO: Use noise to generate the positions along the plane.
    // TODO: Use noise to modify the terrain for the plane.

    // Based on the bounds of the plane, populate the blade instance container with values to be staged to the GPU later.

    // Prepare the instance buffer.
    localBladeInstanceBuffer.reserve(MAX_BLADES);

    // Calculate the bounds of the flat plane (Y is not needed yet as there is no terrain height).
    // [0, 0, 0] is the origin of the plane, the bounds extend half the scale in each direction.
    // Warning: This does not take into account the position of the ground plane.
    glm::vec2 planeBoundsX = glm::vec2(-(MEADOW_SCALE_X / 2), MEADOW_SCALE_X / 2);
    glm::vec2 planeBoundsZ = glm::vec2(-(MEADOW_SCALE_Z / 2), MEADOW_SCALE_Z / 2);
    
    // Do this outside the loop to avoid continuously creating struct instances, just change the data inside it.
    BladeInstanceData bladeInstanceData = {};

    for (size_t i = 0; i < MAX_BLADES; ++i) {

        // Using pre-calculated bounds and no Y variation, generate a random point on the plane's surface.
        glm::vec3 randomPositionOnPlaneBounds = Utils::getRandomVec3(planeBoundsX, glm::vec2(1.0f, 1.0f), planeBoundsZ, false);

        // Populate this instance of blade data.
        bladeInstanceData.worldPosition = randomPositionOnPlaneBounds; 
        bladeInstanceData.width = GRASS_WIDTH;
        bladeInstanceData.height = GRASS_HEIGHT;
        bladeInstanceData.directionAngle = GRASS_NO_ANGLE;
        bladeInstanceData.stiffness = GRASS_STIFFNESS; 
        bladeInstanceData.lean = GRASS_LEAN;

        // Add this blade to the instance buffer.
        localBladeInstanceBuffer.push_back(bladeInstanceData);
    }
}

void VulkanApplication::createBladeInstanceStagingBuffer()
{
    // copyCPUBladeInstanceBufferToHostVisibleMemory

    // Calculate the required size for the staging buffer.
    VkDeviceSize bladeInstanceBufferRequiredSize = sizeof(BladeInstanceData) * MAX_BLADES;

    // Create the staging buffer used as a source to send/transfer buffer data. Note: writes from the CPU are visible to the GPU without explicit flushing.
    VkResult ret = createBuffer(
        bladeInstanceBufferRequiredSize, 
        /*VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,*/
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        /*VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, */
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        bladeInstanceStagingBuffer,  
        bladeInstanceStagingBufferMemory 
    );

    // Copy data from the staging buffer (host) to the shader storage buffer (GPU)
    copyBuffer(bladeInstanceStagingBuffer, bladeInstanceDataBuffer, bladeInstanceBufferRequiredSize);

    // Convert the staging buffer to a pointer to be accessed easier.
    //void* data;
    //vkMapMemory(m_LogicalDevice, bladeInstanceStagingBufferMemory, 0, bladeInstanceBufferRequiredSize, 0, &data);
    //
    //// Copy the instance data from the local instance buffer into the staging buffer.
    //memcpy(data, localBladeInstanceBuffer.data(), (size_t)bladeInstanceBufferRequiredSize);
    //
    //// Releases the mapped memory so the GPU can safely access the written data.
    //vkUnmapMemory(m_LogicalDevice, bladeInstanceStagingBufferMemory); 

}

void VulkanApplication::prepareImGuiDrawData()
{
    ImGui::Begin("Driver Details", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("Device Count: %i", driverData.deviceCount);
    ImGui::Text("Name: %s", driverData.name.c_str()); 
    ImGui::SameLine(); ImGui::Text("ID: %i", driverData.deviceID);
    ImGui::Text("Driver Version: %i.%i.%i", driverData.versionMajor, driverData.versionMinor, driverData.versionPatch);
    ImGui::Text("Vulkan API Version supported: %i.%i.%i", driverData.apiMajor, driverData.apiMinor, driverData.apiPatch);

    ImGui::Text("Vertex count: %i", driverData.vertexCount);

    ImGui::Text("Frames per second: %f", 1 / (lastFrameTime / 1000));
    ImGui::Text("Delta time: %f", deltaTime);

    ImGui::Text("Frame number: %i", frameCount);

    ImGui::End();
}

VkResult VulkanApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findGPUMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);

    return VK_SUCCESS;
}

void VulkanApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    endSingleTimeCommands(commandBuffer);
}

void VulkanApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    // Copy from the staging buffer into the shader storage buffer, where the data will reside on the GPU for its use in shaders.
    // Copy buffer outside of a render pass (WHY????? just got told to, figure this out)
    copyBuffer(bladeInstanceStagingBuffer, bladeInstanceDataBuffer, sizeof(BladeInstanceData) * MAX_BLADES);

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);    

    //
    // Start model pipeline.
    //

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);     

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Ground plane rendering.
    VkBuffer quadVertexBuffers[] = { quadVertexBuffer };                                        
    VkDeviceSize quadOffsets[] = { 0 };                                                         
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, quadVertexBuffers, quadOffsets);                
    vkCmdBindIndexBuffer(commandBuffer, quadIndexBuffer, 0, VK_INDEX_TYPE_UINT16);              
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipelineLayout, 0, 1, &uniformBufferDescriptorSet, 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, quadMesh.indexCount, 1, 0, 0, 0); 

    //
    // End model pipeline.
    //

    //
    // Start grass pipeline.
    //

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, grassPipeline);

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &bladeInstanceDataBuffer, offsets);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, grassPipelineLayout, 0, 1, &bladeInstanceSSBODescriptorSet, 0, nullptr);

    vkCmdDraw(commandBuffer, 1, MAX_BLADES, 0, 0);

    // Define a region of data to copy the blade instance staging buffer to the shader storage buffer (blade instance data buffer).
    //VkBufferCopy copyRegion = {};
    //copyRegion.srcOffset = 0;
    //copyRegion.dstOffset = 0;
    //copyRegion.size = sizeof(BladeInstanceData) * MAX_BLADES;
    
    // Copy from the staging buffer into the shader storage buffer, where the data will reside on the GPU for its use in shaders.
    //vkCmdCopyBuffer(commandBuffer, bladeInstanceStagingBuffer, bladeInstanceDataBuffer, 1, &copyRegion);

    //
    // End grass pipeline.
    //

    // Start ImGui rendering.
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer); 
    // End ImGui rendering.

    vkCmdEndRenderPass(commandBuffer); 

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanApplication::cleanupApplication(GLFWwindow* window)
{
    cleanupSwapchain();

    vkDestroyBuffer(m_LogicalDevice, uniformBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, uniformBufferMemory, nullptr);

    vkDestroyDescriptorPool(m_LogicalDevice, descriptorPool, nullptr);
    //vkDestroyDescriptorSetLayout(m_LogicalDevice, descriptorSetLayout, nullptr);

    //vkDestroyPipeline(m_LogicalDevice, graphicsPipeline, nullptr);
    //vkDestroyPipelineLayout(m_LogicalDevice, pipelineLayout, nullptr);

    vkDestroyRenderPass(m_LogicalDevice, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_LogicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_LogicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_LogicalDevice, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(m_LogicalDevice, commandPool, nullptr);

    vkDestroyDevice(m_LogicalDevice, nullptr);

    if (kEnableValidationLayers) {
        destroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_VkInstance, m_SurfaceKHR, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void VulkanApplication::cleanupSwapchain()
{
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(m_LogicalDevice, swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(m_LogicalDevice, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(m_LogicalDevice, swapChain, nullptr);
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanApplication::linkWindowToVulkan(GLFWwindow* window)
{
    this->window = window;
    if (this->window == nullptr) {
        throw std::runtime_error("bad window pointer.");
    }
}

uint32_t VulkanApplication::findGPUMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    // If we don't return early, we didn't find a suitable type.
    throw std::runtime_error("failed to find suitable memory type!");
}

VulkanApplication::SwapChainSupportDetails VulkanApplication::checkSwapchainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_SurfaceKHR, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_SurfaceKHR, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_SurfaceKHR, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_SurfaceKHR, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_SurfaceKHR, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void VulkanApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo)
{
    outCreateInfo = {};
    outCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    outCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    outCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    outCreateInfo.pfnUserCallback = debugCallback;
}

VkResult VulkanApplication::createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        return VK_SUCCESS;
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return VK_SUCCESS;
}

void VulkanApplication::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkResult VulkanApplication::createModelDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo uboLayoutInfo = {};
    uboLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboLayoutInfo.bindingCount = 1;
    uboLayoutInfo.pBindings = &uboBinding;

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &uboLayoutInfo, nullptr, &modelDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create model descriptor set layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createGrassDescriptorSetLayout()
{
    // Warning: This is setting the availability for this descriptor type to be used in all shaders that we currently have in use.
    // Defines in which shaders the buffers can be used in.
    VkShaderStageFlags usageStageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // This layout requires a UBO for the camera data to be used here too, so that the grass positions can be represented as points.

    std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings = {};

    // Uniform buffer objects.
    layoutBindings[0] = {};
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;
    
    // Shader storage buffer objects.
    layoutBindings[1] = {};
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // Warning: This may need to be dynamic SSBO down the line.
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutCreateInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &layoutCreateInfo, nullptr, &grassDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create grass descriptor set layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createModelDescriptorSets()
{
    //
    // Create descriptor sets for the model pipeline.
    //

    VkDescriptorSetLayout modelLayout(modelDescriptorSetLayout);

    VkDescriptorSetAllocateInfo modelAllocInfo = {};
    modelAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    modelAllocInfo.descriptorPool = descriptorPool;
    modelAllocInfo.descriptorSetCount = 1;
    modelAllocInfo.pSetLayouts = &modelLayout;

    // Allocate uniform buffer descriptor set memory.
    if (vkAllocateDescriptorSets(m_LogicalDevice, &modelAllocInfo, &uniformBufferDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkDescriptorBufferInfo uboBufferInfo = {};
    uboBufferInfo.buffer = uniformBuffer;
    uboBufferInfo.offset = 0;
    uboBufferInfo.range = sizeof(CameraUniformBufferObject); // Assumes only one CameraUniformBufferObject will be sent.    

    VkWriteDescriptorSet modelDescriptorWrite = {};
    modelDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    modelDescriptorWrite.dstSet = uniformBufferDescriptorSet;
    modelDescriptorWrite.dstBinding = 0;
    modelDescriptorWrite.dstArrayElement = 0;
    modelDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelDescriptorWrite.descriptorCount = 1;
    modelDescriptorWrite.pBufferInfo = &uboBufferInfo;

    vkUpdateDescriptorSets(m_LogicalDevice, 1, &modelDescriptorWrite, 0, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createGrassDescriptorSets()
{
    //
    // Create descriptor sets for the grass pipeline.
    //

    VkDescriptorSetLayout grassLayout(grassDescriptorSetLayout);

    //std::array<VkDescriptorSet, 2> grassDescriptorSets = { bladeInstanceSSBODescriptorSet, bladeInstanceCameraDataUBODescriptorSet };

    VkDescriptorSetAllocateInfo grassAllocInfo = {};
    grassAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    grassAllocInfo.descriptorPool = descriptorPool;
    grassAllocInfo.descriptorSetCount = 1;
    grassAllocInfo.pSetLayouts = &grassLayout;

    // Allocate shader storage buffer descriptor set memory.

    VkResult ret = vkAllocateDescriptorSets(m_LogicalDevice, &grassAllocInfo, &bladeInstanceSSBODescriptorSet);

    if (ret != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
        return ret;
    }   

    std::array<VkWriteDescriptorSet, 2> grassDescriptorWrites = {};

    VkDescriptorBufferInfo uboBufferInfo = {};
    uboBufferInfo.buffer = uniformBuffer;
    uboBufferInfo.offset = 0;
    uboBufferInfo.range = sizeof(CameraUniformBufferObject); // Assumes only one CameraUniformBufferObject will be sent.

    grassDescriptorWrites[0] = {};
    grassDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    grassDescriptorWrites[0].dstSet = bladeInstanceSSBODescriptorSet;
    grassDescriptorWrites[0].dstBinding = 0;
    grassDescriptorWrites[0].dstArrayElement = 0;
    grassDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    grassDescriptorWrites[0].descriptorCount = 1;
    grassDescriptorWrites[0].pBufferInfo = &uboBufferInfo;

    VkDescriptorBufferInfo ssboBufferInfo = {};
    ssboBufferInfo.buffer = bladeInstanceDataBuffer;
    ssboBufferInfo.offset = 0;
    ssboBufferInfo.range = sizeof(BladeInstanceData) * MAX_BLADES;

    grassDescriptorWrites[1] = {};
    grassDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    grassDescriptorWrites[1].dstSet = bladeInstanceSSBODescriptorSet;
    grassDescriptorWrites[1].dstBinding = 1;
    grassDescriptorWrites[1].dstArrayElement = 0;
    grassDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    grassDescriptorWrites[1].descriptorCount = 1;
    grassDescriptorWrites[1].pBufferInfo = &ssboBufferInfo;    

    vkUpdateDescriptorSets(m_LogicalDevice, static_cast<uint32_t>(grassDescriptorWrites.size()), grassDescriptorWrites.data(), 0, nullptr);

    return ret;
}

VkSampleCountFlagBits VulkanApplication::getMaxUsableMSAASampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat VulkanApplication::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkImageView VulkanApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

void VulkanApplication::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void VulkanApplication::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer VulkanApplication::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

void VulkanApplication::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(m_LogicalDevice, commandPool, 1, &commandBuffer);
}

bool VulkanApplication::checkPhysicalDeviceSuitability(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool isSwapchainAdequate = false;
    if (checkPhysicalDeviceExtensionSupport(device)) {
        SwapChainSupportDetails swapChainSupport = checkSwapchainSupport(device);
        isSwapchainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
        && deviceFeatures.tessellationShader
        && indices.isComplete()
        && checkPhysicalDeviceExtensionSupport(device)
        && isSwapchainAdequate;
}

bool VulkanApplication::checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount; 
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount); 
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) { 
        requiredExtensions.erase(extension.extensionName); 
    }
    return requiredExtensions.empty(); 
}

VulkanApplication::QueueFamilyIndices VulkanApplication::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        
        driverData.queueFamilyCount = queueFamilyCount;
        driverData.queueFlags = queueFamily.queueFlags;

        if (indices.isComplete()) {
            break;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_SurfaceKHR, &presentSupport);

        // Graphics and compute family.
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.graphicsAndComputeFamily = i;
        }

        // Present queue family.
        if (presentSupport) {
            indices.presentFamily = i;
        }      

        i++;
    }

    return indices;
}