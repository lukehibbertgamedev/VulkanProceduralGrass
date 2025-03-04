#include "VulkanApplication.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <set>
#include <cstdint> 

#include "Utility.h"

// ===============================================================================================================================================================================

// Validation layers method of outputting notes, warnings, or errors.
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl << std::endl;
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

    return extensions;
}

// Device extensions.
static const std::vector<const char*> kDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

// ===============================================================================================================================================================================

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

VkResult VulkanApplication::initialiseApplication()
{
    VkResult ret = createDefaultCamera();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create camera.");

    ret = createInstance();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create instance.");

    ret = createDebugMessenger();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create debug messenger.");

    ret = createGlfwSurface();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create surface.");

    ret = createPhysicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create physical device.");

    ret = createLogicalDevice();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create logical device.");

    ret = createSwapchain();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create swapchain.");

    ret = createSwapchainImageViews();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create swapchain image views.");

    ret = createRenderPass();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create render pass.");

    ret = createDescriptorSetLayouts();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create descriptor layout.");

    ret = createPipelines();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create model | grass pipeline.");

    ret = createDepthResources();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create depth resources.");

    ret = createFrameBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create frame buffers.");

    ret = createCommandPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create command pool.");

    createMeshObjects();

    populateBladeInstanceBuffer();

    ret = createTextureResources();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create image | sampler resources.");

    ret = createVertexBuffer();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create vertex buffer.");

    ret = createIndexBuffer();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create index buffer.");

    ret = createShaderStorageBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create shader storage buffer.");

    ret = createUniformBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create uniform buffer.");

    ret = createDescriptorPool();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create descriptor pool.");

    createNumBladesBuffer();

    ret = createDescriptorSets();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create descriptor sets.");

    createBladeInstanceStagingBuffer();

    ret = createCommandBuffers();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create command buffer.");

    ret = createSynchronizationObjects();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create semaphores | fences.");

    ret = createImGuiImplementation();
    if (ret != VK_SUCCESS) throw std::runtime_error("Could not create imgui implementation.");

    return ret;
}

