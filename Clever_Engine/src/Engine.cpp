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

		uint8_t windowId = renderingController.CreateNewWindow("Main Window", 1000, 540);

		CameraScene& scene1 = renderingController.GetWindow(windowId).CreateNewScene<CameraScene>(worldController.GetRegistry(), 960/2, 540, 0, 0);
		CameraScene& scene2 = renderingController.GetWindow(windowId).CreateNewScene<CameraScene>(worldController.GetRegistry(), 960 / 2, 540, (960 / 2) + 10, 0);

		Registry& reg = worldController.GetRegistry();

		uint32_t camera1_id = reg.CreateEntity();
		reg.AddComponent<Camera>(camera1_id);

		uint32_t camera2_id = reg.CreateEntity();
		reg.AddComponent<Camera>(camera2_id);

		scene1.SetCameraEntityID(camera1_id);
		scene2.SetCameraEntityID(camera2_id);

		this->worldController.AddTriangle();

		while (true)
		{
			renderingController.Update();
			if (renderingController.GetWindowCount() == 0)
				break;

			worldController.Update();
			renderingController.Render(worldController.GetRegistry());
		}
	}

	void Engine::Terminate()
	{

	}
}