#pragma once
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <GLFW/glfw3.h>
#include <unordered_map>

#include "Context/VulkanContext.h"
#include "Event/Io/KeySet.h"
#include "Event/Io/ConversionData.h"

#include "World/ECS/Components.h"
class Window
{
public:
	std::string title = "Default";
	int width = 100;
	int height = 100;
	int posx = 0;
	int posy = 0;

	KeySet keyset;
public:
	Window(std::shared_ptr<Vulkan::VulkanContext> vulkanContext, std::string title, int width, int height, int posx, int posy);
	~Window() = default;


	bool IsWindowStillValid();
	void InitWindow();  // Initialize window and OpenGL context
	void CloseWindow();                                         // Close window and unload OpenGL context

	void AddChildRenderSurface(uint8_t renderSurfaceID);

	void Render(std::unordered_map<uint32_t, Transform>&);

	uint8_t CreateNewRenderSurface(uint32_t width, uint32_t height, int posx = 0, int posy = 0);

	std::shared_ptr<Vulkan::Window> GetVulkanWindow()
	{
		return v_VulkanWindow;
	}

	int RenderSurfaceCount();

	inline void ClearKeySets() { keyset.clear(); }
	inline KeySet GetKeySet() { return keyset; }
	uint8_t GetWindowID() { return WindowID; }
private:
	uint8_t WindowID = 0;//Same ID as in VulkanContext because there is a 1:1 mapping between Window and RenderSurface in VulkanContext
	std::vector<uint8_t> childrenRenderSurfaces{};

	GLFWwindow* p_GLFWWindow;
	std::shared_ptr<Vulkan::Window> v_VulkanWindow;
	Vulkan::SurfaceFlags defaultVulkanWindowFlags = Vulkan::SurfaceFlags::Resizeable | Vulkan::SurfaceFlags::Fullscreenable;
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));

	windowObj->keyset.keys.push_back((InputCodes::Keyboard)keyboardGLFWtoCleverKeyCodes.at(key));

}

static void mouseButton_callback(GLFWwindow* window, int mouseButton, int action, int mods)
{
	Window* windowObj = static_cast<Window*>(glfwGetWindowUserPointer(window));
	windowObj->keyset.mouseButtons.push_back((InputCodes::Mouse)mouseGLFWtoCleverKeyCodes.at(mouseButton));
}