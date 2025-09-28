#pragma once

#include "ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"

#include <vector>
#include <memory>
#include <string>
#include <random>

namespace Vulkan {
	class RenderSurface {
	public:
		VulkanSurface vulkanSurface;
		std::vector<std::shared_ptr<VulkanScene>> vulkanScenes;
		SurfaceFlags flags;
		std::shared_ptr<VulkanCore> vulkanCore;

		bool needsToBeRecreated = false;

		RenderSurface(std::shared_ptr<VulkanCore> core, SurfaceFlags flags, uint8_t id);

		void InitRenderSurface(GLFWwindow* glfwWindowptr);  // Initialize window and OpenGL context
		void CloseRenderSurface();                                         // Close window and unload OpenGL context
		void RecreateSwapChain();

		void AddRandomTriangle(uint8_t sceneID);
		void RenderScene(uint8_t sceneID);

		uint8_t CreateNewScene(uint8_t width = 0, uint8_t height = 0, uint8_t posx = 0, uint8_t posy = 0);

	private:
		uint8_t renderSurfaceID = 0;
		uint8_t nextAvailableSceneID = 0;
	};
}