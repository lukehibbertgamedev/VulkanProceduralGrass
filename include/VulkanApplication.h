#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <../PortableVulkanSDK1.3/include/vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <optional>

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

	VkResult createInstance();
	void createDebugMessenger();
	VkResult createPhysicalDevice();
	VkResult createLogicalDevice();
	VkResult createWin32Surface(GLFWwindow* window);
	VkResult createGlfwSurface(GLFWwindow* window);

	void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
private:
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& outCreateInfo);
	VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	bool checkPhysicalDeviceSuitability(VkPhysicalDevice device);

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
	
};