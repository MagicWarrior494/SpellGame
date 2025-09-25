#include "WindowManager.h"

WindowManager::WindowManager(std::shared_ptr<Vulkan::VulkanContext> vulkanContext)
	: vulkanContext(vulkanContext)
{
}

void WindowManager::SetUp()
{
}


void WindowManager::Update()
{
	for (auto& [id, window] : windows)
	{
		if (window->WindowShouldClose()) {
			window->CloseWindow();
		}
	}

	for (auto it = windows.begin(); it != windows.end(); )
	{
		Window& window = *(it->second);
		if (window.IsWindowStillValid() == false)
		{
			it = windows.erase(it);
		}
		else {
			it++;
		}
	}
	vulkanContext->Update();
}

void WindowManager::CreateNewWindow(std::string title, int width, int height, int posx, int posy)
{
	std::unique_ptr<Window> window = std::make_unique<Window>(vulkanContext, title, width, height, posx, posy);
	window->InitWindow();
	windows.insert({ window->GetVulkanContextWindowId(), std::move(window)});
}

void WindowManager::RenderAllWindows()
{
	for (auto it = windows.begin(); it != windows.end(); it++)
	{
		Window& window = *(it->second);
		vulkanContext->RenderWindow(window.GetVulkanWindow());
	}
}

