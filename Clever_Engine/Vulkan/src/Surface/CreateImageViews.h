#pragma once
#include "Context/ContextVulkanData.h"
#include "SurfaceFlags.h"
#include "CreateImage.h"

namespace Vulkan {
	inline void CreateSwapchainImages(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface, SurfaceFlags flags)
	{
		VulkanCore& vulkanCore = *VC;
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(
			vulkanCore.vkDevice,
			vulkanSurface.vkSwapChain,
			&imageCount,
			nullptr
		);

		vulkanSurface.surfaceSwapchainImages.resize(imageCount);

		std::vector<VkImage> swapchainImages(imageCount);
		vkGetSwapchainImagesKHR(
			vulkanCore.vkDevice,
			vulkanSurface.vkSwapChain,
			&imageCount,
			swapchainImages.data()
		);

		for (uint8_t i = 0; i < imageCount; ++i) {
			vulkanSurface.surfaceSwapchainImages[i].image = swapchainImages[i];
			vulkanSurface.surfaceSwapchainImages[i].memory = VK_NULL_HANDLE;
			vulkanSurface.surfaceSwapchainImages[i].view = VK_NULL_HANDLE;
		}

		for (size_t i = 0; i < imageCount; ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = vulkanSurface.surfaceSwapchainImages[i].image;
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(vulkanCore.vkDevice, &createInfo, nullptr, &vulkanSurface.surfaceSwapchainImages[i].view) != VK_SUCCESS)
			{
				throw std::runtime_error("Unable to make Image View!");
			}
		}
		
		if ((flags & SurfaceFlags::EnableDepth) != SurfaceFlags::None) {
			VkFormat depthFormat = (flags & SurfaceFlags::EnableStencil) != SurfaceFlags::None
				? VK_FORMAT_D24_UNORM_S8_UINT
				: VK_FORMAT_D32_SFLOAT;

			vulkanSurface.surfaceDepthImages.resize(vulkanSurface.MAX_FRAMES_IN_FLIGHT);
			vulkanSurface.surfaceDepthImages = initImageByType(vulkanCore, ImageType::Depth, vulkanSurface.windowSize.x, vulkanSurface.windowSize.y, vulkanSurface.MAX_FRAMES_IN_FLIGHT);
		}
	}

	inline void CleanupSwapchainImages(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
	{
		VulkanCore& vulkanCore = *VC;
		for (auto& imageView : vulkanSurface.surfaceSwapchainImages) {
			if (imageView.view != VK_NULL_HANDLE)
				vkDestroyImageView(vulkanCore.vkDevice, imageView.view, nullptr);
		}
		for (auto& imageView : vulkanSurface.surfaceDepthImages) {
			if (imageView.view != VK_NULL_HANDLE)
				vkDestroyImageView(vulkanCore.vkDevice, imageView.view, nullptr);
		}
		vulkanSurface.surfaceDepthImages.clear();
		vulkanSurface.surfaceSwapchainImages.clear();
	}
}