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

		renderingController.GetWindow(windowId).CreateNewScene(worldController.GetRegistry(), 960, 540, 0, 0);

		Registry& reg = worldController.GetRegistry();
		uint32_t camera_id = reg.CreateEntity();

		Camera camera{};
		camera.fov = 45.0f;
		camera.nearPlane = 0.1f;
		camera.farPlane = 100.0f;
		camera.aspectRatio = (float)960 / (float)540;
		camera.position = glm::vec3(0.0f, 0.0f, 5.0f);

		reg.SetComponent<Camera>(camera_id, camera);

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