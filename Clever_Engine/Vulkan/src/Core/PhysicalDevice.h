#pragma once
#include "Context/ContextVulkanData.h"
#include "LogicalDevice.h"
#include <stdexcept>

namespace Vulkan {
	void CreatePhysicalDevice(std::shared_ptr<VulkanCore> VC)
	{
		VulkanCore& vulkanCore = *VC;
		uint32_t physicalDeviceCount = 0;

		vkEnumeratePhysicalDevices(vulkanCore.vkInstance, &physicalDeviceCount, NULL);

		std::vector<VkPhysicalDevice> possiblePhysicalDevices;
		std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties;

		possiblePhysicalDevices.resize(physicalDeviceCount);
		physicalDeviceProperties.resize(physicalDeviceCount);

		if (vkEnumeratePhysicalDevices(vulkanCore.vkInstance, &physicalDeviceCount, possiblePhysicalDevices.data()))
		{
			throw std::runtime_error("Unable to find able physical devices");
		}

		for (VkPhysicalDevice physicalDevice : possiblePhysicalDevices)
		{
			PhysicalDeviceData physicalDeviceData = GetPhysicalDeviceProperties(physicalDevice);

			if (physicalDeviceData.isComplete())
			{
				vulkanCore.vkPhysicalDevice = physicalDevice;
				vulkanCore.d_PhysicalDeviceData = physicalDeviceData;
				return;
			}
		}
		throw std::runtime_error("Failed to find a valid Physical Device(GPU)");
	}
}