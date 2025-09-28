#pragma once
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "Window.h"
#include "Context/VulkanContext.h"

class WindowManager {
public:
	WindowManager(std::shared_ptr<Vulkan::VulkanContext> vulkanContext);
	~WindowManager() = default;

	void Update();
	void SetUp();

	inline int WindowCount() { return windows.size(); }

	int CreateNewWindow(std::string title, int width, int height, int posx = 0, int posy = 0);

	void RenderAllWindows();

	Window& getWindow(int windowID)
	{
		return *(windows.at(windowID));
	}

	inline std::map<int, std::unique_ptr<Window>>& GetWindows() { return windows; }

private:
	std::shared_ptr<Vulkan::VulkanContext> vulkanContext;
	std::map<int, std::unique_ptr<Window>> windows;
};