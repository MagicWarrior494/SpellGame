#include "Window.h"

bool Window::IsWindowStillValid()
{
	if (vulkanContext->GetWindow(vulkanWindowId) == nullptr)
	{
		return false;
	}
	return true;
}

int Window::GetVulkanContextWindowId()
{
	return vulkanWindowId;
}

void Window::InitWindow(int width, int height, std::string title, int posx = 0, int posy = 0)
{
	if(IsWindowStillValid())
		vulkanContext->GetWindow(vulkanWindowId)->InitWindow(width, height, title, posx, posy);
}