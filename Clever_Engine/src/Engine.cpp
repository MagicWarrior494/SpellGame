#include "Engine.h"


#include <iostream>



namespace Engine {

	Engine::Engine() :
		vulkanContext(std::make_shared<Vulkan::VulkanContext>()), windowManager(vulkanContext), eventController(windowManager.GetWindows())
	{
		vulkanContext->Init();
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

		windowManager.CreateNewWindow("Window1", 800, 600);
		windowManager.CreateNewWindow("Window2", 800, 600);

		eventController.RegisterFunction(KeySet{ Keyboard::KEY_N }, [this] {
			this->windowManager.CreateNewWindow("NewWindow", 800, 600);
		});

		while (true)
		{
			windowManager.Update();
			eventController.Update();

			windowManager.RenderAllWindows();
			

			if (windowManager.WindowCount() == 0)
				break;
		}
	}

	void Engine::Terminate()
	{

	}
}