#pragma once
#include "Context/ContextVulkanData.h"
#include <stdexcept>

namespace Vulkan {

    inline void CreateVulkanRenderSurface(std::shared_ptr<VulkanCore> VC, GLFWwindow* p_GLFWWindow, VkSurfaceKHR& vulkanSurface)
	{
        VulkanCore& vulkanCore = *VC;

		if (!p_GLFWWindow)
		{
			throw std::runtime_error("failed to create GLFW window!");
		}

		if (glfwCreateWindowSurface(vulkanCore.vkInstance, p_GLFWWindow, nullptr, &vulkanSurface))
		{
			throw std::runtime_error("Failed to create Window Surface");
		}
	}
}