#pragma once
#include "Context/ContextVulkanData.h"
#include "SurfaceFlags.h"

namespace Vulkan {
    inline void CreateFrameBuffers(
        std::shared_ptr<VulkanCore> vulkanCore,
        VkRenderPass renderPass,
        const std::vector<VulkanImage>& colorImages,
        const std::vector<VulkanImage>& depthImages,
        std::vector<VkFramebuffer>& outFramebuffers,
        uint32_t width,
        uint32_t height)
    {
        outFramebuffers.clear();
        size_t count = std::max(colorImages.size(), size_t(1));

        for (size_t i = 0; i < count; ++i)
        {
            std::vector<VkImageView> attachments;

            // Color attachment
            if (!colorImages.empty())
                attachments.push_back(colorImages[i].view);

            // Depth attachment
            if (!depthImages.empty())
                attachments.push_back(depthImages[i].view);

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            VkFramebuffer framebuffer;
            if (vkCreateFramebuffer(vulkanCore->vkDevice, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer!");

            outFramebuffers.push_back(framebuffer);
        }
    }
}