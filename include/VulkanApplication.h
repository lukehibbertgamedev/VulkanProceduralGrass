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

#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3

#include "Vertex.h"
#include "Sphere.h"
#include "GrassBlade.h"

#include <optional>
#include <vector>
#include <string>

// Ground plane bounds definitions
#define MEADOW_SCALE_X 2
#define MEADOW_SCALE_Y 1
#define MEADOW_SCALE_Z 3

#define PLANE_SCALE_X MEADOW_SCALE_X // X is right.
#define PLANE_SCALE_Y MEADOW_SCALE_Z // Y is forward.
#define PLANE_SCALE_Z MEADOW_SCALE_Y // Z is up.

static constexpr bool kEnableValidationLayers = true;
const std::vector<const char*> kValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

static constexpr bool kEnableImGuiDemoWindow = true;
static constexpr int MAX_FRAMES_IN_FLIGHT = 2; // kMaxFramesInFlight
static constexpr unsigned int PARTICLE_COUNT = 1u << 23;

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

	int vertexCount = 0;

	float grassDrawCallTime = 0.0f;
};

struct CameraUniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

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
	VkResult createDescriptorSetLayouts();		
	VkResult createPipelines();
	VkResult createFrameBuffers();
	VkResult createCommandPool();
	VkResult createShaderStorageBuffers(); // For blade <3
	VkResult createVertexBuffer();
	VkResult createIndexBuffer();
	VkResult createUniformBuffers();
	VkResult createDescriptorPool();
	VkResult createDescriptorSets();
	VkResult createCommandBuffer();
	VkResult createSynchronizationObjects();
	VkResult createImGuiImplementation();

	void createMeshObjects();
	void populateBladeInstanceBuffer();
	void createBladeInstanceStagingBuffer();

	void prepareImGuiDrawData();

	// Creates a buffer, creates its memory requirements, and allocates and binds the buffer memory. Returns a VkResult.
	VkResult createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	// Copy from srcBuffer to dstBuffer passing the size of srcBuffer so the command knows how much data to copy. Performs vkCmdCopyBuffer.
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); 

	void cleanupApplication(GLFWwindow* window);

	void cleanupSwapchain();
	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	void linkWindowToVulkan(GLFWwindow* window);

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
private:
	VkResult createModelDescriptorSetLayout(); // For camera ubo.
	VkResult createGrassDescriptorSetLayout(); // For grass data ssbo.

	VkResult createModelDescriptorSets(); // For camera buffer object.
	VkResult createGrassDescriptorSets(); // For grass data buffer.

	VkResult createMeshPipeline(); // For regular rendering of meshes.
	VkResult createGrassPipeline(); // Exclusively for grass rendering.

	VkSampleCountFlagBits getMaxUsableMSAASampleCount();
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

	// Important Vulkan structures for HelloTriangle.
	VkInstance m_VkInstance = VK_NULL_HANDLE;							// Vulkan library handle.
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;			// Vulkan debug output handle.
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;					// GPU chosen as the default device.
	VkDevice m_LogicalDevice = VK_NULL_HANDLE;							// Handle to allow interaction with the GPU.

	// Window.
	GLFWwindow* window = nullptr;										// Glfw window pointer to keep track of the window for use in the swapchain.
	VkSurfaceKHR m_SurfaceKHR = VK_NULL_HANDLE;							// A handle to the area on screen that rendering can occur, connects Vulkan and the windowing system.

	// GPU features, properties, limitations.
	VkPhysicalDeviceProperties deviceProperties = {};					// A structure describing key GPU driver information (ie., driver name, driver version), including a collection of limitations.
	VkPhysicalDeviceFeatures deviceFeatures = {};						// A structure describing features that are supported by the GPU in Vulkan allowing optional additional capabilities.

	// Command queues.
	VkQueue computeQueue = VK_NULL_HANDLE;								// Queues compute shader commands for general-purpose computations.
	VkQueue graphicsQueue = VK_NULL_HANDLE;								// Queues graphics rendering commands for processing graphics shaders.
	VkQueue presentQueue = VK_NULL_HANDLE;								// Manages the commands for presenting the swapchain images to the screen.

	// Swapchain structures.
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;							// Handle to manage the swapping of rendering images to a window surface.
	std::vector<VkImage> swapChainImages = {};							// A collection of images used by the swapchain, each image corresponds to a framebuffer used for drawing.
	std::vector<VkImageView> swapChainImageViews = {};					// A collection of image views for each image to access and interpret image data for rendering.
	std::vector<VkFramebuffer> swapChainFramebuffers = {};				// A collection of framebuffer objects for each image to define attachments used in rendering images.
	VkFormat swapChainImageFormat = {};									// A reference to the format of the images used by the swapchain. Determines how pixel data is stored and displayed.
	VkExtent2D swapChainExtent = {};									// A reference to the dimensions of the images used by the swapchain. Determines the resolution of the images.

	// Render passes and pipelines.
	VkRenderPass renderPass = VK_NULL_HANDLE;							// Defines how rendering operations are performed and how framebuffers are used during rendering.
	//VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;					// A description of how the graphics pipeline will be configured.
	//VkPipeline graphicsPipeline = VK_NULL_HANDLE;						// Describes the entire state of the graphics rendering process (input assembly, rasterization, blending states, including shaders, etc...)
	VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;			// A description of how the compute pipeline will be configured, organises descriptor sets and push constants used to pass data to compute shaders.
	VkPipeline computePipeline = VK_NULL_HANDLE;						// Describes the entire state of how compute shaders will be executed allowing for general-purpose computing on the GPU, independent of the graphics pipeline.

	// Commands.
	VkCommandPool commandPool = VK_NULL_HANDLE;							// A handle to the manager that allocates command buffers to increase memory efficiency.
	std::vector<VkCommandBuffer> commandBuffers = {};					// A collection of command buffers, each buffer contains a list of GPU commands to be executed.
	std::vector<VkCommandBuffer> computeCommandBuffers = {};			// A collection of command buffers, each buffer contains a list of GPU commands to be executed specifically for compute shaders.

	// Synchronisation.
	std::vector<VkSemaphore> imageAvailableSemaphores = {};				// Per-frame synchronisation used for signalling when swapchain images are available for rendering.
	std::vector<VkSemaphore> renderFinishedSemaphores = {};				// Per-frame synchronisation used for signalling when rendering to an image is completed allowing safe image presentation to the screen.
	std::vector<VkFence> inFlightFences = {};							// Per-frame synchronisation used for signalling when a frame's rendering has completed allowing the GPU tasks to complete.
	std::vector<VkSemaphore> computeFinishedSemaphores = {};			// Per-frame synchronisation used for signalling when a compute command has finished executing.
	std::vector<VkFence> computeInFlightFences = {};					// Per-frame synchronisation used for signalling when a frame's compute command queue has completed.

	// Descriptors.
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;					// A handle to the manager that allocates descriptor sets to increase correct resource allocation.
	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;				// A handle to the manager that allocates descriptor sets specifically for ImGui resources necessary for rendering its interface.
	VkDescriptorSetLayout computeDescriptorSetLayout = VK_NULL_HANDLE;	// A description of how descriptor sets (ie., resources/buffers) are accessed specifically by compute shaders.
	std::vector<VkDescriptorSet> computeDescriptorSets = {};			// A collection of resources/buffers/images that can be bound to compute shaders during compute operations.

	VkBuffer quadVertexBuffer = VK_NULL_HANDLE;							// ... 
	VkDeviceMemory quadVertexBufferMemory = VK_NULL_HANDLE;				// ... 
	VkBuffer quadIndexBuffer = VK_NULL_HANDLE;							// ... 
	VkDeviceMemory quadIndexBufferMemory = VK_NULL_HANDLE;				// ... 

	// Multi-sampling.
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;			// ...

	// Other.
	float lastFrameTime = 0.0f;											// Used to calculate FPS.
	float deltaTime = 0.0f;												// Used to smooth out update logic.
	double lastTime = 0.0f;												// Used to calculated lastFrameTime.
	DriverData driverData = {};											// A custom collection of GPU data to be displayed easily in ImGui.
	uint32_t currentFrame = 0;											// A reference to the current frame in a double-buffered setup, helps manage which framebuffer is currently being used for rendering.
	bool framebufferResized = false;									// A flag to determine if the swapchain should be recreated to accomodate new window dimensions.
	int frameCount = 0;

	Sphere sphereMesh;
	Quad quadMesh;

	// Uniform buffer objects (UBO).
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	void* uniformBufferMapped;
	VkDescriptorSet uniformBufferDescriptorSet;
	 
	// Prepare staging buffer and its associated memory for holding the instance data temporarily before it gets transferred to the GPU.
	VkBuffer bladeInstanceStagingBuffer;
	VkDeviceMemory bladeInstanceStagingBufferMemory;
	std::vector<BladeInstanceData> localBladeInstanceBuffer; // A holding buffer of instance data per-blade. 

	// Shader storage buffer objects (SSBO). 
	VkBuffer bladeInstanceDataBuffer;
	VkDeviceMemory bladeInstanceDataBufferMemory;
	void* bladeInstanceDataBufferMapped;
	VkDescriptorSet bladeInstanceSSBODescriptorSet; // Rename to not include ssbo if this has ubo too

	MeshInstance groundPlane;

	VkDescriptorSetLayout modelDescriptorSetLayout; // Contains UBO for models (ie plane)
	VkDescriptorSetLayout grassDescriptorSetLayout; // Contains SSBO for grass buffer

	VkPipelineLayout modelPipelineLayout;
	VkPipelineLayout grassPipelineLayout;

	VkPipeline modelPipeline; // Pipeline structure for a model/object render pass.
	VkPipeline grassPipeline; // Pipeline structure for the grass render pass.

};