void VulkanApplication::render()
{
    // Compute pipeline stage:

    // Re-sync and reset.
    vkWaitForFences(m_LogicalDevice, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_LogicalDevice, 1, &computeInFlightFences[currentFrame]);
    vkResetCommandBuffer(computeCommandBuffers[currentFrame], 0);

    // Record command buffer.
    recordComputeCommandBuffer(computeCommandBuffers[currentFrame]);

    VkSubmitInfo computeSubmitInfo = {};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
    computeSubmitInfo.signalSemaphoreCount = 1;
    computeSubmitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];
    if (vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    }

    // Graphics render stage:

    // Sync between stages.
    vkWaitForFences(m_LogicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX); 

    // Update camera data buffer.
    updateUniformBuffer(currentFrame);

    // Swap to next swapchain image.
    uint32_t imageIndex;
    VkResult ret = vkAcquireNextImageKHR(m_LogicalDevice, swapchainData.handle, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (ret == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (ret != VK_SUCCESS && ret != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Re-sync and reset.
    vkResetFences(m_LogicalDevice, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    std::array<VkSemaphore, 2> waitSemaphores = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
    std::array<VkPipelineStageFlags, 2> waitStages = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo graphicsSubmitInfo = {};
    graphicsSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    graphicsSubmitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    graphicsSubmitInfo.pWaitSemaphores = waitSemaphores.data();
    graphicsSubmitInfo.pWaitDstStageMask = waitStages.data();
    graphicsSubmitInfo.commandBufferCount = 1;
    graphicsSubmitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    graphicsSubmitInfo.signalSemaphoreCount = 1;
    graphicsSubmitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame]; // Signal this so present can wait for this.
    if (vkQueueSubmit(graphicsQueue, 1, &graphicsSubmitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = { swapchainData.handle };
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; 
    ret = vkQueuePresentKHR(presentQueue, &presentInfo); // Send all image data to the screen, this will render this frame and begin the vertex shader.

    // Re-initialise swapchain if necessary.
    if (ret == VK_ERROR_OUT_OF_DATE_KHR || ret == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapchain();
    }
    else if (ret != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // Frame complete, increment frame and wrap if it goes beyond max frames in flight.
    currentFrame = (currentFrame + 1) % kMaxFramesInFlight;
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

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera->getFOV()), 
        swapchainData.extents.width / (float)swapchainData.extents.height, camera->nearPlane, camera->farPlane);
    projectionMatrix[1][1] *= -1; // Invert the y-axis due to differing coordinate systems.

    // Calculate correct rotation matrix for the plane to be flat ground.
    glm::mat4 rotationMatrixX = glm::rotate(glm::mat4(1.0f), glm::radians(groundPlane.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationMatrixY = glm::rotate(glm::mat4(1.0f), glm::radians(groundPlane.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationMatrixZ = glm::rotate(glm::mat4(1.0f), glm::radians(groundPlane.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotationMatrix = rotationMatrixX * rotationMatrixY * rotationMatrixZ;

    // Calculate the model, view, and projection matrix used by the vertex shader.
    CameraUniformBufferObject ubo = {};
    ubo.model = glm::translate(glm::mat4(1.0f), groundPlane.position) * rotationMatrix * glm::scale(glm::mat4(1.0f), groundPlane.scale);    
    ubo.view = camera->getViewMatrix();
    ubo.proj = projectionMatrix;

    // Copy the contents of the ubo structure into a mapped memory location effectively updating the uniform buffer with the most recent data.
    void* data;
    vkMapMemory(m_LogicalDevice, uniformBufferMemory, 0, sizeof(ubo), 0, &data);
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
    appInfo.applicationVersion = VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, 0);
    appInfo.pEngineName = "Custom Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;
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
            break;
        }
    }

    // Initialise driver data to display in ImGui.
    for (int i = 0; i < deviceCount; ++i) {
        VkPhysicalDeviceProperties deviceProperties = {};
        memset(&deviceProperties, 0, sizeof(deviceProperties));
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        driverData.name = deviceProperties.deviceName;
        driverData.version = deviceProperties.driverVersion;
        driverData.versionMajor = (deviceProperties.driverVersion >> 22) & 0X3FF;
        driverData.versionMinor = (deviceProperties.driverVersion >> 12) & 0X3FF;
        driverData.apiMajor = (deviceProperties.apiVersion >> 22) & 0X3FF;
        driverData.apiMinor = (deviceProperties.apiVersion >> 12) & 0X3FF;
        driverData.apiPatch = deviceProperties.apiVersion & 0X3FF;
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

    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice, m_SurfaceKHR);
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

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.fillModeNonSolid = VK_TRUE; // Enable Vulkan to use VK_POLYGON_MODE_POINT or LINE.
    deviceFeatures.samplerAnisotropy = VK_TRUE; // Enable Vulkan to support anisotropic filtering.
    deviceFeatures.tessellationShader = VK_TRUE; // Enable Vulkan to be able to link and execute tessellation shaders.
    deviceFeatures.shaderTessellationAndGeometryPointSize = VK_TRUE; // Enable Vulkan to allow the use of gl_PointSize within tessellation shaders.
    deviceFeatures.multiDrawIndirect = VK_TRUE; // Enable Vulkan to allow the use of indirect draw commands.

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

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
    SwapChain swapChainSupport = checkSwapchainSupport(m_PhysicalDevice);

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

    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice, m_SurfaceKHR);
    uint32_t queueFamilyIndices[] = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsAndComputeFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr; 
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &swapchainData.handle) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkGetSwapchainImagesKHR(m_LogicalDevice, swapchainData.handle, &imageCount, nullptr);
    swapchainData.images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_LogicalDevice, swapchainData.handle, &imageCount, swapchainData.images.data());
    swapchainData.imageFormat = surfaceFormat.format;
    swapchainData.extents = extent;

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

    swapchainData.cleanupSwapchain(m_LogicalDevice);

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

    ret = createDepthResources();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad depth resources.");
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
    swapchainData.imageViews.resize(swapchainData.images.size());

    for (size_t i = 0; i < swapchainData.images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainData.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainData.imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &swapchainData.imageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchainData.imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Early test fragment bit for testing depth before the fragment shader is called.
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; 
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; 
    
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
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

    ret = createComputePipeline();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create compute pipeline.");
        return ret;
    }

    ret = createGrassPipeline();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create grass pipeline.");
        return ret;
    }

    return ret;
}
 
VkResult VulkanApplication::createMeshPipeline()
{
    // Read SPIR-V files.
    auto meshVertexShaderCode = Utils::readFile("../shaders/mesh.vert.spv"); 
    auto terrainTessellationControlShaderCode = Utils::readFile("../shaders/terrainTessControl.tesc.spv");
    auto terrainTessellationEvalShaderCode = Utils::readFile("../shaders/terrainTessEval.tese.spv");
    auto fragmentShaderCode = Utils::readFile("../shaders/basicShader.frag.spv");

    // Load shader modules.
    VkShaderModule meshVertexShaderModule = createShaderModule(meshVertexShaderCode); 
    VkShaderModule tessellationControlShaderModule = createShaderModule(terrainTessellationControlShaderCode);
    VkShaderModule tessellationEvaluationShaderModule = createShaderModule(terrainTessellationEvalShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    // Configure how the vertex shader will execute within the pipeline.
    VkPipelineShaderStageCreateInfo meshVertexShaderStageInfo = {}; 
    meshVertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
    meshVertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; 
    meshVertexShaderStageInfo.module = meshVertexShaderModule; 
    meshVertexShaderStageInfo.pName = "main"; 

    // Configure the same for the tessellation control shader.
    VkPipelineShaderStageCreateInfo tessellationControlShaderStageInfo = {};
    tessellationControlShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessellationControlShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    tessellationControlShaderStageInfo.module = tessellationControlShaderModule;
    tessellationControlShaderStageInfo.pName = "main";

    // Configure the same for the tessellation evaluation shader.
    VkPipelineShaderStageCreateInfo tessellationEvaluationShaderStageInfo = {};
    tessellationEvaluationShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessellationEvaluationShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    tessellationEvaluationShaderStageInfo.module = tessellationEvaluationShaderModule;
    tessellationEvaluationShaderStageInfo.pName = "main";

    // Configure the same for the fragment shader.
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {}; 
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; 
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; 
    fragmentShaderStageInfo.module = fragmentShaderModule; 
    fragmentShaderStageInfo.pName = "main"; 

    // Connect all shader stages for this pipeline.
    VkPipelineShaderStageCreateInfo shaderStages[] = { meshVertexShaderStageInfo, tessellationControlShaderStageInfo, tessellationEvaluationShaderStageInfo, fragmentShaderStageInfo };

    // Configure how vertex data is structured and passed from a vertex buffer into a pipeline stage.
    VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {}; 
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; 
    vertexInputInfo.vertexBindingDescriptionCount = 1; 
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(Vertex::getAttributeDescriptions().size()); 
    vertexInputInfo.pVertexAttributeDescriptions = Vertex::getAttributeDescriptions().data();

    // Specify how the vertices that are provided by the vertex shader are then assembled into primitives for rendering.
    // You want to specifically use a patch list here, so the tessellation primitive generator can generate patches of subdivided meshes.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {}; 
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE; 

    // Configure how the tessellation connects to the pipeline, most importantly defining the patch control points.
    // The most important is the number of patch control points (per-patch vertices), the evaluation shader is working with quads, hence the value of 4.
    VkPipelineTessellationStateCreateInfo tessellationState = {};
    tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState.pNext = nullptr;
    tessellationState.flags = 0;                // Reserved by Vulkan for future use, this must be 0.
    tessellationState.patchControlPoints = 4;

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
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; // Only required for the terrain.
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Configure settings for how multisampling performs, reducing aliasing and jagged edges in the rendered image.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 0;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

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
    modelPipelineCreateInfo.pTessellationState = &tessellationState;
    modelPipelineCreateInfo.pViewportState = &viewportState; 
    modelPipelineCreateInfo.pRasterizationState = &rasterizer; 
    modelPipelineCreateInfo.pMultisampleState = &multisampling; 
    modelPipelineCreateInfo.pDepthStencilState = &depthStencil; 
    modelPipelineCreateInfo.pColorBlendState = &colorBlending; 
    modelPipelineCreateInfo.pDynamicState = &dynamicState;
    modelPipelineCreateInfo.layout = modelPipelineLayout;
    modelPipelineCreateInfo.renderPass = renderPass; 
    modelPipelineCreateInfo.subpass = 0; 
    modelPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; 

    VkResult a = vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &modelPipelineCreateInfo, nullptr, &modelPipeline);

    if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &modelPipelineCreateInfo, nullptr, &modelPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!"); 
        return VK_ERROR_INITIALIZATION_FAILED; 
    }

    // The pipeline retains a reference to previously created shader modules, so we no longer need local references.
    vkDestroyShaderModule(m_LogicalDevice, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, tessellationEvaluationShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, tessellationControlShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, meshVertexShaderModule, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createComputePipeline()
{
    auto grassComputeShaderCode = Utils::readFile("../shaders/grassCompute.comp.spv");
    VkShaderModule grassComputeShaderModule = createShaderModule(grassComputeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = grassComputeShaderModule;
    computeShaderStageInfo.pName = "main";

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstantsObject);

    VkPipelineLayoutCreateInfo computePipelineLayoutInfo = {};
    computePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computePipelineLayoutInfo.pNext = nullptr;
    computePipelineLayoutInfo.flags = 0;
    computePipelineLayoutInfo.pushConstantRangeCount = 1;
    computePipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    computePipelineLayoutInfo.setLayoutCount = 1;
    computePipelineLayoutInfo.pSetLayouts = &grassDescriptorSetLayout;

    // Create the layout/blueprint for how the compute pipeline will be created.
    if (vkCreatePipelineLayout(m_LogicalDevice, &computePipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.flags = 0;
    computePipelineCreateInfo.layout = computePipelineLayout; 
    computePipelineCreateInfo.stage = computeShaderStageInfo;
    computePipelineCreateInfo.basePipelineHandle = 0;
    computePipelineCreateInfo.basePipelineIndex = 0;

    if (vkCreateComputePipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkDestroyShaderModule(m_LogicalDevice, grassComputeShaderModule, nullptr);

    return VK_SUCCESS; 
}

VkResult VulkanApplication::createGrassPipeline()
{
    // Read SPIR-V files.
    auto grassVertexShaderCode = Utils::readFile("../shaders/grass.vert.spv");
    auto fragmentShaderCode = Utils::readFile("../shaders/basicShader.frag.spv");
    auto tessellationControlShaderCode = Utils::readFile("../shaders/grassTessControl.tesc.spv");
    auto tessellationEvalShaderCode = Utils::readFile("../shaders/grassTessEval.tese.spv");

    // Load shader modules.
    VkShaderModule grassVertexShaderModule = createShaderModule(grassVertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);
    VkShaderModule tessControlShaderModule = createShaderModule(tessellationControlShaderCode);
    VkShaderModule tessEvalShaderModule = createShaderModule(tessellationEvalShaderCode);

    // Configure how the vertex shader will execute within the pipeline...
    VkPipelineShaderStageCreateInfo grassVertexShaderStageInfo = {};
    grassVertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    grassVertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    grassVertexShaderStageInfo.module = grassVertexShaderModule;
    grassVertexShaderStageInfo.pName = "main";

    // Configure the same for the fragment shader...
    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";

    // And so on for the tessellation control shader...
    VkPipelineShaderStageCreateInfo tessControlShaderStageInfo = {};
    tessControlShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessControlShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    tessControlShaderStageInfo.module = tessControlShaderModule;
    tessControlShaderStageInfo.pName = "main";

    // And once more for the tessellation evaluation shader...
    VkPipelineShaderStageCreateInfo tessEvalShaderStageInfo = {};
    tessEvalShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tessEvalShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    tessEvalShaderStageInfo.module = tessEvalShaderModule;
    tessEvalShaderStageInfo.pName = "main";

    // Connect all shader stages for this pipeline, in order of execution (ie., vertex first, then control...).
    VkPipelineShaderStageCreateInfo shaderStages[] = { grassVertexShaderStageInfo, tessControlShaderStageInfo, tessEvalShaderStageInfo, fragmentShaderStageInfo }; 

    // Configure how vertex data is structured and passed from a vertex buffer into a pipeline stage.
     
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Note that there are no vertex binding descriptions or attribute descriptions here, this is because the
    // blade instance data is sent via a shader storage buffer object and does not require this data to be bound. 
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0; 
    vertexInputInfo.pVertexBindingDescriptions = nullptr; 
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    
    // Specify how the vertices that are provided by the vertex shader are then assembled into primitives for rendering.
    // You want to specifically use a patch list here, so the tessellation primitive generator can generate patches of subdivided meshes.
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;                             
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Configure how the tessellation connects to the pipeline, most importantly defining the patch control points.
    // The most important is the number of patch control points (per-patch vertices), the evaluation shader is working with quads, hence the value of 4.
    VkPipelineTessellationStateCreateInfo tessellationState = {};
    tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellationState.pNext = nullptr;
    tessellationState.flags = 0;                // Reserved by Vulkan for future use, this must be 0.
    tessellationState.patchControlPoints = 4;

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
    // Rasterisation mode: POINT / LINE / FILL for POINT MODE / WIREFRAME / STANDARD rendering.
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; 
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;                     
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Configure settings for how multisampling performs, reducing aliasing and jagged edges in the rendered image.
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 0;
    
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

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
    VkPipelineLayoutCreateInfo grassPipelineLayoutInfo = {};
    grassPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    grassPipelineLayoutInfo.pushConstantRangeCount = 0;
    grassPipelineLayoutInfo.pPushConstantRanges = nullptr;
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
    grassPipelineCreateInfo.pTessellationState = &tessellationState;
    grassPipelineCreateInfo.pViewportState = &viewportState;
    grassPipelineCreateInfo.pRasterizationState = &rasterizer;
    grassPipelineCreateInfo.pMultisampleState = &multisampling;
    grassPipelineCreateInfo.pDepthStencilState = &depthStencil;
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
    vkDestroyShaderModule(m_LogicalDevice, tessEvalShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, tessControlShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(m_LogicalDevice, grassVertexShaderModule, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createHeightMapImage()
{
    // Read the pixel data for the texture.
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("../assets/RollingHillsHeightMap.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Create a staging buffer to upload pixel data to the GPU.
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    BufferCreateInfo buffer = {};
    buffer.size = imageSize;
    buffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.pBuffer = &stagingBuffer;
    buffer.pBufferMemory = &stagingBufferMemory;

    VkResult ret = createBuffer(buffer);
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad buffer creation.");
        return ret;
    }

    // Upload pixel data to the GPU.
    void* data;
    vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

    // Since the GPU now has a reference to the pixel data, we don't need to store it on the host anymore.
    stbi_image_free(pixels);

    ImageCreateInfo heightMapImageInfo = {};
    heightMapImageInfo.width = texWidth;
    heightMapImageInfo.height = texHeight;
    heightMapImageInfo.mipLevels = 1;
    heightMapImageInfo.numSamples = VK_SAMPLE_COUNT_1_BIT;
    heightMapImageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    heightMapImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    heightMapImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    heightMapImageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    heightMapImageInfo.pImage = &heightMapImage;
    heightMapImageInfo.pImageMemory = &heightMapImageMemory;

    // Create an image handle.
    ret = createImage(heightMapImageInfo);

    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad image creation.");
        return ret;
    }

    // Allow the height map image to be formatted to the same as the texture data that was read in.
    transitionImageLayout(heightMapImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, heightMapImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(heightMapImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);

    return ret;
}

VkResult VulkanApplication::createHeightMapImageView()
{
    heightMapImageView = createImageView(heightMapImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    if (!heightMapImageView) {
        throw std::runtime_error("bad texture image view.");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createHeightMapSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_LogicalDevice, &samplerInfo, nullptr, &heightMapSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createFrameBuffers()
{
    swapchainData.framebuffers.resize(swapchainData.imageViews.size());
    for (size_t i = 0; i < swapchainData.imageViews.size(); i++) {

        std::array<VkImageView, 2> attachments = { swapchainData.imageViews[i], depthImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainData.extents.width;
        framebufferInfo.height = swapchainData.extents.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_LogicalDevice, &framebufferInfo, nullptr, &swapchainData.framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_PhysicalDevice, m_SurfaceKHR);

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

VkResult VulkanApplication::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    ImageCreateInfo depthImageInfo = {};
    depthImageInfo.width = swapchainData.extents.width;
    depthImageInfo.height = swapchainData.extents.height;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.numSamples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.format = depthFormat;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthImageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    depthImageInfo.pImage = &depthImage;
    depthImageInfo.pImageMemory = &depthImageMemory;

    VkResult ret = createImage(depthImageInfo);
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad image creation.");
        return ret;
    }

    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createTextureResources()
{
    VkResult ret = VK_SUCCESS;

    ret = createHeightMapImage();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad height map image creation.");
        return ret;
    }

    ret = createHeightMapImageView();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad height map image view creation.");
        return ret;
    }

    ret = createHeightMapSampler();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad sampler creation.");
        return ret;
    }

    return ret;
}

VkResult VulkanApplication::createShaderStorageBuffers()
{
    VkDeviceSize bufferSize = sizeof(GrassBladeInstanceData) * kMaxBlades;

    VkResult ret = VK_SUCCESS;

    bladeInstanceDataBuffer.resize(kMaxFramesInFlight);
    bladeInstanceDataBufferMemory.resize(kMaxFramesInFlight);

    // Create 2 SSBOs per-framebuffer, this application uses double-buffering.
    for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
        
        BufferCreateInfo buffer = {};
        buffer.size = bufferSize;
        buffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        buffer.memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        buffer.pBuffer = &bladeInstanceDataBuffer[i];
        buffer.pBufferMemory = &bladeInstanceDataBufferMemory[i];

        ret = createBuffer(buffer);

        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation.");
            return ret;
        }
    } 

    return ret;
}

VkResult VulkanApplication::createVertexBuffer()
{
    // Define a return code for potentially dangerous function calls to ensure they ran correctly.
    VkResult ret = VK_ERROR_INITIALIZATION_FAILED; 

    // for the grass blade base shape.
    {
        // Calculate the total size of the vertex buffers that we will need.
        VkDeviceSize bufferSize = sizeof(Vertex) * bladeShapeMesh.vertexCount;

        // Prepare staging buffer and its associated memory for holding the vertex data temporarily before it gets transferred to the GPU.
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        BufferCreateInfo stagingBufferInfo = {};
        stagingBufferInfo.size = bufferSize;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        stagingBufferInfo.pBuffer = &stagingBuffer;
        stagingBufferInfo.pBufferMemory = &stagingBufferMemory;

        // Create the staging buffer used as a source to send/transfer buffer data.
        // Note: writes from the CPU are visible to the GPU without explicit flushing.
        ret = createBuffer(stagingBufferInfo);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Convert the staging buffer to a pointer to be accessed easier.
        void* data;
        vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);

        // Copy the vertex data from the shape mesh into the staging buffer.
        memcpy(data, bladeShapeMesh.vertices.data(), (size_t)bufferSize);

        // Releases the mapped memory so the GPU can safely access the written data.
        vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

        BufferCreateInfo bladeVertexBufferInfo = {};
        bladeVertexBufferInfo.size = bufferSize;
        bladeVertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bladeVertexBufferInfo.memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bladeVertexBufferInfo.pBuffer = &bladeShapeIndexBuffer;
        bladeVertexBufferInfo.pBufferMemory = &bladeShapeIndexBufferMemory;

        // Creates the vertex buffer on the GPU used as a destination to receive transfers from a source, and as a vertex buffer for drawing. 
        // Note: this memory is local to the device and not accessible by the host (CPU) directly (optimised for GPU access).
        ret = createBuffer(bladeVertexBufferInfo);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Transfers the vertex data from the staging buffer to the vertex buffer.
        copyBuffer(*stagingBufferInfo.pBuffer, *bladeVertexBufferInfo.pBuffer, bufferSize);

        // Clean up the staging buffer and its associated allocated memory.
        vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);
    }

    // for the quad.
    {
        // Calculate the total size of the vertex buffers that we will need.
        VkDeviceSize quadMeshRequiredBufferSize = sizeof(Vertex) * quadMesh.vertexCount;

        // Prepare staging buffer and its associated memory for holding the vertex data temporarily before it gets transferred to the GPU.
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        BufferCreateInfo stagingBufferInfo = {};
        stagingBufferInfo.size = quadMeshRequiredBufferSize;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        stagingBufferInfo.pBuffer = &stagingBuffer;
        stagingBufferInfo.pBufferMemory = &stagingBufferMemory;

        // Create the staging buffer used as a source to send/transfer buffer data.
        // Note: writes from the CPU are visible to the GPU without explicit flushing.
        ret = createBuffer(stagingBufferInfo);
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

        BufferCreateInfo quadVertexBufferInfo = {};
        quadVertexBufferInfo.size = quadMeshRequiredBufferSize;
        quadVertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        quadVertexBufferInfo.memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        quadVertexBufferInfo.pBuffer = &quadVertexBuffer;
        quadVertexBufferInfo.pBufferMemory = &quadVertexBufferMemory;

        // Creates the vertex buffer on the GPU used as a destination to receive transfers from a source, and as a vertex buffer for drawing. 
        // Note: this memory is local to the device and not accessible by the host (CPU) directly (optimised for GPU access).
        ret = createBuffer(quadVertexBufferInfo);
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

    // for the grass blade base shape.
    {
        // Calculate the total size of the index buffers that we will need.
        VkDeviceSize bufferSize = sizeof(uint16_t) * bladeShapeMesh.indexCount;

        // Prepare staging buffer and its associated memory for holding the index data temporarily before it gets transferred to the GPU.
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        BufferCreateInfo stagingBufferInfo = {};
        stagingBufferInfo.size = bufferSize;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        stagingBufferInfo.pBuffer = &stagingBuffer;
        stagingBufferInfo.pBufferMemory = &stagingBufferMemory;

        // Create the staging buffer used as a source to send/transfer buffer data.
        // Note: writes from the CPU are visible to the GPU without explicit flushing.
        ret = createBuffer(stagingBufferInfo);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Convert the staging buffer to a pointer to be accessed easier.
        void* data;
        vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);

        // Copy the index data from the mesh into the staging buffer.
        memcpy(data, bladeShapeMesh.indices.data(), (size_t)bufferSize);

        // Releases the mapped memory so the GPU can safely access the written data.
        vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

        BufferCreateInfo bladeIndexBufferInfo = {};
        bladeIndexBufferInfo.size = bufferSize;
        bladeIndexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bladeIndexBufferInfo.memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bladeIndexBufferInfo.pBuffer = &bladeShapeIndexBuffer;
        bladeIndexBufferInfo.pBufferMemory = &bladeShapeIndexBufferMemory;

        // Creates the index buffer on the GPU used as a destination to receive transfers from a source, and as a index buffer for drawing. 
        // Note: this memory is local to the device and not accessible by the host (CPU) directly (optimised for GPU access).
        ret = createBuffer(bladeIndexBufferInfo);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Transfers the vertex data from the staging buffer to the vertex buffer.
        copyBuffer(stagingBuffer, bladeShapeIndexBuffer, bufferSize);

        // Clean up the staging buffer and its associated allocated memory.
        vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);
    }

    // for the quad.
    {
        // Calculate the total size of the index buffers that we will need.
        VkDeviceSize bufferSize = sizeof(uint16_t) * quadMesh.indexCount;

        // Prepare staging buffer and its associated memory for holding the index data temporarily before it gets transferred to the GPU.
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        BufferCreateInfo stagingBufferInfo = {};
        stagingBufferInfo.size = bufferSize;
        stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferInfo.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        stagingBufferInfo.pBuffer = &stagingBuffer;
        stagingBufferInfo.pBufferMemory = &stagingBufferMemory;

        // Create the staging buffer used as a source to send/transfer buffer data.
        // Note: writes from the CPU are visible to the GPU without explicit flushing.
        ret = createBuffer(stagingBufferInfo);
        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation");
            return ret;
        }

        // Convert the staging buffer to a pointer to be accessed easier.
        void* data; 
        vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data); 

        // Copy the index data from the mesh into the staging buffer.
        memcpy(data, quadMesh.indices.data(), (size_t)bufferSize);

        // Releases the mapped memory so the GPU can safely access the written data.
        vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

        BufferCreateInfo quadIndexBufferInfo = {};
        quadIndexBufferInfo.size = bufferSize;
        quadIndexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        quadIndexBufferInfo.memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        quadIndexBufferInfo.pBuffer = &quadIndexBuffer;
        quadIndexBufferInfo.pBufferMemory = &quadIndexBufferMemory;

        // Creates the index buffer on the GPU used as a destination to receive transfers from a source, and as a index buffer for drawing. 
        // Note: this memory is local to the device and not accessible by the host (CPU) directly (optimised for GPU access).
        ret = createBuffer(quadIndexBufferInfo);
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

    BufferCreateInfo buffer = {};
    buffer.size = bufferSize;
    buffer.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    buffer.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.pBuffer = &uniformBuffer;
    buffer.pBufferMemory = &uniformBufferMemory;

    VkResult ret = createBuffer(buffer);

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
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4}
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; 
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes); 
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 5; // Must be at >= the maximum pool size for one type.

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

VkResult VulkanApplication::createCommandBuffers()
{
    VkResult ret = VK_SUCCESS;

    ret = createGraphicsCommandBuffer();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create graphics command buffer.");
        return ret;
    }

    ret = createComputeCommandBuffer();
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("could not create compute command buffer.");
        return ret;
    }

    return ret;
}

VkResult VulkanApplication::createSynchronizationObjects()
{
    // Graphics synchronisation.
    imageAvailableSemaphores.resize(kMaxFramesInFlight);
    renderFinishedSemaphores.resize(kMaxFramesInFlight);
    inFlightFences.resize(kMaxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < kMaxFramesInFlight; i++) {
        if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics synchronization objects for a frame!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    // Compute synchronisation.
    computeInFlightFences.resize(kMaxFramesInFlight);
    computeFinishedSemaphores.resize(kMaxFramesInFlight);

    for (size_t i = 0; i < kMaxFramesInFlight; i++) {
        if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &computeInFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute synchronization objects for a frame!");
            return VK_ERROR_INITIALIZATION_FAILED;
        }
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createDefaultCamera()
{
    VkResult ret = VK_SUCCESS;

    Defaults defaults;

    camera->position = defaults.position;
    camera->pitch = defaults.pitch;
    camera->yaw = defaults.yaw;

    return ret;
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

    ImGui_ImplGlfw_InitForVulkan(window, true); // Set up the backend to hook ImGui, GLFW, and Vulkan altogether.

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_VkInstance;
    init_info.PhysicalDevice = m_PhysicalDevice;
    init_info.Device = m_LogicalDevice;
    init_info.Queue = graphicsQueue;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.RenderPass = renderPass;
    init_info.Allocator = nullptr;

    ImGui_ImplVulkan_Init(&init_info);

    return ret;
}

void VulkanApplication::createMeshObjects()
{
    // Construct a plane mesh, for the ground.
    MeshTransform _groundPlane = quadMesh.generateQuad(glm::vec3(0.0f, 0.0f, 0.0f));
    _groundPlane.position = glm::vec3(0.0f, 0.0f, 0.0f); // X is right. Y is forward. Z is up.
    _groundPlane.rotation = glm::vec3(0.0f, 0.0f, 45.0f);
    _groundPlane.scale = glm::vec3(MEADOW_SCALE_X, MEADOW_SCALE_Y, MEADOW_SCALE_Z); 
    groundPlane = _groundPlane;
}

void VulkanApplication::populateBladeInstanceBuffer()
{
    // Based on the bounds of the plane, populate the blade instance container with values to be staged to the GPU later.

    // Prepare the instance buffer.
    localBladeInstanceBuffer.reserve(kMaxBlades);

    // Calculate the bounds of the flat plane (Z is not needed yet as there is no terrain height on the host, this is done in tessellation).
    glm::vec2 offset = glm::vec2(-150.0f, 20.0f);        // Scale 120| -150, 20
    groundPlane.position.x += MEADOW_SCALE_X + offset.x; // -30.0f x | ranges [-30, +140]
    groundPlane.position.y += MEADOW_SCALE_Y + offset.y; // 140.0f y | ranges [-30, +140]

    const float zFightingEpsilon = 0.01f; // Small value to avoid the grass being clipped into the ground and causing z-fighting.

    // Do this outside the loop to avoid continuously creating struct instances, just change the data inside it.
    GrassBladeInstanceData bladeInstanceData = {};

    for (size_t i = 0; i < kMaxBlades; ++i) {

        // Using pre-calculated bounds and no Z variation, generate a random point on the plane's surface. 
        glm::vec3 randomPositionOnPlaneBounds = {};
        randomPositionOnPlaneBounds.x = Utils::getRandomFloat(groundPlane.position.x, groundPlane.position.y);
        randomPositionOnPlaneBounds.y = Utils::getRandomFloat(groundPlane.position.x, groundPlane.position.y); 
        randomPositionOnPlaneBounds.z = zFightingEpsilon;

        // Create an instance of a grass blade, and define its' natural world position.
        GrassBlade bladeInstance = GrassBlade();
        bladeInstance.p0AndWidth = glm::vec4(randomPositionOnPlaneBounds, 0.0f);
        bladeInstance.updatePackedData();

        // Populate this instance of blade data.
        bladeInstanceData.p0_width      = bladeInstance.p0AndWidth;
        bladeInstanceData.p1_height     = bladeInstance.p1AndHeight;
        bladeInstanceData.p2_direction  = bladeInstance.p2AndDirection;
        bladeInstanceData.up_stiffness  = bladeInstance.upAndStiffness;

        // Add this blade to the instance buffer.
        localBladeInstanceBuffer.push_back(bladeInstanceData);

        // Create a base mesh instance for a grass blade, to later be tessellated and aligned to its' bezier curve.
        MeshTransform baseBladeGeometry = bladeShapeMesh.generateShape();
        baseBladeGeometry.position = glm::vec3(bladeInstance.p0AndWidth.x, bladeInstance.p0AndWidth.y, bladeInstance.p0AndWidth.z); 
        baseBladeGeometry.scale = glm::vec3(1.0f); 
    }
}

void VulkanApplication::createBladeInstanceStagingBuffer()
{
    // Calculate the required size for the staging buffer.
    VkDeviceSize bladeInstanceBufferRequiredSize = sizeof(GrassBladeInstanceData) * kMaxBlades;

    bladeInstanceStagingBuffer.resize(kMaxFramesInFlight);
    bladeInstanceStagingBufferMemory.resize(kMaxFramesInFlight);

    // Create 2 staging buffers per-SSBO, this application uses double-buffering.
    for (size_t i = 0; i < kMaxFramesInFlight; ++i) {

        BufferCreateInfo buffer = {};
        buffer.size = bladeInstanceBufferRequiredSize;
        buffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        buffer.pBuffer = &bladeInstanceStagingBuffer[i];
        buffer.pBufferMemory = &bladeInstanceStagingBufferMemory[i];

        VkResult ret = createBuffer(buffer);

        if (ret != VK_SUCCESS) {
            throw std::runtime_error("bad buffer creation.");
        }

        // To upload data to the GPU, you first need to map and copy the CPU local data to a staging buffer,
        // then make sure to copy the staging buffer data over to the shader resource buffer using a single time command.

        // Map the memory and copy the data from the local vector into the staging buffer.
        void* data;
        vkMapMemory(m_LogicalDevice, bladeInstanceStagingBufferMemory[i], 0, bladeInstanceBufferRequiredSize, 0, &data);
        memcpy(data, localBladeInstanceBuffer.data(), (size_t)bladeInstanceBufferRequiredSize);
        vkUnmapMemory(m_LogicalDevice, bladeInstanceStagingBufferMemory[i]);

        // Copy data from the staging buffer (host) to the shader storage buffer (GPU).
        copyBuffer(bladeInstanceStagingBuffer[i], bladeInstanceDataBuffer[i], bladeInstanceBufferRequiredSize);
    }    
}

void VulkanApplication::createNumBladesBuffer()
{
    VkDeviceSize numBladesBufferRequiredSize = sizeof(NumBladesBufferObject);

    BufferCreateInfo buffer = {};
    buffer.size = numBladesBufferRequiredSize;
    buffer.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer.memProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    buffer.pBuffer = &numBladesBuffer;
    buffer.pBufferMemory = &numBladesBufferMemory;

    VkResult ret = createBuffer(buffer);

    if (ret != VK_SUCCESS) {
        throw std::runtime_error("bad buffer creation");
    }
}

void VulkanApplication::prepareImGuiDrawData()
{
    ImGui::Begin("Driver Details", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("Vulkan procedural grass renderer.");
    ImGui::Text("Developed by Luke Jason Hibbert.");
    
    ImGui::Separator();

    ImGui::Text("Graphics Processing Unit: %s", driverData.name.c_str()); 
    ImGui::Text("Version: %i.%i", driverData.versionMajor, driverData.versionMinor);
    ImGui::Text("Vulkan API Version supported: %i.%i.%i", driverData.apiMajor, driverData.apiMinor, driverData.apiPatch);
    ImGui::Text("Frames per second: %f", 1 / (lastFrameTime / 1000));
    ImGui::Text("Delta time: %f", deltaTime);
    ImGui::Text("Frame number: %i", frameCount);

    ImGui::Separator();

    ImGui::Text("Max grass blade count: %u", kMaxBlades);
    ImGui::Text("Num grass blades culled: %u", kMaxBlades - driverData.numVisible); 

    ImGui::Separator();

    ImGui::Text("Grass blades: %u/%u", driverData.numVisible, kMaxBlades);

    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.0f), "WASD: Move Camera");
    ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.0f), "Arrows: Rotate Camera");
    ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.0f), "R: Reset Camera Position");
    ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.0f), "L/J: Change FOV");
    ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.0f), "Camera position: x: %i, y: %i, z: %i", (int)camera->position.x, (int)camera->position.y, (int)camera->position.z);
    ImGui::TextColored(ImVec4(0.2f, 0.5f, 0.7f, 1.0f), "Pitch: %i / Yaw: %i / FOV: %i", (int)camera->pitch, (int)camera->yaw, (int)camera->fov);
    
    ImGui::Separator();

    ImGui::End();
}

