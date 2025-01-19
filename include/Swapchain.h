#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct SwapChain {
public:	
	VkSwapchainKHR handle = VK_NULL_HANDLE;						// Handle to manage the swapping of rendering images to a window surface.
	std::vector<VkImage> images = {};							// A collection of images used by the swapchain, each image corresponds to a framebuffer used for drawing.
	std::vector<VkImageView> imageViews = {};					// A collection of image views for each image to access and interpret image data for rendering.
	std::vector<VkFramebuffer> framebuffers = {};				// A collection of framebuffer objects for each image to define attachments used in rendering images.
	VkFormat imageFormat = {};									// A reference to the format of the images used by the swapchain. Determines how pixel data is stored and displayed.
	VkExtent2D extents = {};									// A reference to the dimensions of the images used by the swapchain. Determines the resolution of the images.
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};