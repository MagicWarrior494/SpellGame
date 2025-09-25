#pragma once

#include "ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"

#include <vector>
#include <memory>
#include <string>

namespace Vulkan {
	class RenderSurface {
	public:
		VulkanSurface vulkanSurface;
		std::vector<VulkanScene> vulkanScene;
		SurfaceFlags flags;
		std::shared_ptr<VulkanCore> vulkanCore;

		bool needsToBeRecreated = false;

		RenderSurface(std::shared_ptr<VulkanCore> core, SurfaceFlags flags, uint8_t id);

		void InitRenderSurface(GLFWwindow* glfwWindowptr);  // Initialize window and OpenGL context
		void CloseRenderSurface();                                         // Close window and unload OpenGL context
		void RecreateSwapChain();

	public:
		uint8_t renderSurfaceID = 0;
	};
}