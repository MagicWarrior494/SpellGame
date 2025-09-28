#pragma once
#include "Context/ContextVulkanData.h"
#include "SurfaceFlags.h"

namespace Vulkan {
	inline void CreateFrameBuffers(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface, SurfaceFlags flags)
	{
		VulkanCore& vulkanCore = *VC;
		vulkanSurface.surfaceVkFrameBuffers.resize(vulkanSurface.surfaceSwapchainImages.size());
		for (int i = 0; i < vulkanSurface.surfaceSwapchainImages.size(); i++)
		{
			VkImageView& imageView = vulkanSurface.surfaceSwapchainImages[i].view;
			std::vector<VkImageView> attachments;

			attachments.push_back(imageView);
			if ((flags & SurfaceFlags::EnableDepth) != SurfaceFlags::None)
			{
				attachments.push_back(vulkanSurface.surfaceDepthImages[i].view);
			}

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = vulkanSurface.vkRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = vulkanSurface.windowSize.x;
			framebufferInfo.height = vulkanSurface.windowSize.y;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(vulkanCore.vkDevice, &framebufferInfo, nullptr, &vulkanSurface.surfaceVkFrameBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Could not create Framebuffer!");
			}
		}
	}
	inline void CleanUpFrameBuffers(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
	{
		VulkanCore& vulkanCore = *VC;
		for (auto framebuffer : vulkanSurface.surfaceVkFrameBuffers) {
			vkDestroyFramebuffer(vulkanCore.vkDevice, framebuffer, nullptr);
		}
		vulkanSurface.surfaceVkFrameBuffers.clear();
	}
}