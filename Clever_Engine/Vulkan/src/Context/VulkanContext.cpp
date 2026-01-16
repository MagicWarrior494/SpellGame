#include "VulkanContext.h"
#include "Core/Instance.h"
#include "Core/LogicalDevice.h"

#include "Core/PhysicalDevice.h"

#include "Surface/CreateCommandPool.h"
#include "Surface/CreateCommandBuffers.h"
#include "Surface/CreateVulkanSurface.h"
#include "Surface/CreateSwapChain.h"
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
		vulkanCore->coreCommandPool = CreateCommandPool(vulkanCore);
		std::vector<VkCommandBuffer> dummy;
		dummy.push_back(vulkanCore->coreCommandBuffer);
		CreateCommandBuffers(vulkanCore, vulkanCore->coreCommandPool, 1, dummy);
	}

	void VulkanContext::Update()
	{
		if (windows.empty())
			return;

		//glfwPollEvents();
		//Remove renderSurface from list when the VulkanSurface OBJ within has been cleared.
		for (auto it = windows.begin(); it != windows.end();) {
			std::shared_ptr<Window> renderSurface = it->second;
			if (renderSurface->vulkanSurface.p_GLFWWindow == nullptr) {
				it = windows.erase(it);
			}
			else {
				it++;
			}
		}
	}

	uint8_t VulkanContext::CreateNewWindow(SurfaceFlags flags)
	{
		std::shared_ptr<Window> renderSurface = std::make_shared<Window>(vulkanCore, flags, GetNextSurfaceID());
		uint8_t id = renderSurface->surfaceId;
		windows.insert({ renderSurface->surfaceId, std::move(renderSurface) });
		
		return id;
	}
}