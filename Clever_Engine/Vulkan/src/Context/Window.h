#pragma once

#include "ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"
#include "Objects/Vertex.h"

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <random>
#include <unordered_map>

namespace Vulkan {
	class Window {
	public:
		VulkanSurface vulkanSurface;
		std::map<uint8_t, std::shared_ptr<VulkanScene>> vulkanScenes;
		SurfaceFlags flags;
		std::shared_ptr<VulkanCore> vulkanCore;

		uint8_t surfaceId;

		bool needsToBeRecreated = false;

		Window(std::shared_ptr<VulkanCore> core, SurfaceFlags flags, uint8_t id);

		void InitWindow(GLFWwindow* glfwWindowptr);  // Initialize window and OpenGL context
		void CloseWindow();                          // Close window and unload OpenGL context

		void RenderScenes();

		void ResizeScene(uint8_t sceneID, uint32_t newWidth, uint32_t newHeight);
		void MoveScene(uint8_t sceneID, uint32_t newX, uint32_t newY);

		uint8_t CreateNewScene(uint32_t width = 0, uint32_t height = 0, uint32_t posx = 0, uint32_t posy = 0);
		inline uint32_t GetNextSceneID() {
			return nextSceneID++;
		}


	public:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
	private:
		uint8_t nextSceneID = 0;
		uint8_t MAX_SCENE_COUNT = 16;
	};
}