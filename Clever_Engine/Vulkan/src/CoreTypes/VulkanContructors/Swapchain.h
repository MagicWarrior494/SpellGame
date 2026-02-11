#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <glm.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <stdexcept>

#include "CoreTypes/VulkanCore.h"
#include "Image/VulkanImage.h"
#include "Flags.h"

namespace Vulkan {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct SwapChainCreateInfo {
        void* windowPtr = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        glm::uvec2 windowSize;
        VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
        SurfaceFlags flags = SurfaceFlags::None;
    };

    void WaitForValidFramebufferSize(void* window, glm::uvec2& framebufferSize) {
        // GLFW can report 0 size during minimization or before window appears,
        // so wait until we get a non-zero size
        int width = 0, height = 0;
        do {
            glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);
            glfwWaitEvents();  // Wait for events (resize, etc.) to avoid busy loop
        } while (width == 0 || height == 0);

        framebufferSize.x = static_cast<uint32_t>(width);
        framebufferSize.y = static_cast<uint32_t>(height);
    }

    void CreateSwapchain(VulkanCore* VC, SwapChainCreateInfo info, VkSwapchainKHR& swapChain)
    {
        VulkanCore& vulkanCore = *VC;
        WaitForValidFramebufferSize(info.windowPtr, info.windowSize);

        SwapChainSupportDetails swapChainSupport;
        VkPhysicalDevice physicalDevice = vulkanCore.physicalDeviceData.vkPhysicalDevice;

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
            vulkanCore.physicalDeviceData.graphicsIndex.value(),
            vulkanCore.physicalDeviceData.presentIndex.value()
        };

        if (vulkanCore.physicalDeviceData.graphicsIndex != vulkanCore.physicalDeviceData.presentIndex) {
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

	//Before calling delete old SwapChainImages, Old framebuffers, and old depth images
    std::vector<VulkanImage> CreateSwapchainImages(
        VulkanCore* VC,
        VkSwapchainKHR swapChain,
		VkFormat swapChainImageFormat,
		glm::ivec2 windowSize,
        SwapchainAttachmentType attachmentType)
    {
        VulkanCore& vulkanCore = *VC;
        VkDevice device = vulkanCore.vkDevice;

        // --- Get swapchain images ---
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());

        std::vector<VulkanImage> result{};

        result.resize(imageCount);

        for (uint32_t i = 0; i < imageCount; ++i) {
            result[i].image = images[i];
            result[i].memory = VK_NULL_HANDLE; // Owned by swapchain
            result[i].view = VK_NULL_HANDLE;
            result[i].format = swapChainImageFormat;
            result[i].extent = {
                static_cast<unsigned int>(windowSize.x),
                static_cast<unsigned int>(windowSize.y),
                1
            };
        }

        // --- Create image views for swapchain images ---
        for (uint32_t i = 0; i < imageCount; ++i) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = result[i].image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewInfo, nullptr,
                &result[i].view) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create swapchain image view");
            }
        }
		return result;
    }

    std::vector<VulkanImage> CreateDepthImages(
        VulkanCore* VC,
		uint32_t imageCount,
        glm::ivec2 windowSize,
        SwapchainAttachmentType attachmentType)
    {
        std::vector<VulkanImage> result{};

        
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        if (attachmentType == SwapchainAttachmentType::ColorDepthStencil) {
            depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        }
        result.resize(imageCount);
        result =
            initImageByType(VC, ImageType::Depth,
                windowSize.x,
                windowSize.y,
                imageCount,
                VK_SAMPLE_COUNT_1_BIT,
                depthFormat);
		return result;
    }
    void CleanupImages(VulkanCore* vulkanCore, std::vector<VulkanImage> images)
    {
        for (auto& imageView : images) {
            if (imageView.view != VK_NULL_HANDLE)
                vkDestroyImageView(vulkanCore->vkDevice, imageView.view, nullptr);
        }
        images.clear();
    }
}