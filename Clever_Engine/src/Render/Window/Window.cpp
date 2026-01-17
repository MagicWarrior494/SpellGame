#include "Window.h"



Window::Window(std::shared_ptr<Vulkan::VulkanContext> vulkanContext, std::string title, int width, int height, int posx, int posy)
	: title(title), width(width), height(height), posx(posx), posy(posy)
{
	Vulkan::SurfaceFlags flags = defaultVulkanWindowFlags;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if ((flags & Vulkan::SurfaceFlags::Resizeable) != Vulkan::SurfaceFlags::None) {
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}
	else {
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	}
	if ((flags & Vulkan::SurfaceFlags::Fullscreenable) != Vulkan::SurfaceFlags::None) {
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
	}
	else {
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	p_GLFWWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	if ((flags & Vulkan::SurfaceFlags::Fullscreen) != Vulkan::SurfaceFlags::None) {
		glfwSetWindowMonitor(p_GLFWWindow, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
	}
	else {
		if (posx == 0 && posy == 0)
		{
			posx = mode->width / 4;
			posy = mode->height / 4;
		}
		glfwSetWindowMonitor(p_GLFWWindow, nullptr, posx, posy, width, height, GLFW_DONT_CARE);
	}

	glfwShowWindow(p_GLFWWindow);

	WindowID = vulkanContext->CreateNewWindow(defaultVulkanWindowFlags);

	glfwSetWindowUserPointer(p_GLFWWindow, this);

	glfwSetFramebufferSizeCallback(p_GLFWWindow,
		[](GLFWwindow* window, int width, int height) {
			// Retrieve the SurfaceData pointer from the user pointer
			
			Window* sd = static_cast<Window*>(glfwGetWindowUserPointer(window));
			if (sd) {
				sd->GetVulkanWindow()->needsToBeRecreated = true;

				// Optional: you can update stored window size here if you want
				sd->width = width;
				sd->height = height;
			};
		}
	);

	glfwSetKeyCallback(p_GLFWWindow, key_callback);
	glfwSetMouseButtonCallback(p_GLFWWindow, mouseButton_callback);

	v_VulkanWindow = vulkanContext->GetWindow(WindowID);
}

bool Window::IsWindowStillValid()
{
	if (GetVulkanWindow() == nullptr)
	{
		return false;
	}
	return true;
}

void Window::InitWindow()
{
	if (IsWindowStillValid())
		GetVulkanWindow()->InitWindow(p_GLFWWindow);
}

void Window::Render()
{
	if (IsWindowStillValid())
		GetVulkanWindow()->RenderScenes();
}

void Window::CloseWindow()
{
	GetVulkanWindow()->CloseWindow();
	if (p_GLFWWindow) {
		glfwDestroyWindow(p_GLFWWindow);
		p_GLFWWindow = nullptr;
	}
}

void Window::AddChildRenderSurface(uint8_t renderSurfaceID)
{
	childrenRenderSurfaces.push_back(renderSurfaceID);
}

uint8_t Window::CreateNewRenderSurface(uint32_t width, uint32_t height, int posx, int posy)
{
	uint8_t sceneID = GetVulkanWindow()->CreateNewScene(width, height, posx, posy);
	childrenRenderSurfaces.push_back(sceneID);
	return sceneID;
}
