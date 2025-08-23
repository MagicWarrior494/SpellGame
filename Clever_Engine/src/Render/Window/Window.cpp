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
		GetVulkanWindow().InitWindow(width, height, title, posx, posy);
}

KeySet Window::GetWindowInputs()
{
	std::vector<Vulkan::KeySet> keysets = GetVulkanWindow().GetKeySet();
	KeySet new_keysets;
	for (int i = 0; i < keysets.size(); i++)
	{

		for (auto& key : keysets[i].keys)
		{
			new_keysets.keys.push_back((InputCodes::Keyboard)keyboardGLFWtoCleverKeyCodes[key]);
		}

		for (auto& mouseButton : keysets[i].mouseButtons)
		{
			new_keysets.mouseButtons.push_back((InputCodes::Mouse)mouseGLFWtoCleverKeyCodes[mouseButton]);
		}
	}
	return new_keysets;
}

std::string Window::GetWindowTitle()
{
	return GetVulkanWindow().title;
}

Vulkan::Window& Window::GetVulkanWindow()
{
	return *vulkanContext->GetWindow(vulkanWindowId);
}