#pragma once
#include <cstdint>
#include <glm.hpp>
#include <stdexcept>
#include <memory>
#include <map>

#include "ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"
#include "Window.h"

namespace Vulkan 
{
	/*
	This is what "owns" the Vulkan Data. The Engine is what modify and does the function calling but this owns the data
	*/
	class VulkanContext 
	{
	public:
		VulkanContext() = default;
		void Init();
		void Update();

		uint8_t CreateNewWindow(SurfaceFlags flags);

		VkResult RenderWindow(std::shared_ptr<Window> window);
		inline void RenderAllWindows() {
			for (auto window : windows)
			{
				RenderWindow(window.second);
			}
		};
		
		inline std::shared_ptr<Window> GetWindow(uint8_t windowID) {
			if (windows.find(windowID) == windows.end()) {
				return nullptr;
			}
			return windows[windowID];
		}

	private:

		std::shared_ptr<VulkanCore> vulkanCore;
		std::map<uint8_t, std::shared_ptr<Window>> windows;
		uint8_t nextWindowID = 0;
	};
}