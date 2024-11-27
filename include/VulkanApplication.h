#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3

#include "Vertex.h"
#include "Mesh.h"
#include "GrassBlade.h"
#include "Camera.h"

#include <optional>
#include <vector>
#include <string>

// Ground plane bounds definitions (Z is typically up).
#define MEADOW_SCALE_X 6
#define MEADOW_SCALE_Y 6
#define MEADOW_SCALE_Z 1

static constexpr bool kEnableValidationLayers = true;
const std::vector<const char*> kValidationLayers = { "VK_LAYER_KHRONOS_validation" };
static constexpr bool kEnableImGuiDemoWindow = true;
static constexpr int kMaxFramesInFlight = 2; 

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

	uint32_t numVisible = 0;

	float grassDrawCallTime = 0.0f;
	float computeCallTime = 0.0f;
};

struct CameraUniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct NumBladesBufferObject {
	alignas(4) uint32_t numVisible;
};

// Allow draw call parameters to be stored in GPU memory.
// Passing the parameters in indirectly, not directly into the call.
struct BladeDrawIndirect {
	alignas(4) uint32_t vertexCount;
	alignas(4) uint32_t instanceCount;
	alignas(4) uint32_t firstVertex;
	alignas(4) uint32_t firstInstance;
};

struct PushConstantsObject {
	alignas(4) uint32_t totalNumBlades;
	alignas(4) float elapsed;
};

struct QueueFamilyIndices {
public:
	std::optional<uint32_t> graphicsAndComputeFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() { return graphicsAndComputeFamily.has_value() && presentFamily.has_value(); }
};

