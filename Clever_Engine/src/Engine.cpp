#include "Engine.h"


#include <iostream>

#include "World/ECS/Components.h"

namespace Engine {

	Engine::Engine()
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
		 
		//Event::EventSystem ES{};

		renderingController.SetUp();

		uint8_t windowId = renderingController.CreateNewWindow("Main Window", 960, 540);

		SceneCreationInfo info{ windowId, 960, 540, 0, 0 };

		CameraScene sceneId = sceneController.CreateNewScene<CameraScene>(renderingController, info, 0);

		this->worldController.AddTriangle();

		/*eventController.RegisterFunction(KeySet{ Keyboard::KEY_N },
			EventAction(
				[this](Window& window) {
					int id = this->renderingController.CreateNewWindow("NewWindow", 540, 960);
					this->worldController.AddTriangle();
					},
				1000
			)
		);*/

		eventController.RegisterFunction(KeySet{ Keyboard::KEY_T },
			EventAction(
				[this](Window& window) {
					this->worldController.AddTriangle();
				},
				200
			)
		);

		/*eventController.RegisterFunction(KeySet{ Keyboard::KEY_S },
			EventAction(
				[this](Window& window) {
					SceneCreationInfo info{ windowId, 540, 960, 0, 0 };
					sceneController.CreateNewScene<CameraScene>(renderingController, info);
					window.CreateScene();
					window.addTriangle();
					window.resizeScenes();
				},
				500
			)
		);*/

		while (true)
		{
			eventController.Update(renderingController.GetAllWindows());
			worldController.Update();
			sceneController.Update();
			renderingController.Update();

			if (renderingController.GetWindowCount() == 0)
				break;

			renderingController.Render(worldController.GetRegistry());
		}
	}

	void Engine::Terminate()
	{

	}
}