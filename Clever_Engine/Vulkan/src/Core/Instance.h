#pragma once
#include "Context/ContextVulkanData.h"

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>

namespace Vulkan
{
	std::vector<const char*> getRequiredExtensions(std::vector<const char*> desiredExtensions = {})
	{
		uint32_t glfwExtensionCount;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		for (const auto& ext : desiredExtensions)
		{
			extensions.push_back(ext);
		}

		return extensions;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return false;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void CreateVulkanInstance(std::shared_ptr<VulkanCore> VC)
	{
		VulkanCore& vulkanCore = *VC;
		glfwInit();

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Clever_Engine";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.pEngineName = "Clever";
		appInfo.apiVersion = VK_API_VERSION_1_0;

		auto extensions = getRequiredExtensions();

		const std::vector<const char*> validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		populateDebugMessengerCreateInfo(debugCreateInfo);

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = (uint32_t)extensions.size();
		instanceInfo.ppEnabledExtensionNames = extensions.data();
		instanceInfo.enabledLayerCount = (uint32_t)validationLayers.size();
		instanceInfo.ppEnabledLayerNames = validationLayers.data();
		instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

		if (vkCreateInstance(&instanceInfo, nullptr, &vulkanCore.vkInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}

		VkResult result;

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanCore.vkInstance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			result = func(vulkanCore.vkInstance, &debugCreateInfo, nullptr, &vulkanCore.vkDebugMesseneger);
		}
		else {
			result = VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}