struct SwapChainSupportDetails {
public:
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanApplication {
public:		

	// Handles the set-up, queue submission, and presentation of a frame to the screen surface.
	void render();

	// Updates the uniform buffer object that is bound to both the model and grass pipeline, so they receive the most recent data (re-maps the memory).
	void updateUniformBuffer(uint32_t currentFrame);

	// ...
	uint32_t retrieveNumVisibleBlades();

	void linkWindowToVulkan(GLFWwindow* window);					// - - - - - .
	void linkCameraToVulkan(Camera* camera);						//			 |
	VkResult createInstance();										//			 |
	VkResult createDebugMessenger();								//			 |
	VkResult createPhysicalDevice();								//			 |
	VkResult createLogicalDevice();									//			 |
	VkResult createGlfwSurface();									//			 |
	VkResult createSwapchain();										//			 |
	VkResult recreateSwapchain();									//			 |
	VkResult createSwapchainImageViews();							//			 |
	VkResult createRenderPass();									//			 |
	VkResult createDescriptorSetLayouts();							//			 |
	VkResult createDepthResources();								//			 | - Vulkan application initialisation.
	VkResult createTextureResources();										//			 |
	VkResult createPipelines();										//			 |
	VkResult createFrameBuffers();									//			 |
	VkResult createCommandPool();									//			 |
	VkResult createShaderStorageBuffers(); 							//			 |
	VkResult createVertexBuffer();									//			 |
	VkResult createIndexBuffer();									//			 |
	VkResult createUniformBuffers();								//			 |
	VkResult createDescriptorPool();								//			 |
	VkResult createDescriptorSets();								//			 |
	VkResult createCommandBuffers();								//			 |
	VkResult createSynchronizationObjects();						//			 |
	VkResult createDefaultCamera();									//			 |
	VkResult createImGuiImplementation();							// - - - - - '

	void createMeshObjects();					// Creates the ground plane.
	void populateBladeInstanceBuffer();			// Populates a vector of blade instance data.
	void createBladeInstanceStagingBuffer();	// Copy the vector of instance data to the GPU.
	void createNumBladesBuffer();				// Create the buffer to hold the number of active blades.

	void prepareImGuiDrawData();

	// Creates a buffer, creates its memory requirements, and allocates and binds the buffer memory. Returns a VkResult.
	VkResult createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	// Creates an image, creates its memory requirements, and allocates and binds the image memory. Returls a VkResult.
	VkResult createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	// Copy from srcBuffer to dstBuffer passing the size of srcBuffer so the command knows how much data to copy. Performs vkCmdCopyBuffer.
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// Bind pipelines and populate the command buffer with commands (i.e., vkCmdDraw) for that pipeline.
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); 
	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);

	// Reads a SPIR-V code file and turns it into a handle that can be bound to a VkPipeline.
	VkShaderModule createShaderModule(const std::vector<char>& code);

	void cleanupApplication(GLFWwindow* window);
	void cleanupSwapchain();
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
private:

	VkResult createGraphicsCommandBuffer();
	VkResult createComputeCommandBuffer();

	VkResult createModelDescriptorSetLayout(); // For camera ubo.
	VkResult createGrassDescriptorSetLayout(); // For grass data ssbo.

	VkResult createModelDescriptorSets(); // For camera buffer object.
	VkResult createGrassDescriptorSets(); // For grass data buffer.

	VkResult createMeshPipeline(); // For regular rendering of meshes.
	VkResult createComputePipeline(); // Animates and culls grass blades.
	VkResult createGrassPipeline(); // Exclusively for grass rendering.

	VkResult createHeightMapImage();
	VkResult createHeightMapImageView();
	VkResult createHeightMapSampler();

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
	VkSampleCountFlagBits getMaxUsableMSAASampleCount();
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

	// Base concepts for Vulkan.
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

	// Commands.
	VkCommandPool commandPool = VK_NULL_HANDLE;							// A handle to the manager that allocates command buffers to increase memory efficiency.
	std::vector<VkCommandBuffer> commandBuffers = {};					// A collection of command buffers, each buffer contains a list of GPU commands to be executed.
	std::vector<VkCommandBuffer> computeCommandBuffers = {};			// A collection of command buffers, each buffer contains a list of GPU commands to be executed specifically for compute shaders.

	// Synchronisation.
	std::vector<VkSemaphore> imageAvailableSemaphores = {};				// Per-frame synchronisation used for signalling when swapchain images are available for rendering.
	std::vector<VkSemaphore> renderFinishedSemaphores = {};				// Per-frame synchronisation used for signalling when rendering to an image is completed allowing safe image presentation to the screen.
	std::vector<VkFence> inFlightFences = {};							// Per-frame synchronisation used for signalling when a frame's rendering has completed allowing the GPU tasks to complete.
	
	std::vector<VkFence> computeInFlightFences = {};
	std::vector<VkSemaphore> computeFinishedSemaphores = {};

	// Descriptors.
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;					// A handle to the manager that allocates descriptor sets to increase correct resource allocation.
	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;				// A handle to the manager that allocates descriptor sets specifically for ImGui resources necessary for rendering its interface.

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

	// Uniform buffer objects (UBO).
	VkBuffer uniformBuffer;												// The buffer object containing, most notably, the camera's view and projection matrices.
	VkDeviceMemory uniformBufferMemory;									// Allocated memory for this buffer object.
	void* uniformBufferMapped;											// A handle to map data to the buffer.
	VkDescriptorSet modelPipelineDescriptorSet;							// Descriptor set (one shader resource) that is bound to shaders within the model pipeline.
	 
	std::vector<VkBuffer> bladeInstanceStagingBuffer;								// A temporary holding buffer containing the blade data ready for CPU > GPU copy.
	std::vector<VkDeviceMemory> bladeInstanceStagingBufferMemory;					// Allocated memory for the holding buffer used to copy blade data to the GPU.
	std::vector<GrassBladeInstanceData> localBladeInstanceBuffer;			// A CPU buffer of instance data per-blade, populates the staging buffer, which populates the SSBO. 

	// Shader storage buffer objects (SSBO). 
	// There needs to be 2 of these since we are double buffering, the data is read from the last frame and written to the current, and then swapped.
	std::vector<VkBuffer> bladeInstanceDataBuffer;						// The shader resources containing all grass blade data.
	std::vector<VkDeviceMemory> bladeInstanceDataBufferMemory;			// Allocated memory for the shader resources.
	std::vector<void*> bladeInstanceDataBufferMapped;					// Handles to map data to the buffer.
	VkDescriptorSet grassPipelineDescriptorSet;							// Descriptor set (two shader resources) that is bound to shaders within the grass pipeline. 

	MeshTransform groundPlane;											// A handle to the ground plane transform.

	Quad quadMesh;														// One-time data structure containing vertex and index data for this mesh.
	VkBuffer quadVertexBuffer = VK_NULL_HANDLE;							// The vertex buffer for this mesh.
	VkDeviceMemory quadVertexBufferMemory = VK_NULL_HANDLE;				// The memory corresponding to the vertex buffer.
	VkBuffer quadIndexBuffer = VK_NULL_HANDLE;							// The index buffer for this mesh.
	VkDeviceMemory quadIndexBufferMemory = VK_NULL_HANDLE;				// The memory corresponding to the index buffer.

	BaseBladeShape bladeShapeMesh;										// One-time data structure containing vertex and index data for this mesh.
	VkBuffer bladeShapeVertexBuffer = VK_NULL_HANDLE;					// The vertex buffer for this mesh.
	VkDeviceMemory bladeShapeVertexBufferMemory = VK_NULL_HANDLE;		// The memory corresponding to the vertex buffer.
	VkBuffer bladeShapeIndexBuffer = VK_NULL_HANDLE;					// The index buffer for this mesh.
	VkDeviceMemory bladeShapeIndexBufferMemory = VK_NULL_HANDLE;		// The memory corresponding to the index buffer.

	VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;	// A layout that determines what shader resources can later be bound for this pipeline. Contains UBO for models (ie plane).
	VkDescriptorSetLayout grassDescriptorSetLayout = VK_NULL_HANDLE;	// A layout that determines what shader resources can later be bound for this pipeline. Contains SSBO for grass instance buffer.

	VkPipelineLayout modelPipelineLayout = VK_NULL_HANDLE;				// A pipeline configuration for the rendering of models/meshes.
	VkPipelineLayout grassPipelineLayout = VK_NULL_HANDLE;				// A pipeline configuration for the rendering of grass blades.
	VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;			// A pipeline configuration for the culling and animation of grass blades.

	VkPipeline modelPipeline = VK_NULL_HANDLE;							// A pipeline structure for a model/mesh render pass.
	VkPipeline grassPipeline = VK_NULL_HANDLE;							// A pipeline structure for the grass blade render pass.
	VkPipeline computePipeline = VK_NULL_HANDLE;						// A pipeline structure for the grass animation and culling pass.

	Camera* camera;														// A handle to a dynamic camera that works with WASDEQ, arrow keys, LJ, and RTY. 

	VkImage depthImage = VK_NULL_HANDLE;								// A handle to the image that represents a depth stencil.
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;					// Allocated memory for this image resource.
	VkImageView depthImageView = VK_NULL_HANDLE;						// A handle to the actual image data for the depth stencil.

	VkBuffer numBladesBuffer;											// ...
	VkDeviceMemory numBladesBufferMemory;								// ...

	// Height map data.
	VkImage heightMapImage;
	VkImageView heightMapImageView;
	VkDeviceMemory heightMapImageMemory;
	VkSampler heightMapSampler;

};