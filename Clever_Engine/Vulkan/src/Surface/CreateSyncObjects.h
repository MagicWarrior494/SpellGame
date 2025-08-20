#pragma once

namespace Vulkan {
	inline void CreateSyncObjects(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
	{
		VulkanCore& vulkanCore = *VC;
		vulkanSurface.surfaceVkFences.resize(vulkanSurface.MAX_FRAMES_IN_FLIGHT);
		vulkanSurface.imageAvailableSemaphores.resize(vulkanSurface.MAX_FRAMES_IN_FLIGHT);
		vulkanSurface.renderFinishedSemaphores.resize(vulkanSurface.MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < vulkanSurface.MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(vulkanCore.vkDevice, &semaphoreInfo, nullptr, &vulkanSurface.imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(vulkanCore.vkDevice, &semaphoreInfo, nullptr, &vulkanSurface.renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(vulkanCore.vkDevice, &fenceInfo, nullptr, &vulkanSurface.surfaceVkFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create semaphores!");
			}
		}
	}
}