VkResult VulkanApplication::createBuffer(BufferCreateInfo& bufferCreateInfo)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferCreateInfo.size;
    bufferInfo.usage = bufferCreateInfo.usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, bufferCreateInfo.pBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_LogicalDevice, *bufferCreateInfo.pBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findGPUMemoryType(memRequirements.memoryTypeBits, bufferCreateInfo.memProperties);
    
    if (vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, bufferCreateInfo.pBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkBindBufferMemory(m_LogicalDevice, *bufferCreateInfo.pBuffer, *bufferCreateInfo.pBufferMemory, 0);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createImage(ImageCreateInfo& imageCreateInfo)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = imageCreateInfo.width;
    imageInfo.extent.height = imageCreateInfo.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = imageCreateInfo.mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageCreateInfo.format;
    imageInfo.tiling = imageCreateInfo.tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = imageCreateInfo.usage;
    imageInfo.samples = imageCreateInfo.numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_LogicalDevice, &imageInfo, nullptr, imageCreateInfo.pImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_LogicalDevice, *imageCreateInfo.pImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findGPUMemoryType(memRequirements.memoryTypeBits, imageCreateInfo.properties);

    if (vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, imageCreateInfo.pImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    vkBindImageMemory(m_LogicalDevice, *imageCreateInfo.pImage, *imageCreateInfo.pImageMemory, 0);

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

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0] = {};
    clearValues[0].color = { { 0.05f, 0.3f, 0.9f, 1.0f} };
    clearValues[1] = {};
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainData.framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapchainData.extents;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);    

    //
    // Start model pipeline.
    //

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipeline);     

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainData.extents.width);
    viewport.height = static_cast<float>(swapchainData.extents.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchainData.extents;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Ground plane rendering.
    VkBuffer quadVertexBuffers[] = { quadVertexBuffer };                                        
    VkDeviceSize quadOffsets[] = { 0 };                                                         
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, quadVertexBuffers, quadOffsets);                
    vkCmdBindIndexBuffer(commandBuffer, quadIndexBuffer, 0, VK_INDEX_TYPE_UINT16);    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, modelPipelineLayout, 0, 1, &modelPipelineDescriptorSet, 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, quadMesh.indexCount, 1, 0, 0, 0); 

    //
    // End model pipeline.
    //

    //
    // Start grass pipeline.
    //

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, grassPipeline);

    VkDeviceSize quadOffsetsGRASS[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &bladeInstanceDataBuffer[currentFrame], quadOffsetsGRASS);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, grassPipelineLayout, 0, 1, &grassPipelineDescriptorSet, 0, nullptr);

    uint32_t numVisibleBlades = retrieveNumVisibleBlades();

    // The tessellation primitive generator expects to be generating quads, hence the value of 4.
    vkCmdDraw(commandBuffer, 4, numVisibleBlades, 0, 0);

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

