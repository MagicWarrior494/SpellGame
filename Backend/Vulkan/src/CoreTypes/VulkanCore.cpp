#include "VulkanCore.h"
#include "VulkanContructors/Instance.h"
#include "VulkanContructors/LogicalDevice.h"
#include "VulkanContructors/PhysicalDevice.h"
#include "VulkanContructors/CommandPool.h"
#include "VulkanContructors/CommandBuffer.h"

namespace Vulkan
{
	VulkanCore::VulkanCore()
	{
		CreateVulkanInstance(this);
		CreatePhysicalDevice(this);
		CreateLogicalDevice(this);//This also makes the DebugUtilsMessengerEXT object and the graphics and present Queue
		vkCoreCommandPool = CreateCommandPool(this);
		vkCoreCommandBuffer = CreateCommandBuffer(this);
	}
}