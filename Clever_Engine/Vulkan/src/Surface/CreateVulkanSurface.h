#pragma once
#include "Context/ContextVulkanData.h"
#include <stdexcept>

namespace Vulkan {

    inline void CreateVulkanRenderSurface(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface, SurfaceFlags flags)
	{
        VulkanCore& vulkanCore = *VC;
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		
		if ((flags & SurfaceFlags::Resizeable) != SurfaceFlags::None) {
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		}
		else {
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		}
		if ((flags & SurfaceFlags::Fullscreenable) != SurfaceFlags::None) {
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		}
		else {
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
		}

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		

		vulkanSurface.p_GLFWWindow = glfwCreateWindow(vulkanSurface.windowSize.x, vulkanSurface.windowSize.y, vulkanSurface.windowTitle.c_str(), nullptr, nullptr);
        
		if ((flags & SurfaceFlags::Fullscreen) != SurfaceFlags::None) {
            glfwSetWindowMonitor(vulkanSurface.p_GLFWWindow, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
		}
		else {
            glfwSetWindowMonitor(vulkanSurface.p_GLFWWindow, nullptr, vulkanSurface.windowPos.x, vulkanSurface.windowPos.y, vulkanSurface.windowSize.x, vulkanSurface.windowSize.y, GLFW_DONT_CARE);
		}
        
        

		if (!vulkanSurface.p_GLFWWindow)
		{
			throw std::runtime_error("failed to create GLFW window!");
		}

        glfwShowWindow(vulkanSurface.p_GLFWWindow);

		if (glfwCreateWindowSurface(vulkanCore.vkInstance, vulkanSurface.p_GLFWWindow, nullptr, &vulkanSurface.vkSurface))
		{
			throw std::runtime_error("Failed to create Window Surface");
		}
        glfwSetWindowPos(vulkanSurface.p_GLFWWindow, 500, 500);
	}

    inline void CloseVulkanRenderSurface(std::shared_ptr<VulkanCore> VC, VulkanSurface& vulkanSurface)
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

        // Destroy GLFW window
        if (vulkanSurface.p_GLFWWindow) {
            glfwDestroyWindow(vulkanSurface.p_GLFWWindow);
            vulkanSurface.p_GLFWWindow = nullptr;
        }
	}
}