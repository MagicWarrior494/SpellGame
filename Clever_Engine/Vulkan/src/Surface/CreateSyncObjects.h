#pragma once
#include <vector>
#include <memory>
#include <stdexcept>
#include "Context/ContextVulkanData.h"

namespace Vulkan {
    inline void CreateSyncObjects(
        std::shared_ptr<VulkanCore> vulkanCore,
        size_t count,
        std::vector<VkSemaphore>& imageAvailableSemaphores,
        std::vector<VkSemaphore>& renderFinishedSemaphores,
        std::vector<VkFence>& inFlightFences)
    {
        imageAvailableSemaphores.resize(count);
        renderFinishedSemaphores.resize(count);
        inFlightFences.resize(count);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start signaled

        for (size_t i = 0; i < count; i++)
        {
            if (vkCreateSemaphore(vulkanCore->vkDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(vulkanCore->vkDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(vulkanCore->vkDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create synchronization objects!");
            }
        }
    }
}