#pragma once
#include "Context/ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"

namespace Vulkan {
    inline void CreateCommandBuffers(
        std::shared_ptr<VulkanCore> vulkanCore,
        VkCommandPool commandPool,
        uint32_t bufferCount,
        std::vector<VkCommandBuffer>& outCommandBuffers)
    {
        outCommandBuffers.resize(bufferCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = bufferCount;

        if (vkAllocateCommandBuffers(vulkanCore->vkDevice, &allocInfo, outCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Command Buffers!");
        }
    }
}