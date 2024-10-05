#pragma once

#include <../PortableVulkanSDK1.3/include/vulkan/vulkan.h>
#include <GLFW/glfw3.h>

static constexpr bool kEnableValidationLayers = false;

class VulkanApplication {

public:

	VkResult createInstance();
	void createDebugMessenger();
	VkResult createPhysicalDevice();

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
	VkDevice m_Device = VK_NULL_HANDLE;													// Vulkan device for commands.
	VkSurfaceKHR m_SurfaceKHR = VK_NULL_HANDLE;											// Vulkan window surface.

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
};