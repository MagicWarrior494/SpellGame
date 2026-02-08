#pragma once
#include <optional>
#include <string>
#include <vulkan/vulkan.h>

namespace Vulkan
{
	struct PhysicalDevice
	{

		std::optional<uint32_t> graphicsIndex;
		std::optional<uint32_t> presentIndex;

		std::string deviceName = "Undefined";

		VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;

		bool isComplete()
		{
			return graphicsIndex.has_value() && presentIndex.has_value();
		}
	};

	class VulkanCore
	{ 
	public:
		VulkanCore();
		~VulkanCore();
		
	public:
		VkInstance vkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT vkDebugMesseneger = VK_NULL_HANDLE;
		VkDevice vkDevice = VK_NULL_HANDLE;
		PhysicalDevice physicalDeviceData{};
		VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
		VkQueue vkPresentQueue = VK_NULL_HANDLE;
		VkCommandPool vkCoreCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer vkCoreCommandBuffer = VK_NULL_HANDLE;
	};
}