#include "Engine.h"
#include <iostream>
#include "World/ECS/Components.h"

namespace Engine {

	Engine::Engine()
		:
		renderingController(RenderingController{}),
		worldController(WorldController{})
	{
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


		renderingController.SetUp();

		uint8_t windowId = renderingController.CreateNewWindow("Main Window", 960, 540);
		renderingController.GetWindow(windowId).CreateNewScene(960 / 2, 540, 0, 0);

		this->worldController.AddTriangle();

		while (true)
		{
			if (renderingController.GetWindowCount() == 0)
				break;

			worldController.Update();
			renderingController.Update();
			renderingController.Render(worldController.GetRegistry());
		}
	}

	void Engine::Terminate()
	{

	}
}