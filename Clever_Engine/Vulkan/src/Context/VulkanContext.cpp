#include "VulkanContext.h"
#include "Core/Instance.h"
#include "Core/LogicalDevice.h"

#include "Core/PhysicalDevice.h"
#include "Core/CommandPool.h"


#include "Surface/CreateVulkanSurface.h"
#include "Surface/CreateSwapChain.h"
#include "Surface/CreateImageViews.h"
#include "Surface/CreateFrameBuffers.h"
#include "Surface/CreateRenderpass.h"
#include "Surface/CreateCommandBuffers.h"
#include "Surface/CreateSyncObjects.h"
#include "Surface/CreateImage.h"
#include "Scene/CreateDescriptors.h"
#include "Scene/CreatePipelines.h"
#include "Buffers/CreateBuffer.h"

namespace Vulkan {
	void VulkanContext::Init() {
		vulkanCore = std::make_shared<VulkanCore>();

		CreateVulkanInstance(vulkanCore);
		CreatePhysicalDevice(vulkanCore);
		CreateLogicalDevice(vulkanCore);//This also makes the DebugUtilsMessengerEXT object and the graphics and present Queue
		CreateCommandPool(vulkanCore);
	}

	void VulkanContext::Update()
	{
		if (renderSurfaces.empty())
			return;

		//glfwPollEvents();
		//Remove renderSurface from list when the VulkanSurface OBJ within has been cleared.
		for (auto it = renderSurfaces.begin(); it != renderSurfaces.end();) {
			std::shared_ptr<RenderSurface> renderSurface = it->second;
			if (renderSurface->vulkanSurface.p_GLFWWindow == nullptr) {
				it = renderSurfaces.erase(it);
			}
			else {
				it++;
			}
		}
	}

	uint8_t VulkanContext::CreateNewWindow(SurfaceFlags flags)
	{
		std::shared_ptr<RenderSurface> renderSurface = std::make_shared<RenderSurface>(vulkanCore, flags, nextWindowID);
		renderSurfaces.insert({ nextWindowID, std::move(renderSurface) });
		nextWindowID++;
		return nextWindowID - 1;
	}
}