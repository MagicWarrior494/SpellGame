#pragma once
#include <map>
#include <memory>
#include <GLFW/glfw3.h>
#include "Render/Window/Window.h"
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
	
	Window& NewWindow(const std::string& title, int width, int height);
	void CloseWindow(Window& window);
	void ResizeWindow(Window& window, int width, int height);
	bool IsWindowAlive(int windowId) {
		return m_windows.find(windowId) != m_windows.end();
	}

	Window& GetWindow(int windowId);

private:
	std::unique_ptr<GraphicsAPI> m_graphicsAPI;
	std::map<int, std::unique_ptr<Window>> m_windows;
};

