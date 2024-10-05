#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <../PortableVulkanSDK1.3/include/vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <optional>
#include <vector>

static constexpr bool kEnableValidationLayers = false;


class VulkanApplication {

public:

	struct QueueFamilyIndices {
	public:
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	struct SwapChainSupportDetails {
	public:
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	VkShaderModule createShaderModule(const std::vector<char>& code);

	VkResult createInstance();
	void createDebugMessenger();
	VkResult createPhysicalDevice();
	VkResult createLogicalDevice();
	VkResult createGlfwSurface(GLFWwindow* window);
	VkResult createSwapchain(GLFWwindow* window);
	VkResult createSwapchainImageViews();
	VkResult createGraphicsPipeline();

	VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
private:
	SwapChainSupportDetails checkSwapchainSupport(VkPhysicalDevice device);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo);
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	bool checkPhysicalDeviceSuitability(VkPhysicalDevice device);
	bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device);
public:
	// Vulkan.
	VkInstance m_VkInstance = VK_NULL_HANDLE;											// Vulkan library handle.
	VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;							// Vulkan debug output handle.
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;									// GPU chosen as the default device.
	VkDevice m_LogicalDevice = VK_NULL_HANDLE;													// Vulkan device for commands.
	VkSurfaceKHR m_SurfaceKHR = VK_NULL_HANDLE;											// Vulkan window surface.

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	VkPipelineLayout pipelineLayout;
};