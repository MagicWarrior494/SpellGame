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
		uint32_t sceneIDCounter = 0;

		VulkanSurface vulkanSurface;
		std::vector<std::shared_ptr<VulkanScene>> vulkanScenes;
		SurfaceFlags flags;
		std::shared_ptr<VulkanCore> vulkanCore;

		uint8_t surfaceId;

		bool needsToBeRecreated = false;

		RenderSurface(std::shared_ptr<VulkanCore> core, SurfaceFlags flags, uint8_t id);

		void InitRenderSurface(GLFWwindow* glfwWindowptr);  // Initialize window and OpenGL context
		void CloseRenderSurface();                          // Close window and unload OpenGL context

		void AddRandomTriangle();
		void Render();

		void resizeScenes();
		uint8_t CreateNewScene(uint32_t width = 0, uint32_t height = 0, uint32_t posx = 0, uint32_t posy = 0);
		inline uint32_t GetNextSceneID() {
			return sceneIDCounter++;
		}

	public:
		RenderSurface(const RenderSurface&) = delete;
		RenderSurface& operator=(const RenderSurface&) = delete;
	};
}