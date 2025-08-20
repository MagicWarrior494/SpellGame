#pragma once
#include <algorithm>
#include <stdexcept>

namespace Vulkan {
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

    inline void WaitForValidFramebufferSize(GLFWwindow* window, glm::uvec2& framebufferSize) {
		// GLFW can report 0 size during minimization or before window appears,
		// so wait until we get a non-zero size
		int width = 0, height = 0;
		do {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();  // Wait for events (resize, etc.) to avoid busy loop
		} while (width == 0 || height == 0);

		framebufferSize.x = static_cast<uint32_t>(width);
		framebufferSize.y = static_cast<uint32_t>(height);
	}

    inline void CreateSwapchain(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface, SurfaceFlags flags) {
        VulkanCore& vulkanCore = *VC;
        WaitForValidFramebufferSize(vulkanSurface.p_GLFWWindow, vulkanSurface.windowSize);

        SwapChainSupportDetails swapChainSupport;
        VkPhysicalDevice physicalDevice = vulkanCore.vkPhysicalDevice;
        VkSurfaceKHR surface = vulkanSurface.vkSurface;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainSupport.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            swapChainSupport.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapChainSupport.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            swapChainSupport.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapChainSupport.presentModes.data());
        }

        // Choose surface format
        VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
        for (const auto& availableFormat : swapChainSupport.formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = availableFormat;
                break;
            }
        }

        // Clamp window size extent
        vulkanSurface.windowSize.x = std::clamp(vulkanSurface.windowSize.x,
            swapChainSupport.capabilities.minImageExtent.width,
            swapChainSupport.capabilities.maxImageExtent.width);
        vulkanSurface.windowSize.y = std::clamp(vulkanSurface.windowSize.y,
            swapChainSupport.capabilities.minImageExtent.height,
            swapChainSupport.capabilities.maxImageExtent.height);

        VkExtent2D extent = { vulkanSurface.windowSize.x, vulkanSurface.windowSize.y };

        // Choose present mode
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if ((flags & SurfaceFlags::EnableVSync) == SurfaceFlags::None) {
            for (const auto& mode : swapChainSupport.presentModes) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    presentMode = mode;
                    break;
                }
            }
        }

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = surface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent = extent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = {
            vulkanCore.d_PhysicalDeviceData.graphicsIndex.value(),
            vulkanCore.d_PhysicalDeviceData.presentIndex.value()
        };

        if (vulkanCore.d_PhysicalDeviceData.graphicsIndex != vulkanCore.d_PhysicalDeviceData.presentIndex) {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0;
            swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        }

        swapchainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = presentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = vulkanSurface.vkSwapChain; // Use old swapchain if recreating

        if (vkCreateSwapchainKHR(vulkanCore.vkDevice, &swapchainCreateInfo, nullptr, &vulkanSurface.vkSwapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create Swap Chain!");
        }

        // After successful creation, destroy old swapchain if needed:
        if (swapchainCreateInfo.oldSwapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vulkanCore.vkDevice, swapchainCreateInfo.oldSwapchain, nullptr);
        }
    }

    inline void CleanupSwapchain(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
    {
        VulkanCore& vulkanCore = *VC;
        if (vulkanSurface.vkSwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vulkanCore.vkDevice, vulkanSurface.vkSwapChain, nullptr);
            vulkanSurface.vkSwapChain = VK_NULL_HANDLE;
        }
    }
}