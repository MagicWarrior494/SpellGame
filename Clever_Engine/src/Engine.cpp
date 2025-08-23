#include "Engine.h"


#include <iostream>



namespace Engine {

	Engine::Engine()
	{
		vulkanContext = std::make_shared<Vulkan::VulkanContext>();

		vulkanContext->Init();
		windowManager.SetUp(vulkanContext);
		eventController.Init(windowManager.GetWindows());
	}

	void Engine::SetUp(std::string setUpFilePath)
	{
		/*
		Load Setup file and use that data for setup
		Create Windows(s)
		Create Graphics Context 
		Start capturing events
		*/
	}

	void Engine::SetUp()
	{
		/*
		Create Windows(s)
		Create Graphics Context
		Start capturing events
		*/
		 
		//Event::EventSystem ES{};

		windowManager.CreateNewWindow("Window1");
		windowManager.CreateNewWindow("Window2");

		while (true)
		{
			vulkanContext->Update();
			windowManager.Update();
			eventController.Update();

			vulkanContext->RenderAllWindows();
			

			if (windowManager.WindowCount() == 0)
				break;
		}
	}

	void Engine::Terminate()
	{

	}
}