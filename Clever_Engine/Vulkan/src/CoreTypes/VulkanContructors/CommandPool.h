#pragma once
#include "CoreTypes/VulkanCore.h"
#include <stdexcept>

namespace Vulkan
{
    inline VkCommandPool CreateCommandPool(VulkanCore* vulkanCore)
    {
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = vulkanCore->physicalDeviceData.graphicsIndex.value();

        VkCommandPool commandPool{};
        if (vkCreateCommandPool(vulkanCore->vkDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Command Pool!");
        }

        return commandPool;
    }
}