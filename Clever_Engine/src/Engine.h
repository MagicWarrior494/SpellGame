#pragma once

#include "Context/VulkanContext.h"

#include "Render/Window/WindowManager.h"
#include "Event/EventController.h"

#include <stdexcept>
#include <string>

namespace Engine {

	class Engine
	{
	public:
		/*
		This is the INIT for the Engine, It takes a file path which contains JSON data about anything the engine would like
		to remember from run to run. Like How many windows, window positions, file locations for model and texture data,
		Optimization for different Vulkan object and stuff like that.
		This is not The WORLD SCENE DATA, nothing about the in world object themselves.
		*/
		Engine();

		void SetUp(std::string setUpFilePath);

		//Default Setup
		void SetUp();


		void Terminate();
	private:
		std::string m_SetUpFilePath;
		WindowManager windowManager;
		EventController eventController;
		std::shared_ptr<Vulkan::VulkanContext> vulkanContext;
	};
}