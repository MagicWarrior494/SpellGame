#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "CoreTypes/VulkanCore.h"
#include <GLFW/glfw3.h>

namespace Vulkan {

	void CreateVulkanSurface(VulkanCore* VC, void* windowPtr, VkSurfaceKHR& vulkanSurface)
	{
		VulkanCore& vulkanCore = *VC;

		if (!windowPtr)
		{
			throw std::runtime_error("failed to create GLFW window!");
		}

		if (glfwCreateWindowSurface(vulkanCore.vkInstance, (GLFWwindow*)windowPtr, nullptr, &vulkanSurface))
		{
			throw std::runtime_error("Failed to create Window Surface");
		}
	}
}