void VulkanApplication::recordComputeCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &grassPipelineDescriptorSet, 0, nullptr);

    PushConstantsObject pushConstantsObject = {};
    pushConstantsObject.totalNumBlades = kMaxBlades;
    pushConstantsObject.elapsed = glfwGetTime();

    vkCmdPushConstants(commandBuffer, computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstantsObject), &pushConstantsObject);

    // This should run ONCE PER-BLADE. Dividing the work-group by 32 to match the ideal size of a warp on most hardware, then working with
    // 32 threads per-thread group on the local size in the shader. These values are multiplied so it makes the MAX count anyway.
    vkCmdDispatch(commandBuffer, ((kMaxBlades - 1) / 32u) + 1, 1, 1); // Currently only a 1 dimensional array of thread groups and work groups.

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to end recording compute command buffer!");
    }
}

void VulkanApplication::cleanupApplication(GLFWwindow* window)
{
    // Synchronisation Objects. 
    for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
        vkDestroyFence(m_LogicalDevice, computeInFlightFences[i], nullptr);
        vkDestroySemaphore(m_LogicalDevice, computeFinishedSemaphores[i], nullptr);

        vkDestroyFence(m_LogicalDevice, inFlightFences[i], nullptr);
        vkDestroySemaphore(m_LogicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_LogicalDevice, imageAvailableSemaphores[i], nullptr);
    }

    // Command Buffer - uses command pool, so destroy pool later.
    vkFreeCommandBuffers(m_LogicalDevice, commandPool, static_cast<uint32_t>(computeCommandBuffers.size()), computeCommandBuffers.data());
    vkFreeCommandBuffers(m_LogicalDevice, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    // Blade instance staging buffer.
    for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
        vkDestroyBuffer(m_LogicalDevice, bladeInstanceStagingBuffer[i], nullptr);
        vkFreeMemory(m_LogicalDevice, bladeInstanceStagingBufferMemory[i], nullptr);
    }

    // Descriptor Sets - uses descriptor pool, so destroy pool later.
    std::array<VkDescriptorSet, 2> descriptorSets = { grassPipelineDescriptorSet, modelPipelineDescriptorSet }; 
    vkFreeDescriptorSets(m_LogicalDevice, descriptorPool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());

    // Num blades buffer.
    vkDestroyBuffer(m_LogicalDevice, numBladesBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, numBladesBufferMemory, nullptr);

    // Descriptor Pool.
    vkDestroyDescriptorPool(m_LogicalDevice, imguiDescriptorPool, nullptr);
    vkDestroyDescriptorPool(m_LogicalDevice, descriptorPool, nullptr); 

    // Uniform Buffer Object.
    vkDestroyBuffer(m_LogicalDevice, uniformBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, uniformBufferMemory, nullptr);

    // Shader Storage Buffer Object.
    for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
        vkDestroyBuffer(m_LogicalDevice, bladeInstanceDataBuffer[i], nullptr);
        vkFreeMemory(m_LogicalDevice, bladeInstanceDataBufferMemory[i], nullptr);
    }

    // Index Buffers.
    vkDestroyBuffer(m_LogicalDevice, bladeShapeIndexBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, bladeShapeIndexBufferMemory, nullptr);
    vkDestroyBuffer(m_LogicalDevice, quadIndexBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, quadIndexBufferMemory, nullptr);

    // Vertex Buffers.
    vkDestroyBuffer(m_LogicalDevice, bladeShapeVertexBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, bladeShapeVertexBufferMemory, nullptr);
    vkDestroyBuffer(m_LogicalDevice, quadVertexBuffer, nullptr);
    vkFreeMemory(m_LogicalDevice, quadVertexBufferMemory, nullptr);

    // Texture resources.
    vkDestroySampler(m_LogicalDevice, heightMapSampler, nullptr);
    vkDestroyImageView(m_LogicalDevice, heightMapImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, heightMapImage, nullptr);
    vkFreeMemory(m_LogicalDevice, heightMapImageMemory, nullptr);

    // Command Pool.
    vkDestroyCommandPool(m_LogicalDevice, commandPool, nullptr);

    for (size_t i = 0; i < swapchainData.framebuffers.size(); i++) {
        vkDestroyFramebuffer(m_LogicalDevice, swapchainData.framebuffers[i], nullptr);
    }

    // Depth Resources.
    vkDestroyImageView(m_LogicalDevice, depthImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, depthImage, nullptr);
    vkFreeMemory(m_LogicalDevice, depthImageMemory, nullptr);
    
    // Pipelines.
    vkDestroyPipelineLayout(m_LogicalDevice, grassPipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_LogicalDevice, computePipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_LogicalDevice, modelPipelineLayout, nullptr);
    vkDestroyPipeline(m_LogicalDevice, grassPipeline, nullptr);
    vkDestroyPipeline(m_LogicalDevice, computePipeline, nullptr);
    vkDestroyPipeline(m_LogicalDevice, modelPipeline, nullptr);

    // Descriptor Set Layouts.
    vkDestroyDescriptorSetLayout(m_LogicalDevice, grassDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_LogicalDevice, modelDescriptorSetLayout, nullptr);

    // Render Pass.
    vkDestroyRenderPass(m_LogicalDevice, renderPass, nullptr);

    // Swapchain Images
    for (size_t i = 0; i < swapchainData.imageViews.size(); i++) {
        vkDestroyImageView(m_LogicalDevice, swapchainData.imageViews[i], nullptr);
    }

    // Swapchain.
    vkDestroySwapchainKHR(m_LogicalDevice, swapchainData.handle, nullptr);

    // Logical Device.
    vkDestroyDevice(m_LogicalDevice, nullptr);

    // Window Surface.
    vkDestroySurfaceKHR(m_VkInstance, m_SurfaceKHR, nullptr);

    // Debug Messenger.
    if (kEnableValidationLayers) {
        destroyDebugUtilsMessengerEXT(m_VkInstance, m_DebugMessenger, nullptr);
    }

    // Instance.
    vkDestroyInstance(m_VkInstance, nullptr);

    // Window.
    glfwDestroyWindow(window);
    glfwTerminate();
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

