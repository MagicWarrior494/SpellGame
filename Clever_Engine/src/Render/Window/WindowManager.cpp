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

int WindowManager::CreateNewWindow(std::string title, int width, int height, int posx, int posy)
{
	std::unique_ptr<Window> window = std::make_unique<Window>(vulkanContext, title, width, height, posx, posy);
	window->InitWindow();
	window->CreateScene();

	int id = window->GetVulkanContextWindowId();

	windows.insert({ window->GetVulkanContextWindowId(), std::move(window)});

	return id;
}

void WindowManager::RenderAllWindows()
{
	for (auto it = windows.begin(); it != windows.end(); it++)
	{
		Window& window = *(it->second);
		window.Update();
	}
}

