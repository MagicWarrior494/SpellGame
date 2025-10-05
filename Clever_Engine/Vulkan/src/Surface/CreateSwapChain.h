#pragma once
#include <algorithm>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "SurfaceFlags.h"

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

    struct SwapChainCreateInfo {
        GLFWwindow* p_GLFWWindow = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        glm::uvec2 windowSize;
		VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
        SurfaceFlags flags = SurfaceFlags::None;
	};

    inline void CreateSwapchain(std::shared_ptr<VulkanCore> VC, SwapChainCreateInfo info, VkSwapchainKHR& swapChain) {
        VulkanCore& vulkanCore = *VC;
        WaitForValidFramebufferSize(info.p_GLFWWindow, info.windowSize);

        SwapChainSupportDetails swapChainSupport;
        VkPhysicalDevice physicalDevice = vulkanCore.vkPhysicalDevice;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, info.surface, &swapChainSupport.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, info.surface, &formatCount, nullptr);
        if (formatCount != 0) {
            swapChainSupport.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, info.surface, &formatCount, swapChainSupport.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, info.surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            swapChainSupport.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, info.surface, &presentModeCount, swapChainSupport.presentModes.data());
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
        info.windowSize.x = std::clamp(info.windowSize.x,
            swapChainSupport.capabilities.minImageExtent.width,
            swapChainSupport.capabilities.maxImageExtent.width);
        info.windowSize.y = std::clamp(info.windowSize.y,
            swapChainSupport.capabilities.minImageExtent.height,
            swapChainSupport.capabilities.maxImageExtent.height);

        VkExtent2D extent = { info.windowSize.x, info.windowSize.y };

        // Choose present mode
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        if ((info.flags & SurfaceFlags::EnableVSync) == SurfaceFlags::None) {
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
        swapchainCreateInfo.surface = info.surface;
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

        if (vkCreateSwapchainKHR(vulkanCore.vkDevice, &swapchainCreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create Swap Chain!");
        }
    }
}