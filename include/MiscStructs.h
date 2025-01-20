#pragma once

// ===============================================================================================================================================================================

// A collection of miscellaneous structures, some for abstractions, some for core concepts.

// ===============================================================================================================================================================================

#include <string>
#include <optional>

// ===============================================================================================================================================================================

// Structure to determine if the queue families have supporting operations.
struct QueueFamilyIndices {
public:
	std::optional<uint32_t> graphicsAndComputeFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() { return graphicsAndComputeFamily.has_value() && presentFamily.has_value(); }
};

// Find the relevant queue families that support the needs of graphics, compute, and present operations.
inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		// Graphics and compute family.
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			indices.graphicsAndComputeFamily = i;
		}

		// Present queue family.
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

// Collection of data relevant to the GPU and application to display in Dear ImGui.
struct GPUData {
public:
	std::string name;
	uint32_t version = 0;
	unsigned int versionMajor = 0, versionMinor = 0;
	unsigned int apiMajor = 0, apiMinor = 0, apiPatch = 0;
	uint32_t numVisible = 0;
};

// Vulkan-style info struct for abstracted buffer creation.
struct BufferCreateInfo {
public:
	VkDeviceSize size;
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags memProperties;
	VkBuffer* pBuffer;
	VkDeviceMemory* pBufferMemory;
};

// Vulkan-style info struct for abstracted image creation.
struct ImageCreateInfo {
	uint32_t width;
	uint32_t height;
	uint32_t mipLevels;
	VkSampleCountFlagBits numSamples;
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags properties;
	VkImage* pImage;
	VkDeviceMemory* pImageMemory;
};