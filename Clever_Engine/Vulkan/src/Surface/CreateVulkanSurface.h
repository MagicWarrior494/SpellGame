#pragma once
#include "Context/ContextVulkanData.h"
#include <stdexcept>

namespace Vulkan {

    inline void CreateVulkanRenderSurface(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface, SurfaceFlags flags)
	{
        VulkanCore& vulkanCore = *VC;

		if (!vulkanSurface.p_GLFWWindow)
		{
			throw std::runtime_error("failed to create GLFW window!");
		}

        
		if (glfwCreateWindowSurface(vulkanCore.vkInstance, vulkanSurface.p_GLFWWindow, nullptr, &vulkanSurface.vkSurface))
		{
			throw std::runtime_error("Failed to create Window Surface");
		}
        
	}

    inline void CloseVulkanSurface(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
	{
        VulkanCore& vulkanCore = *VC;
		if (vulkanCore.vkDevice != VK_NULL_HANDLE) {
			vkDeviceWaitIdle(vulkanCore.vkDevice);
		}

        // Destroy semaphores
        for (auto semaphore : vulkanSurface.imageAvailableSemaphores) {
            if (semaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(vulkanCore.vkDevice, semaphore, nullptr);
            }
        }
        vulkanSurface.imageAvailableSemaphores.clear();

        for (auto semaphore : vulkanSurface.renderFinishedSemaphores) {
            if (semaphore != VK_NULL_HANDLE) {
                vkDestroySemaphore(vulkanCore.vkDevice, semaphore, nullptr);
            }
        }
        vulkanSurface.renderFinishedSemaphores.clear();

        // Destroy fences
        for (auto fence : vulkanSurface.surfaceVkFences) {
            if (fence != VK_NULL_HANDLE) {
                vkDestroyFence(vulkanCore.vkDevice, fence, nullptr);
            }
        }
        vulkanSurface.surfaceVkFences.clear();

        // Destroy framebuffers
        for (auto framebuffer : vulkanSurface.surfaceVkFrameBuffers) {
            if (framebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(vulkanCore.vkDevice, framebuffer, nullptr);
            }
        }
        vulkanSurface.surfaceVkFrameBuffers.clear();

        // Destroy render pass
        if (vulkanSurface.vkRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(vulkanCore.vkDevice, vulkanSurface.vkRenderPass, nullptr);
            vulkanSurface.vkRenderPass = VK_NULL_HANDLE;
        }

        // Destroy swapchain
        if (vulkanSurface.vkSwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vulkanCore.vkDevice, vulkanSurface.vkSwapChain, nullptr);
            vulkanSurface.vkSwapChain = VK_NULL_HANDLE;
        }

        // Destroy surface
        if (vulkanSurface.vkSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(vulkanCore.vkInstance, vulkanSurface.vkSurface, nullptr);
            vulkanSurface.vkSurface = VK_NULL_HANDLE;
        }
	}
}