uint32_t VulkanApplication::retrieveNumVisibleBlades()
{
    vkDeviceWaitIdle(m_LogicalDevice);

    void* data;
    vkMapMemory(m_LogicalDevice, numBladesBufferMemory, 0, sizeof(NumBladesBufferObject), 0, &data);

    uint32_t numVisible = *static_cast<uint32_t*>(data);
    driverData.numVisible = numVisible;

    vkUnmapMemory(m_LogicalDevice, numBladesBufferMemory);

    return numVisible;
}

void VulkanApplication::linkWindowToVulkan(GLFWwindow* window)
{
    this->window = window;
    if (this->window == nullptr) {
        throw std::runtime_error("bad window pointer.");
    }
}

void VulkanApplication::linkCameraToVulkan(Camera* camera)
{
    this->camera = camera;
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

SwapChain VulkanApplication::checkSwapchainSupport(VkPhysicalDevice device)
{
    SwapChain details;
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

VkResult VulkanApplication::createGraphicsCommandBuffer()
{
    commandBuffers.resize(kMaxFramesInFlight);

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

VkResult VulkanApplication::createComputeCommandBuffer()
{
    computeCommandBuffers.resize(kMaxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)computeCommandBuffers.size();

    if (vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate compute command buffers!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return VK_SUCCESS;
}

VkResult VulkanApplication::createModelDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    uboBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerBinding = {};
    samplerBinding.binding = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    samplerBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboBinding, samplerBinding };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_LogicalDevice, &layoutInfo, nullptr, &modelDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create model descriptor set layout!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return VK_SUCCESS;
}

VkResult VulkanApplication::createGrassDescriptorSetLayout()
{
    // This layout requires a UBO for the camera data to be used here too, so that the grass positions can be represented as points.
    std::array<VkDescriptorSetLayoutBinding, 5> layoutBindings = {};

    // Uniform buffer objects.
    layoutBindings[0] = {};
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;
    
    // Shader storage buffer object last frame.
    layoutBindings[1] = {};
    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; 
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT; 
    layoutBindings[1].pImmutableSamplers = nullptr;

    // Shader storage buffer object current frame
    layoutBindings[2] = {};
    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; 
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[2].pImmutableSamplers = nullptr;

    // Shader buffer object for returning the number of grass blades for the draw calls.
    layoutBindings[3] = {};
    layoutBindings[3].binding = 3;
    layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[3].descriptorCount = 1;
    layoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[3].pImmutableSamplers = nullptr;

    // Height map for the grass to have its height aligned to the terrain.
    layoutBindings[4] = {};
    layoutBindings[4].binding = 4;
    layoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBindings[4].descriptorCount = 1;
    layoutBindings[4].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[4].pImmutableSamplers = nullptr;

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
    if (vkAllocateDescriptorSets(m_LogicalDevice, &modelAllocInfo, &modelPipelineDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    std::array<VkWriteDescriptorSet, 2> modelDescriptorWrites = {};

    VkDescriptorBufferInfo uboBufferInfo = {};
    uboBufferInfo.buffer = uniformBuffer;
    uboBufferInfo.offset = 0;
    uboBufferInfo.range = sizeof(CameraUniformBufferObject); // Assumes only one CameraUniformBufferObject will be sent.    

    modelDescriptorWrites[0] = {};
    modelDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    modelDescriptorWrites[0].pNext = nullptr;
    modelDescriptorWrites[0].dstSet = modelPipelineDescriptorSet;
    modelDescriptorWrites[0].dstBinding = 0;
    modelDescriptorWrites[0].dstArrayElement = 0;
    modelDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelDescriptorWrites[0].descriptorCount = 1;
    modelDescriptorWrites[0].pImageInfo = nullptr;
    modelDescriptorWrites[0].pBufferInfo = &uboBufferInfo;
    modelDescriptorWrites[0].pTexelBufferView = nullptr;

    VkDescriptorImageInfo heightMapImageInfo = {};
    heightMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    heightMapImageInfo.imageView = heightMapImageView;
    heightMapImageInfo.sampler = heightMapSampler;

    modelDescriptorWrites[1] = {};
    modelDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    modelDescriptorWrites[1].pNext = nullptr;
    modelDescriptorWrites[1].dstSet = modelPipelineDescriptorSet;
    modelDescriptorWrites[1].dstBinding = 1;
    modelDescriptorWrites[1].dstArrayElement = 0;
    modelDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    modelDescriptorWrites[1].descriptorCount = 1;
    modelDescriptorWrites[1].pImageInfo = &heightMapImageInfo;
    modelDescriptorWrites[1].pBufferInfo = nullptr;
    modelDescriptorWrites[1].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_LogicalDevice, static_cast<uint32_t>(modelDescriptorWrites.size()), modelDescriptorWrites.data(), 0, nullptr);

    return VK_SUCCESS;
}

VkResult VulkanApplication::createGrassDescriptorSets()
{
    //
    // Create descriptor sets for the grass pipeline.
    //

    VkDescriptorSetLayout grassLayout(grassDescriptorSetLayout);

    VkDescriptorSetAllocateInfo grassAllocInfo = {};
    grassAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    grassAllocInfo.descriptorPool = descriptorPool;
    grassAllocInfo.descriptorSetCount = 1;
    grassAllocInfo.pSetLayouts = &grassLayout;

    // Allocate shader storage buffer descriptor set memory.
    VkResult ret = vkAllocateDescriptorSets(m_LogicalDevice, &grassAllocInfo, &grassPipelineDescriptorSet);
    if (ret != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
        return ret;
    }   

    std::array<VkWriteDescriptorSet, 5> grassDescriptorWrites = {};

    VkDescriptorBufferInfo uboBufferInfo = {};
    uboBufferInfo.buffer = uniformBuffer;
    uboBufferInfo.offset = 0;
    uboBufferInfo.range = sizeof(CameraUniformBufferObject); // Assumes only one CameraUniformBufferObject will be sent.

    grassDescriptorWrites[0] = {};
    grassDescriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    grassDescriptorWrites[0].dstSet = grassPipelineDescriptorSet;
    grassDescriptorWrites[0].dstBinding = 0;
    grassDescriptorWrites[0].dstArrayElement = 0;
    grassDescriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    grassDescriptorWrites[0].descriptorCount = 1;
    grassDescriptorWrites[0].pBufferInfo = &uboBufferInfo;

    VkDescriptorBufferInfo ssboBufferInfoLastFrame = {};
    ssboBufferInfoLastFrame.buffer = bladeInstanceDataBuffer[(currentFrame - 1) % kMaxFramesInFlight]; // Return the index for the previous frame, % to ensure correct wrapping.  
    ssboBufferInfoLastFrame.offset = 0;  
    ssboBufferInfoLastFrame.range = sizeof(GrassBladeInstanceData) * kMaxBlades; 

    grassDescriptorWrites[1] = {};
    grassDescriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    grassDescriptorWrites[1].dstSet = grassPipelineDescriptorSet;
    grassDescriptorWrites[1].dstBinding = 1;
    grassDescriptorWrites[1].dstArrayElement = 0;
    grassDescriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    grassDescriptorWrites[1].descriptorCount = 1;
    grassDescriptorWrites[1].pBufferInfo = &ssboBufferInfoLastFrame;

    VkDescriptorBufferInfo ssboBufferInfoCurrentFrame = {};
    ssboBufferInfoCurrentFrame.buffer = bladeInstanceDataBuffer[(currentFrame) % kMaxFramesInFlight]; // Return the index for the current frame, % to ensure correct wrapping. 
    ssboBufferInfoCurrentFrame.offset = 0; 
    ssboBufferInfoCurrentFrame.range = sizeof(GrassBladeInstanceData) * kMaxBlades; 

    grassDescriptorWrites[2] = {};
    grassDescriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    grassDescriptorWrites[2].dstSet = grassPipelineDescriptorSet;
    grassDescriptorWrites[2].dstBinding = 2;
    grassDescriptorWrites[2].dstArrayElement = 0;
    grassDescriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    grassDescriptorWrites[2].descriptorCount = 1;
    grassDescriptorWrites[2].pBufferInfo = &ssboBufferInfoCurrentFrame;

    VkDescriptorBufferInfo sboNumBladesBufferInfo = {};
    sboNumBladesBufferInfo.buffer = numBladesBuffer;
    sboNumBladesBufferInfo.offset = 0;
    sboNumBladesBufferInfo.range = sizeof(NumBladesBufferObject);

    grassDescriptorWrites[3] = {};
    grassDescriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    grassDescriptorWrites[3].dstSet = grassPipelineDescriptorSet;
    grassDescriptorWrites[3].dstBinding = 3;
    grassDescriptorWrites[3].dstArrayElement = 0;
    grassDescriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    grassDescriptorWrites[3].descriptorCount = 1;
    grassDescriptorWrites[3].pBufferInfo = &sboNumBladesBufferInfo;

    VkDescriptorImageInfo heightMapImageInfo = {};
    heightMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    heightMapImageInfo.imageView = heightMapImageView;
    heightMapImageInfo.sampler = heightMapSampler;

    grassDescriptorWrites[4] = {};
    grassDescriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    grassDescriptorWrites[4].pNext = nullptr;
    grassDescriptorWrites[4].dstSet = grassPipelineDescriptorSet;
    grassDescriptorWrites[4].dstBinding = 4;
    grassDescriptorWrites[4].dstArrayElement = 0;
    grassDescriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    grassDescriptorWrites[4].descriptorCount = 1;
    grassDescriptorWrites[4].pImageInfo = &heightMapImageInfo;
    grassDescriptorWrites[4].pBufferInfo = nullptr;
    grassDescriptorWrites[4].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(m_LogicalDevice, static_cast<uint32_t>(grassDescriptorWrites.size()), grassDescriptorWrites.data(), 0, nullptr);

    return ret;
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

VkFormat VulkanApplication::findDepthFormat() 
{
    return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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
    
    VkCommandBufferBeginInfo beginInfo = {};
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
    QueueFamilyIndices indices = findQueueFamilies(device, m_SurfaceKHR);

    bool isSwapchainAdequate = false;
    if (checkPhysicalDeviceExtensionSupport(device)) {
        SwapChain swapChainSupport = checkSwapchainSupport(device);
        isSwapchainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
        && deviceFeatures.tessellationShader
        && deviceFeatures.shaderTessellationAndGeometryPointSize
        && deviceFeatures.multiDrawIndirect
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
    std::set<std::string> requiredExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());
    for (const auto& extension : availableExtensions) { 
        requiredExtensions.erase(extension.extensionName); 
    }
    return requiredExtensions.empty(); 
}
