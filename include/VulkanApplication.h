#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp> 

// Dear ImGui & its relevant backends.
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

// Custom headers.
#include "Buffer.h"
#include "Swapchain.h"
#include "Mesh.h"
#include "GrassBlade.h"
#include "Camera.h"
#include "MiscStructs.h"
#include "Constants.h"

// STL.
#include <optional>
#include <vector>

class VulkanApplication {
public:		

	// Handles the set-up, queue submission, and presentation of a frame to the screen surface.
	void render();

	// Set-up/preparation.
	void linkWindowToVulkan(GLFWwindow* window);	// Link the global window to the Vulkan application.
	void linkCameraToVulkan(Camera* camera);		// Link the global camera to the Vulkan application.
	VkResult initialiseApplication();				// Set up all Vulkan and application specific structures.
	void prepareImGuiDrawData();					// Dear ImGui draw data commands for later rendering.

	// Release memory allocations.
	void cleanupApplication(GLFWwindow* window);
private:

	VkResult createInstance();										// - - - - - .
	VkResult createDebugMessenger();								//			 |
	VkResult createPhysicalDevice();								//			 |
	VkResult createLogicalDevice();									//			 |
	VkResult createGlfwSurface();									//			 |
	VkResult createSwapchain();										//			 |
	VkResult recreateSwapchain();									//			 |
	VkResult createSwapchainImageViews();							//			 |
	VkResult createRenderPass();									//			 |
	VkResult createDescriptorSetLayouts();							//			 |
	VkResult createModelDescriptorSetLayout();						//			 |
	VkResult createGrassDescriptorSetLayout();						//			 |
	VkResult createDepthResources();								//			 | 
	VkResult createTextureResources();								//			 |
	VkResult createHeightMapImage();								//			 |
	VkResult createHeightMapImageView();							//			 |
	VkResult createHeightMapSampler();								//			 |
	VkResult createPipelines();										//			 | 
	VkResult createMeshPipeline();									//			 |
	VkResult createComputePipeline();								//			 |
	VkResult createGrassPipeline();									//			 | ---> Vulkan application initialisation.
	VkResult createFrameBuffers();									//			 |
	VkResult createCommandPool();									//			 |
	void createMeshObjects();										//			 |
	void populateBladeInstanceBuffer();								//			 |
	VkResult createShaderStorageBuffers(); 							//			 |
	VkResult createVertexBuffer();									//			 |
	VkResult createIndexBuffer();									//			 |
	VkResult createUniformBuffers();								//			 |
	VkResult createDescriptorPool();								//			 |
	void createNumBladesBuffer();									//			 |
	VkResult createDescriptorSets();								//			 |
	void createBladeInstanceStagingBuffer();						//			 |
	VkResult createModelDescriptorSets();							//			 |
	VkResult createGrassDescriptorSets();							//			 |
	VkResult createCommandBuffers();								//			 |
	VkResult createGraphicsCommandBuffer();							//			 |
	VkResult createComputeCommandBuffer();							//			 |
	VkResult createSynchronizationObjects();						//			 |
	VkResult createDefaultCamera();									//			 |
	VkResult createImGuiImplementation();							// - - - - - '

	// Updates the uniform buffer object that is bound to both the model and grass pipeline, so they receive the most recent data (re-maps the memory).
	void updateUniformBuffer(uint32_t currentFrame);

	// Map the memory from the num blades buffer to determine number of grass blades to draw.
	uint32_t retrieveNumVisibleBlades();

	// Creates a buffer, creates its memory requirements, and allocates and binds the buffer memory. Returns a VkResult.
	VkResult createBuffer(BufferCreateInfo& bufferCreateInfo);

	// Creates an image, creates its memory requirements, and allocates and binds the image memory. Returls a VkResult.
	VkResult createImage(ImageCreateInfo& imageCreateInfo);

	// Copy from srcBuffer to dstBuffer passing the size of srcBuffer so the command knows how much data to copy. Performs vkCmdCopyBuffer.
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// Bind pipelines and populate the command buffer with commands (i.e., vkCmdDraw) for the graphics pipeline.
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	// Bind pipelines and populate the command buffer with commands (i.e., vkCmdDispatch) for the compute pipeline.
	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);

	// Reads a SPIR-V code file and turns it into a handle that can be bound to a VkPipeline.
	VkShaderModule createShaderModule(const std::vector<char>& code);

	// Determine best tiling format for images.
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// Determine the best supported format for the depth images.
	VkFormat findDepthFormat();

	// Determine the best supported format for the swapchain images.
	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	// Determine the best supported present mode for the swapchain.
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	// Determine the best extents for the swapchain.
	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	// Abstraction to create an image view for an image.
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	// Performs single time commands to copy the data from a buffer to an image.
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	// Performs single time commands to insert an image memory dependency.
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	// Begin recording commands for use outside of the main command buffer recording.
	VkCommandBuffer beginSingleTimeCommands();

	// End and submit recorded commands for use outside of the main command buffer recording.
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	// Determine the best supported memory type for a GPU buffer.
	uint32_t findGPUMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	// Determine if the swapchain is supported by its capabilities, format, and present modes.
	SwapChain checkSwapchainSupport(VkPhysicalDevice device);

	// Fill the create struct for the validation layers debug output.
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo);

	// Load the proc addresst to create the validation layers debug output.
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	// Load the proc address to destroy the validation layers debug output.
	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

	// Determine if the physical device features meet certain application expectations.
	bool checkPhysicalDeviceSuitability(VkPhysicalDevice device);

	// Determine what device extensions the application can support.
	bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device);
