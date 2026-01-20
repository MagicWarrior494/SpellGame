#pragma once
#include <cstdint>
#include <glm.hpp>
#include <stdexcept>
#include <memory>
#include <map>

#include "ContextVulkanData.h"
#include "Surface/SurfaceFlags.h"
#include "Window.h"
#include <chrono>

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

		//RETURNS NULLPTR if not found
		inline std::shared_ptr<Window> GetWindow(uint8_t id)
		{
			if (windows.find(id) != windows.end()) {
				return windows.at(id);
			}
			return nullptr;
		}

		VulkanBuffer& GetObjectMatrixBuffer()
		{
			return vulkanCore->persistentData.objectMatrixStorageBuffer;
		}

		size_t GetPhysicalDeviceMinAlignment()
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(vulkanCore->vkPhysicalDevice, &properties);
			size_t minAlignment = properties.limits.minUniformBufferOffsetAlignment;
			return minAlignment;
		}

		float GetDeltaTime()
		{
			return deltaTime;
		}

		float PrintFPS(float dt)
		{
			static float timer = 0.0f;
			static int frameCount = 0;
			static float lastFPS = 0.0f;

			timer += dt;
			frameCount++;

			// Update the FPS count every 1.0 second
			if (timer >= 1.0f)
			{
				lastFPS = (float)frameCount / timer;

				printf("FPS: %.2f  (%.2f ms)\n", lastFPS, (1000.0f / lastFPS));

				// Reset for the next second
				timer = 0.0f;
				frameCount = 0;
			}

			return lastFPS;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> lastFrame = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();

		float deltaTime;

		std::shared_ptr<VulkanCore> vulkanCore;
		std::map<uint8_t, std::shared_ptr<Window>> windows;
	};
}