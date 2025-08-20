#pragma once
#include "Context/ContextVulkanData.h"
#include <stdexcept>

namespace Vulkan 
{
	inline void CreateCommandPool(std::shared_ptr<VulkanCore> VC, uint8_t count = 1)
	{
		VulkanCore& vulkanCore = *VC;
		for (int i = 0; i < count; i++) {
			VkCommandPoolCreateInfo commandPoolInfo{};
			commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolInfo.queueFamilyIndex = vulkanCore.d_PhysicalDeviceData.graphicsIndex.value();

			VkCommandPool commandPool{};

			if (vkCreateCommandPool(vulkanCore.vkDevice, &commandPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
				throw std::runtime_error("failed to create Command Pool!");

			vulkanCore.VkCommandPools.push_back(commandPool);
		}
	}
}