public:

	// Base concepts for Vulkan.
	VkInstance m_VkInstance = VK_NULL_HANDLE;							// Vulkan library handle.
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;			// Vulkan debug output handle.
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;					// GPU chosen as the default device.
	VkDevice m_LogicalDevice = VK_NULL_HANDLE;							// Handle to allow interaction with the GPU.
	SwapChain swapchainData = {};										// Structure of swapchain data.

	// Window.
	GLFWwindow* window = nullptr;										// Glfw window pointer to keep track of the window for use in the swapchain.
	VkSurfaceKHR m_SurfaceKHR = VK_NULL_HANDLE;							// A handle to the area on screen that rendering can occur, connects Vulkan and the windowing system.

	// GPU features & properties.
	VkPhysicalDeviceProperties deviceProperties = {};					// A structure describing key GPU driver information (ie., driver name, driver version), including a collection of limitations.
	VkPhysicalDeviceFeatures deviceFeatures = {};						// A structure describing features that are supported by the GPU in Vulkan allowing optional additional capabilities.

	// Command queues.
	VkQueue computeQueue = VK_NULL_HANDLE;								// Queues compute shader commands for general-purpose computations.
	VkQueue graphicsQueue = VK_NULL_HANDLE;								// Queues graphics rendering commands for processing graphics shaders.
	VkQueue presentQueue = VK_NULL_HANDLE;								// Manages the commands for presenting the swapchain images to the screen.

	// Commands.
	VkCommandPool commandPool = VK_NULL_HANDLE;							// A handle to the manager that allocates command buffers to increase memory efficiency.
	std::vector<VkCommandBuffer> commandBuffers = {};					// A collection of command buffers, each buffer contains a list of GPU commands to be executed.
	std::vector<VkCommandBuffer> computeCommandBuffers = {};			// A collection of command buffers, each buffer contains a list of GPU commands to be executed specifically for compute shaders.

	// Render passes and pipelines.
	VkRenderPass renderPass = VK_NULL_HANDLE;							// Defines how rendering operations are performed and how framebuffers are used during rendering.
	VkPipelineLayout modelPipelineLayout = VK_NULL_HANDLE;				// A pipeline configuration for the rendering of models/meshes.
	VkPipelineLayout grassPipelineLayout = VK_NULL_HANDLE;				// A pipeline configuration for the rendering of grass blades.
	VkPipelineLayout computePipelineLayout = VK_NULL_HANDLE;			// A pipeline configuration for the culling and animation of grass blades.
	VkPipeline modelPipeline = VK_NULL_HANDLE;							// A pipeline structure for a model/mesh render pass.
	VkPipeline grassPipeline = VK_NULL_HANDLE;							// A pipeline structure for the grass blade render pass.
	VkPipeline computePipeline = VK_NULL_HANDLE;						// A pipeline structure for the grass animation and culling pass.
		
	// Synchronisation.
	std::vector<VkSemaphore> imageAvailableSemaphores = {};				// Per-frame synchronisation used for signalling when swapchain images are available for rendering.
	std::vector<VkSemaphore> renderFinishedSemaphores = {};				// Per-frame synchronisation used for signalling when rendering to an image is completed allowing safe image presentation to the screen.
	std::vector<VkFence> inFlightFences = {};							// Per-frame synchronisation used for signalling when a frame's rendering has completed allowing the GPU tasks to complete.
	std::vector<VkFence> computeInFlightFences = {};
	std::vector<VkSemaphore> computeFinishedSemaphores = {};

	// Descriptors.
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;					// A handle to the manager that allocates descriptor sets to increase correct resource allocation.
	VkDescriptorPool imguiDescriptorPool = VK_NULL_HANDLE;				// A handle to the manager that allocates descriptor sets specifically for ImGui resources necessary for rendering its interface.
	VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;	// A layout that determines what shader resources can later be bound for this pipeline. Contains UBO for models (ie plane).
	VkDescriptorSetLayout grassDescriptorSetLayout = VK_NULL_HANDLE;	// A layout that determines what shader resources can later be bound for this pipeline. Contains SSBO for grass instance buffer.
	VkDescriptorSet modelPipelineDescriptorSet = VK_NULL_HANDLE;		// Descriptor set (one shader resource) that is bound to shaders within the model pipeline.
	VkDescriptorSet grassPipelineDescriptorSet;							// Descriptor set (two shader resources) that is bound to shaders within the grass pipeline. 

	// Buffers.
	std::vector<VkBuffer> bladeInstanceStagingBuffer = {};				// A temporary holding buffer containing the blade data ready for CPU > GPU copy.
	std::vector<VkBuffer> bladeInstanceDataBuffer;						// The shader resources containing all grass blade data.
	VkBuffer uniformBuffer = VK_NULL_HANDLE;							// The buffer object containing, most notably, the camera's view and projection matrices.
	VkBuffer quadVertexBuffer = VK_NULL_HANDLE;							// The vertex buffer for this mesh.
	VkBuffer quadIndexBuffer = VK_NULL_HANDLE;							// The index buffer for this mesh.
	VkBuffer bladeShapeVertexBuffer = VK_NULL_HANDLE;					// The vertex buffer for this mesh.
	VkBuffer bladeShapeIndexBuffer = VK_NULL_HANDLE;					// The index buffer for this mesh.
	VkBuffer numBladesBuffer = VK_NULL_HANDLE;							// A buffer for determining how many blades to draw.
	std::vector<VkDeviceMemory> bladeInstanceStagingBufferMemory = {};	// Allocated memory for the holding buffer used to copy blade data to the GPU.
	std::vector<VkDeviceMemory> bladeInstanceDataBufferMemory;			// Allocated memory for the shader resources.
	VkDeviceMemory uniformBufferMemory = VK_NULL_HANDLE;				// Allocated memory for this buffer object.
	VkDeviceMemory quadVertexBufferMemory = VK_NULL_HANDLE;				// The memory corresponding to the vertex buffer.
	VkDeviceMemory quadIndexBufferMemory = VK_NULL_HANDLE;				// The memory corresponding to the index buffer.
	VkDeviceMemory bladeShapeVertexBufferMemory = VK_NULL_HANDLE;		// The memory corresponding to the vertex buffer.
	VkDeviceMemory bladeShapeIndexBufferMemory = VK_NULL_HANDLE;		// The memory corresponding to the index buffer.
	VkDeviceMemory numBladesBufferMemory = VK_NULL_HANDLE;				// The memory corresponding to the num blades buffer.
	void* uniformBufferMapped = nullptr;								// A handle to map data to the buffer.
	std::vector<void*> bladeInstanceDataBufferMapped;					// Handles to map data to the buffer.	

	// Images.
	VkImage heightMapImage = VK_NULL_HANDLE;							// A handle to the image that represents the height map.
	VkImage depthImage = VK_NULL_HANDLE;								// A handle to the image that represents a depth stencil.
	VkImageView heightMapImageView = VK_NULL_HANDLE;					// A handle to the actual image data for the height map.
	VkImageView depthImageView = VK_NULL_HANDLE;						// A handle to the actual image data for the depth stencil.
	VkDeviceMemory heightMapImageMemory = VK_NULL_HANDLE;				// Allocated memory for this image resource.
	VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;					// Allocated memory for this image resource.
	
	// Miscellaneous.
	float lastFrameTime = 0.0f;											// Used to calculate FPS.
	float deltaTime = 0.0f;												// Used to smooth out update logic.
	double lastTime = 0.0f;												// Used to calculated lastFrameTime.
	GPUData driverData = {};											// A custom collection of GPU data to be displayed easily in ImGui.
	uint32_t currentFrame = 0;											// A reference to the current frame in a double-buffered setup, helps manage which framebuffer is currently being used for rendering.
	bool framebufferResized = false;									// A flag to determine if the swapchain should be recreated to accomodate new window dimensions.
	int frameCount = 0;													// Determines the number of frames passed since start-up.
	Camera* camera = nullptr;											// A handle to a dynamic camera that works with WASDEQ, arrow keys, LJ, and RTY. 
	Quad quadMesh;														// One-time data structure containing vertex and index data for this mesh.
	BaseBladeShape bladeShapeMesh;										// One-time data structure containing vertex and index data for this mesh.
	MeshTransform groundPlane;											// A handle to the ground plane transform.
	std::vector<GrassBladeInstanceData> localBladeInstanceBuffer = {};	// A CPU buffer of instance data per-blade, populates the staging buffer, which populates the SSBO. 
	VkSampler heightMapSampler = VK_NULL_HANDLE;						// Sampler for use in sampling the height map to displace the terrain.
};
