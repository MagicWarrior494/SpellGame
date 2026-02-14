#pragma once
#include <map>
#include <memory>
#include <GLFW/glfw3.h>
#include "Render/Window/Window.h"
#include "Render/Window/Scene.h"
#include "Graphics/GraphicsAPI.h"

#define VULKAN 1

#ifdef VULKAN

#include "Graphics/VulkanGrahicsAPI.h"

#elif defined(OPENGL)

#endif

class Renderer
{
public:
	Renderer()
	{
		glfwInit();
		#ifdef VULKAN
			m_graphicsAPI = std::make_unique<VulkanGraphicsAPI>();
		#elif defined(OPENGL)
				// Initialize OpenGL GraphicsAPI here
		#endif
	};
	~Renderer() = default;
	
	void Update();
	
	int NewWindow(const std::string& title, int width, int height);
	void CloseWindow(Window& window);
	void ResizeWindow(Window& window, int width, int height);
	bool IsWindowAlive(int windowId) {
		return m_windows.find(windowId) != m_windows.end();
	}

	Window& GetWindow(int windowId);

	int NewScene(int windowId, int width, int height);
	void MoveScene(int sceneId, int x, int y);
	void ResizeScene(int sceneId, int width, int height);
	void DeleteScene(int sceneId);

private:
	std::unique_ptr<GraphicsAPI> m_graphicsAPI;
	std::map<int, std::unique_ptr<Window>> m_windows;
	std::map<int, std::unique_ptr<Scene>> m_scenes;
};

