#include "WindowManager.h"


void WindowManager::SetUp(std::shared_ptr<Vulkan::VulkanContext> vulkanContext) {
	this->vulkanContext = vulkanContext;
}

void WindowManager::Update()
{
	for (auto it = windows.begin(); it != windows.end(); )
	{
		Window& window = it->second;
		if (window.IsWindowStillValid() == false)
		{
			it = windows.erase(it);
		}
		else {
			it++;
		}
	}
}

void WindowManager::CreateNewWindow()
{
	Window window{vulkanContext};
	window.InitWindow(800, 600, "First Window!", 0, 0);
	windows.insert({ window.GetVulkanContextWindowId(), window});
}

