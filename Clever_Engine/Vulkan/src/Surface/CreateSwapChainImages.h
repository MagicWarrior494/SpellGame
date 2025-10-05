#pragma once
#include "Context/ContextVulkanData.h"
#include "SurfaceFlags.h"
#include "CreateImage.h"
#include "CreateFrameBuffers.h"

namespace Vulkan {

    inline void CreateSwapchainImages(
        std::shared_ptr<VulkanCore> VC,
        VulkanSurface* vulkanSurface,
        SwapchainAttachmentType attachmentType)
    {
        VulkanCore& vulkanCore = *VC;
        VkDevice device = vulkanCore.vkDevice;

        // --- Clear previous resources ---
        for (auto& img : vulkanSurface->surfaceColorImages) {
            if (img.view) {
                vkDestroyImageView(device, img.view, nullptr);
                img.view = VK_NULL_HANDLE;
            }
            img = VulkanImage{};
        }
        vulkanSurface->surfaceColorImages.clear();

        for (auto fb : vulkanSurface->surfaceFrameBuffers) {
            if (fb != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(device, fb, nullptr);
            }
        }

        for (auto& depth : vulkanSurface->surfaceDepthImages) {
            if (depth.view) {
                vkDestroyImageView(device, depth.view, nullptr);
            }
            if (depth.image) {
                vkDestroyImage(device, depth.image, nullptr);
            }
            if (depth.memory) {
                vkFreeMemory(device, depth.memory, nullptr);
            }
        }
        vulkanSurface->surfaceDepthImages.clear();

        // --- Get swapchain images ---
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(device, vulkanSurface->surfaceSwapChain, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(device, vulkanSurface->surfaceSwapChain, &imageCount, images.data());

        vulkanSurface->surfaceColorImages.resize(imageCount);

        for (uint32_t i = 0; i < imageCount; ++i) {
            vulkanSurface->surfaceColorImages[i].image = images[i];
            vulkanSurface->surfaceColorImages[i].memory = VK_NULL_HANDLE; // Owned by swapchain
            vulkanSurface->surfaceColorImages[i].view = VK_NULL_HANDLE;
            vulkanSurface->surfaceColorImages[i].format = vulkanSurface->surfaceswapChainImageFormat;
            vulkanSurface->surfaceColorImages[i].extent = {
                vulkanSurface->windowSize.x,
                vulkanSurface->windowSize.y,
                1
            };
        }

        // --- Create image views for swapchain images ---
        for (uint32_t i = 0; i < imageCount; ++i) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = vulkanSurface->surfaceColorImages[i].image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = vulkanSurface->surfaceswapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewInfo, nullptr,
                &vulkanSurface->surfaceColorImages[i].view) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create swapchain image view");
            }
        }

        // --- Optional Depth/Stencil attachment ---
        bool makeDepth = (attachmentType != SwapchainAttachmentType::ColorOnly);
        if (makeDepth) {
            VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
            if (attachmentType == SwapchainAttachmentType::ColorDepthStencil) {
                depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
            }

            vulkanSurface->surfaceDepthImages =
                initImageByType(vulkanCore, ImageType::Depth,
                    vulkanSurface->windowSize.x,
                    vulkanSurface->windowSize.y,
                    imageCount,
                    VK_SAMPLE_COUNT_1_BIT,
                    depthFormat);
        }
    }

	inline void CleanupSwapchainImages(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
	{
		VulkanCore& vulkanCore = *VC;
		for (auto& imageView : vulkanSurface.surfaceColorImages) {
			if (imageView.view != VK_NULL_HANDLE)
				vkDestroyImageView(vulkanCore.vkDevice, imageView.view, nullptr);
		}
		for (auto& imageView : vulkanSurface.surfaceDepthImages) {
			if (imageView.view != VK_NULL_HANDLE)
				vkDestroyImageView(vulkanCore.vkDevice, imageView.view, nullptr);
		}
		vulkanSurface.surfaceDepthImages.clear();
		vulkanSurface.surfaceColorImages.clear();
	}
}