#include "RenderSurface.h"

#include "Surface/CreateVulkanSurface.h"
#include "Surface/CreateSwapChain.h"
#include "Surface/CreateImageViews.h"
#include "Surface/CreateFrameBuffers.h"
#include "Surface/CreateRenderpass.h"
#include "Surface/CreateCommandBuffers.h"
#include "Surface/CreateSyncObjects.h"
#include "Surface/CreateImage.h"

namespace Vulkan {
	RenderSurface::RenderSurface(std::shared_ptr<VulkanCore> core, SurfaceFlags flags, uint8_t id) : 
		vulkanCore(std::move(core)), flags(flags), renderSurfaceID(id)
	{
		//vulkanSurface = VulkanSurface{};
	}

	void RenderSurface::InitRenderSurface(GLFWwindow* glfwWindowptr)
	{
		vulkanSurface.p_GLFWWindow = glfwWindowptr;

		if ((flags & SurfaceFlags::EnableTripleBuffer) != SurfaceFlags::None) {
			vulkanSurface.MAX_FRAMES_IN_FLIGHT = 3;
		}

		CreateVulkanRenderSurface(vulkanCore, vulkanSurface, flags);
		CreateSwapchain(vulkanCore, vulkanSurface, flags);
		CreateSwapchainImages(vulkanCore, vulkanSurface, flags);//Also makes DepthVulkanImages if Depth bit is set in flags
		CreateRenderPass(vulkanCore, vulkanSurface, flags);
		CreateFrameBuffers(vulkanCore, vulkanSurface, flags);
		CreateCommandBuffers(vulkanCore, vulkanSurface);
		CreateSyncObjects(vulkanCore, vulkanSurface);

		if (vulkanScene.size() > 0) return;
		//CREATING FIRST SCENE OF RENDERSURFACE, might want to make a way to create a new rendersurface without making a new scene



	}
	void RenderSurface::CloseRenderSurface()
	{
		CloseVulkanSurface(vulkanCore, vulkanSurface);
	}
	void RenderSurface::RecreateSwapChain()
	{
		//glfwPollEvents();
		int width = 0, height = 0;
		glfwGetFramebufferSize(vulkanSurface.p_GLFWWindow, &width, &height);

		if (width == 0 || height == 0)
		{
			//Because window is still minimized and shouldnt be processed.
			return;
		}

		vkDeviceWaitIdle(vulkanCore->vkDevice);
		CleanUpFrameBuffers(vulkanCore, vulkanSurface);
		CleanupSwapchainImages(vulkanCore, vulkanSurface);
		CleanupSwapchain(vulkanCore, vulkanSurface);
		CreateSwapchain(vulkanCore, vulkanSurface, flags);
		CreateSwapchainImages(vulkanCore, vulkanSurface, flags);
		CreateFrameBuffers(vulkanCore, vulkanSurface, flags);
		needsToBeRecreated = false;
	}
}