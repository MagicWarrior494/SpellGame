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

		int windowID1 = windowManager.CreateNewWindow("Window1", 800, 600);
		windowManager.getWindow(windowID1).addTriangle();

		eventController.RegisterFunction(KeySet{ Keyboard::KEY_N }, [this] (Window& window) {
			int id = this->windowManager.CreateNewWindow("NewWindow", 800, 600);
		});

		eventController.RegisterFunction(KeySet{ Keyboard::KEY_T }, [this] (Window& window) {
			window.addTriangle();
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