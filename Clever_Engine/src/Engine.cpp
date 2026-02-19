#include "Engine.h"
#include <iostream>
#include "World/ECS/Components.h"

#include "Render/Graphics/VulkanGrahicsAPI.h"


Engine::Engine():
	m_WorldController(WorldController{}), m_AssetManager(AssetManager{})
{
	glfwInit();
	#ifdef VULKAN
			m_graphicsAPI = std::make_unique<VulkanGraphicsAPI>();
	#elif defined(OPENGL)
			// Initialize OpenGL GraphicsAPI here
	#endif
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
	m_WorldController.Update();
	for (auto& [id, window] : m_windows)
	{
		window->Update();
		window->Render();
	}
}

Window& Engine::CreateWindow(const std::string& title, int width, int height)
{
	int windowId = static_cast<int>(m_windows.size());
	auto window = std::make_unique<Window>(title, width, height);
	m_windows[windowId] = std::move(window);
	return *m_windows[windowId];
}

Scene& Engine::CreateScene(int windowId, int width, int height)
{
	int sceneId = static_cast<int>(m_scenes.size());
	auto scene = std::make_unique<Scene>(windowId, width, height);
	m_scenes[sceneId] = std::move(scene);
	return *m_scenes[sceneId];
}

void Engine::Terminate()
{

}