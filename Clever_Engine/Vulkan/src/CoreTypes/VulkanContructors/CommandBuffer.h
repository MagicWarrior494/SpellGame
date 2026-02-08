#pragma once
#include "CoreTypes/VulkanCore.h"
#include <vector>
#include <stdexcept>

namespace Vulkan {
    inline VkCommandBuffer CreateCommandBuffer(
        VulkanCore* vulkanCore)
    {
        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = vulkanCore->vkCoreCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(vulkanCore->vkDevice, &allocInfo, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Command Buffer!");
        }

        return buffer;
    }

    inline std::vector<VkCommandBuffer> CreateCommandBuffers(
        VulkanCore* vulkanCore,
        VkCommandPool commandPool,
        uint32_t count)
    {
        std::vector<VkCommandBuffer> buffers(count, VK_NULL_HANDLE);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;
        if (vkAllocateCommandBuffers(vulkanCore->vkDevice, &allocInfo, buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate Command Buffers!");
        }
        return buffers;
	}

    inline VkCommandBuffer BeginSingleTimeCommands(VulkanCore* VC)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = VC->vkCoreCommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(VC->vkDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    inline void EndSingleTimeCommands(VulkanCore* VC, VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(VC->vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(VC->vkGraphicsQueue);

        vkFreeCommandBuffers(VC->vkDevice, VC->vkCoreCommandPool, 1, &commandBuffer);
    }
}