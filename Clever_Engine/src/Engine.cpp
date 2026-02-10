#include "Engine.h"
#include <iostream>
#include "World/ECS/Components.h"

namespace Engine {

	Engine::Engine()
		:
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
	}

	void Engine::Tick()
	{
		//Update World
		worldController.Update();
		m_renderer.Update();
		//Render World
		//renderingController.Render(worldController.GetRegistry());
	}

	void Engine::Terminate()
	{

	}
}