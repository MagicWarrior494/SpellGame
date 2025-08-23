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

	inline int WindowCount() { return windows->size(); }

	void CreateNewWindow(std::string title);

	inline std::shared_ptr<std::map<int, Window>> GetWindows() { return windows; }

private:
	std::shared_ptr<Vulkan::VulkanContext> vulkanContext;
	std::shared_ptr<std::map<int, Window>> windows;
};