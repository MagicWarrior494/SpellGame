#pragma once
#include <vulkan/vulkan.h>
#include <expected>
#include <set>
#include "Context/ContextVulkanData.h"
#include <stdexcept>

namespace Vulkan {

	PhysicalDeviceData GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice)
	{
		uint32_t queueFamilyPropertyCount = 0;
		std::vector<VkQueueFamilyProperties> queueFamilyPropteries;

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

		queueFamilyPropteries.resize(queueFamilyPropertyCount);

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyPropteries.data());

		VkPhysicalDeviceProperties physicalDeviceProperties;

		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		PhysicalDeviceData physicalDeviceData;

		physicalDeviceData.deviceName = physicalDeviceProperties.deviceName;

		int i = 0;
		for (const auto& queueFamilyProperty : queueFamilyPropteries)
		{
			if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				physicalDeviceData.graphicsIndex = i;
			}
			if (queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				physicalDeviceData.presentIndex = i;
			}
			if (physicalDeviceData.graphicsIndex.has_value() && physicalDeviceData.presentIndex.has_value())
				break;

			i++;
		}
		return physicalDeviceData;
	};

	void CreateLogicalDevice(std::shared_ptr<VulkanCore> VC)
	{
		VulkanCore& vulkanCore = *VC;

		std::vector <VkDeviceQueueCreateInfo> queueCreateInfo;

		uint32_t queueFamilyPropertyCount = 0;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;

		vkGetPhysicalDeviceQueueFamilyProperties(vulkanCore.vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);

		queueFamilyProperties.resize(queueFamilyPropertyCount);

		vkGetPhysicalDeviceQueueFamilyProperties(vulkanCore.vkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

		bool graphicsQueue = false;
		bool presentQueue = false;

		PhysicalDeviceData QFI = GetPhysicalDeviceProperties(vulkanCore.vkPhysicalDevice);

		float queuePriority = 1.0f;
		std::set<uint32_t> uniqueIndicies = { QFI.graphicsIndex.value(), QFI.presentIndex.value() };
		for (auto index : uniqueIndicies)
		{
			VkDeviceQueueCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.queueFamilyIndex = index;
			info.queueCount = 1;
			info.pQueuePriorities = &queuePriority;
			queueCreateInfo.push_back(info);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.fillModeNonSolid = true;
		deviceFeatures.wideLines = true;

		std::vector<const char*> deviceExtextions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		std::vector<const char*> validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtextions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtextions.data();
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();

		if (vkCreateDevice(vulkanCore.vkPhysicalDevice, &deviceCreateInfo, nullptr, &vulkanCore.vkDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create Logical Device!");
		}

		vkGetDeviceQueue(vulkanCore.vkDevice, vulkanCore.d_PhysicalDeviceData.graphicsIndex.value(), 0, &vulkanCore.graphicsQueue);
		vkGetDeviceQueue(vulkanCore.vkDevice, vulkanCore.d_PhysicalDeviceData.presentIndex.value(), 0, &vulkanCore.presentQueue);
	}
}