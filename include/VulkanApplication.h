#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <../PortableVulkanSDK1.3/include/vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <array>        // For std::array
#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3

#include <optional>
#include <vector>
#include <string>

static constexpr bool kEnableValidationLayers = false;
static constexpr bool kEnableImGuiDemoWindow = true;
static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // kMaxFramesInFlight
static constexpr unsigned int PARTICLE_COUNT = 1u << 9;

struct DriverData {
public:
	std::string name;
	uint32_t version = 0;
	unsigned int versionMajor = 0, versionMinor = 0, versionPatch = 0;
	uint32_t deviceID = 0;
	VkPhysicalDeviceType deviceType;
	unsigned int apiMajor = 0, apiMinor = 0, apiPatch = 0;
	uint32_t deviceCount = 0;

	uint32_t queueFamilyCount = 0;
	VkQueueFlags queueFlags;
};

struct Particle {
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 color;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Particle);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Particle, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Particle, color);

		return attributeDescriptions;
	}
};


//struct Vertex {
//	glm::vec3 pos;
//	glm::vec3 color;
//	glm::vec2 texCoord;
//
//	static VkVertexInputBindingDescription getBindingDescription() {
//		VkVertexInputBindingDescription bindingDescription{};
//		bindingDescription.binding = 0;
//		bindingDescription.stride = sizeof(Vertex);
//		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//		return bindingDescription;
//	}
//
//	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
//		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
//		attributeDescriptions[0].binding = 0;
//		attributeDescriptions[0].location = 0;
//		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//		attributeDescriptions[0].offset = offsetof(Vertex, pos);
//		attributeDescriptions[1].binding = 0;
//		attributeDescriptions[1].location = 1;
//		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//		attributeDescriptions[1].offset = offsetof(Vertex, color);
//		attributeDescriptions[2].binding = 0;
//		attributeDescriptions[2].location = 2;
//		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
//		return attributeDescriptions;
//	}
//};

// See more on alignment: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
//struct UniformBufferObject {
//	alignas(16) glm::mat4 model;
//	alignas(16) glm::mat4 view;
//	alignas(16) glm::mat4 proj;
//};

struct UniformBufferObject {
	float deltaTime = 1.0f;
};

//const std::vector<Vertex> vertices = {
//	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {-0.0f, 0.0f}},
//	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f}},
//	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 1.0f}},
//	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {-0.0f, 1.0f}},
//
//	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {-0.0f, 0.0f}},
//	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f}},
//	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 1.0f}},
//	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {-0.0f, 1.0f}}
//};
//
//const std::vector<uint16_t> indices = {
//	0, 1, 2, 2, 3, 0,
//	4, 5, 6, 6, 7, 4
//};

class VulkanApplication {

public:

	struct QueueFamilyIndices {
	public:
		std::optional<uint32_t> graphicsAndComputeFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() { return graphicsAndComputeFamily.has_value() && presentFamily.has_value(); }
	};
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	struct SwapChainSupportDetails {
	public:
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	VkShaderModule createShaderModule(const std::vector<char>& code);

	void render();
	void updateUniformBuffer(uint32_t currentFrame);

	VkResult createInstance();
	VkResult createDebugMessenger();
	VkResult createPhysicalDevice();
	VkResult createLogicalDevice();
	VkResult createGlfwSurface();
	VkResult createSwapchain();
	VkResult recreateSwapchain();
	VkResult createSwapchainImageViews();
	VkResult createRenderPass();
	VkResult createComputeDescriptorSetLayout(); // Compute relevant
	VkResult createDescriptorSetLayout();
	VkResult createGraphicsPipeline();
	VkResult createComputePipeline(); // Compute relevant
	VkResult createFrameBuffers();
	VkResult createCommandPool();
	VkResult createShaderStorageBuffers(); // Compute relevant
	VkResult createColourResources();
	VkResult createDepthResources();
	VkResult createTextureImage();
	VkResult createTextureImageView();
	VkResult createTextureSampler();
	VkResult createVertexBuffer();
	VkResult createIndexBuffer();
	VkResult createUniformBuffers();
	VkResult createDescriptorPool();
	VkResult createComputeDescriptorSets(); // Compute relevant
	VkResult createDescriptorSets();
	VkResult createComputeCommandBuffer(); // Compute relevant
	VkResult createCommandBuffer();
	VkResult createSynchronizationObjects();
	VkResult createImGuiImplementation();

	void prepareImGuiDrawData();

	VkResult createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkResult createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer); // Compute relevant
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); 

	void cleanupApplication(GLFWwindow* window);

	void cleanupSwapchain();
	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	void linkWindowToVulkan(GLFWwindow* window);

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
private:
	VkSampleCountFlagBits getMaxUsableMSAASampleCount();
	bool depthFormatHasStencilComponent(VkFormat format);
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	uint32_t findGPUMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	SwapChainSupportDetails checkSwapchainSupport(VkPhysicalDevice device);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo);
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	bool checkPhysicalDeviceSuitability(VkPhysicalDevice device);
	bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device);
public:

	// FIXME: All member variables should be default initialised.

	// Vulkan.
	VkInstance m_VkInstance = VK_NULL_HANDLE;											// Vulkan library handle.
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;							// Vulkan debug output handle.
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;									// GPU chosen as the default device.
	VkDevice m_LogicalDevice = VK_NULL_HANDLE;													// Vulkan device for commands.
	VkSurfaceKHR m_SurfaceKHR = VK_NULL_HANDLE;											// Vulkan window surface.

	GLFWwindow* window = nullptr;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;


	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;

	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline;

	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrame = 0;
	bool framebufferResized = false;

	VkBuffer vertexBuffer = VK_NULL_HANDLE; 
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE; 
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;

	DriverData driverData = {};


	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets;

	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkImageView textureImageView = VK_NULL_HANDLE;
	VkSampler textureSampler = VK_NULL_HANDLE;

	VkImage depthImage = VK_NULL_HANDLE;
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	std::vector<VkBuffer> shaderStorageBuffers;
	std::vector<VkDeviceMemory> shaderStorageBuffersMemory;

	VkDescriptorSetLayout computeDescriptorSetLayout;
	VkQueue computeQueue;
	std::vector<VkDescriptorSet> computeDescriptorSets;
	VkPipelineLayout computePipelineLayout;
	VkPipeline computePipeline;
	std::vector<VkFence> computeInFlightFences;
	std::vector<VkSemaphore> computeFinishedSemaphores;
	std::vector<VkCommandBuffer> computeCommandBuffers;

	float lastFrameTime = 0.0f;
	float dt = 0.0f;
	double lastTime = 0.0f;


};