#pragma once
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <map>

#include "Window.h"
#include "Context/VulkanContext.h"

class WindowManager {
public:
	WindowManager() = default;
	~WindowManager() = default;

	void Update();
	void SetUp(std::shared_ptr<Vulkan::VulkanContext> vulkanContext);

	inline int WindowCount() { return windows.size(); }

	void CreateNewWindow();

private:
	std::shared_ptr<Vulkan::VulkanContext> vulkanContext;
	std::map<int, Window> windows;
};