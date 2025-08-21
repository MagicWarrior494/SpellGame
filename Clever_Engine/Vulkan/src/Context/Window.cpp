#include "Window.h"

#include "Surface/CreateVulkanSurface.h"
#include "Surface/CreateSwapChain.h"
#include "Surface/CreateImageViews.h"
#include "Surface/CreateFrameBuffers.h"
#include "Surface/CreateRenderpass.h"
#include "Surface/CreateCommandBuffers.h"
#include "Surface/CreateSyncObjects.h"
#include "Surface/CreateImage.h"

namespace Vulkan {
	Window::Window(std::shared_ptr<VulkanCore> core, SurfaceFlags flags) : renderSurface{}, vulkanCore(std::move(core))
	{
		renderSurface.flags = flags;
	}

	void Window::InitWindow(int width, int height, std::string title, int posx, int posy)
	{
		VulkanSurface& vulkanSurface = renderSurface.vulkanSurface;
		vulkanSurface.windowSize = { width, height };
		vulkanSurface.windowPos = { posx, posy };
		vulkanSurface.windowTitle = title;

		if ((renderSurface.flags & SurfaceFlags::EnableTripleBuffer) != SurfaceFlags::None) {
			vulkanSurface.MAX_FRAMES_IN_FLIGHT = 3;
		}

		CreateVulkanRenderSurface(vulkanCore, vulkanSurface, renderSurface.flags);
		CreateSwapchain(vulkanCore, vulkanSurface, renderSurface.flags);
		CreateSwapchainImages(vulkanCore, vulkanSurface, renderSurface.flags);//Also makes DepthVulkanImages if Depth bit is set in flags
		CreateRenderPass(vulkanCore, vulkanSurface, renderSurface.flags);
		CreateFrameBuffers(vulkanCore, vulkanSurface, renderSurface.flags);
		CreateCommandBuffers(vulkanCore, vulkanSurface);
		CreateSyncObjects(vulkanCore, vulkanSurface);

		glfwSetWindowUserPointer(vulkanSurface.p_GLFWWindow, this);
		glfwSetFramebufferSizeCallback(vulkanSurface.p_GLFWWindow,
			[](GLFWwindow* window, int width, int height) {
				// Retrieve the SurfaceData pointer from the user pointer
				Window* sd = static_cast<Window*>(glfwGetWindowUserPointer(window));
				if (sd) {
					sd->renderSurface.resized = true;

					// Optional: you can update stored window size here if you want
					sd->renderSurface.vulkanSurface.windowSize = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
				}
			}
		);
		glfwSetKeyCallback(vulkanSurface.p_GLFWWindow, key_callback);
		glfwSetMouseButtonCallback(vulkanSurface.p_GLFWWindow, mouseButton_callback);
	}

	void Window::ClearKeySets()
	{
		keysets.clear();
	}
}