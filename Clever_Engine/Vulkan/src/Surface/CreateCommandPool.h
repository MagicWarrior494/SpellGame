#pragma once
#include "Context/ContextVulkanData.h"
#include <stdexcept>

namespace Vulkan 
{
    inline VkCommandPool CreateCommandPool(std::shared_ptr<VulkanCore> vulkanCore)
    {
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = vulkanCore->d_PhysicalDeviceData.graphicsIndex.value();

        VkCommandPool commandPool{};
        if (vkCreateCommandPool(vulkanCore->vkDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Command Pool!");
        }

        return commandPool;
    }
}