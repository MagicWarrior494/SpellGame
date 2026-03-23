#pragma once
#include <vulkan/vulkan.h>
#include <glm.hpp>

#include "CoreTypes/VulkanCore.h"
#include "Flags.h"
#include "CoreTypes/VulkanContructors/Swapchain.h"
#include "CoreTypes/VulkanContructors/FrameBuffer.h"

namespace Vulkan
{
    void RecreateWindowResources(
        VulkanCore* VC,
        SwapChainCreateInfo createInfo,
        VkSwapchainKHR& swapChain,
		VkRenderPass& renderPass,
        std::vector<VkFramebuffer>& framebuffers,
        std::vector<VulkanImage>& swapChainImages,
        std::vector<VulkanImage>& depthImages,
        SwapchainAttachmentType attachmentType)
    {
        VulkanCore& vulkanCore = *VC;

        WaitForValidFramebufferSize(createInfo.windowPtr, createInfo.windowSize);
        vkDeviceWaitIdle(vulkanCore.vkDevice);

		CleanupFramebuffers(VC, framebuffers);
        CleanupImages(VC, swapChainImages);
        CleanupImages(VC, depthImages);

        if (swapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vulkanCore.vkDevice, swapChain, nullptr);
            swapChain = VK_NULL_HANDLE;
        }

        CreateSwapchain(VC, createInfo, swapChain);

        swapChainImages = CreateSwapchainImages(
            VC,
            swapChain,
            createInfo.swapChainImageFormat,
            glm::ivec2(createInfo.windowSize),
            attachmentType
        );

        if ((attachmentType != SwapchainAttachmentType::ColorOnly))
            depthImages = CreateDepthImages(
                VC,
                static_cast<uint32_t>(swapChainImages.size()),
                glm::ivec2(createInfo.windowSize),
                attachmentType
            );

        framebuffers = CreateFrameBuffers(
            VC,
            renderPass,
            swapChainImages,
            depthImages,
            createInfo.windowSize.x,
            createInfo.windowSize.y
		);
    }
}