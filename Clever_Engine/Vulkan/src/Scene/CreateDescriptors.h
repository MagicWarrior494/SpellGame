#pragma once
#include <memory>
#include <stdexcept>

#include "Context/ContextVulkanData.h"


namespace Vulkan {
	inline void CreateDescriptorPool(std::shared_ptr<VulkanCore> VC, std::shared_ptr<VulkanScene> vulkanScene)
	{
		VkDescriptorPoolSize sizeInfo{};
		sizeInfo.descriptorCount = static_cast<uint32_t>(VC->MAX_FRAMES_IN_FLIGHT);
		sizeInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.poolSizeCount = 1;
		info.pPoolSizes = &sizeInfo;
		info.maxSets = static_cast<uint32_t>(VC->MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPool pool;

		if (vkCreateDescriptorPool(VC->vkDevice, &info, nullptr, &pool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool");
		}

		vulkanScene->vkDesciptorPool = std::move(pool);
	}



	inline void CreateDescriptorSetLayouts(std::shared_ptr<VulkanCore> VC, std::shared_ptr<VulkanScene> vulkanScene)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 0;
		layoutInfo.pBindings = nullptr;

		VkDescriptorSetLayout layout;
		if (vkCreateDescriptorSetLayout(VC->vkDevice, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout");
		}

		vulkanScene->sceneVkDesciptoreSetLayouts.push_back(layout);
	}



	inline void CreateDescriptorSets(std::shared_ptr<VulkanCore> VC, std::shared_ptr<VulkanScene> vulkanScene)
	{
		std::vector<VkDescriptorSetLayout> layouts(VC->MAX_FRAMES_IN_FLIGHT, vulkanScene->sceneVkDesciptoreSetLayouts[0]);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = vulkanScene->vkDesciptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		vulkanScene->sceneVkDescriptorSets.resize(layouts.size());
		if (vkAllocateDescriptorSets(VC->vkDevice, &allocInfo, vulkanScene->sceneVkDescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets");
		}
	}
}