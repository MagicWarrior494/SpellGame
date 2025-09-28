#pragma once
#include "Context/ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"

namespace Vulkan {
	inline void CreateCommandBuffers(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
	{
		VulkanCore& vulkanCore = *VC;
		vulkanSurface.surfaceVkCommandBuffers.resize(VC->MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocationInfo{};
		allocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocationInfo.commandPool = vulkanCore.VkCommandPools[0];
		allocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocationInfo.commandBufferCount = static_cast<uint32_t>(vulkanSurface.surfaceVkCommandBuffers.size());

		if (vkAllocateCommandBuffers(vulkanCore.vkDevice, &allocationInfo, vulkanSurface.surfaceVkCommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Unable to allocate Command Buffers");
		}
	}
}