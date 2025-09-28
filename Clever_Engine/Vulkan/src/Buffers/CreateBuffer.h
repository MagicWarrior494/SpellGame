#pragma once
#include <memory>
#include <stdexcept> 

#include "Context/ContextVulkanData.h"

namespace Vulkan {
	inline uint32_t FindMemoryType(
		VkPhysicalDevice physicalDevice,
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties
	) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	inline VkCommandBuffer BeginSingleTimeCommands(std::shared_ptr<VulkanCore> VC)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VC->VkCommandPools[0];
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(VC->vkDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	inline void EndSingleTimeCommands(std::shared_ptr<VulkanCore> VC, VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(VC->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(VC->graphicsQueue);

		vkFreeCommandBuffers(VC->vkDevice, VC->VkCommandPools[0], 1, &commandBuffer);
	}

	inline void CreateBuffer(std::shared_ptr<VulkanCore> VC, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(VC->vkDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VC->vkDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(VC->vkPhysicalDevice, memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(VC->vkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(VC->vkDevice, buffer, bufferMemory, 0);
	}

	inline void CopyBuffer(std::shared_ptr<VulkanCore> VC, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(VC);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(VC, commandBuffer);
	}

	//Creates a Physical Device side buffer that can be read from the vertex
	template<typename T>
	inline VulkanBuffer CreateAndAllocateBuffer(std::shared_ptr<VulkanCore> VC, std::vector<T>& content, BufferTypes buffertype) {
		VulkanBuffer vulkanBuffer{};

		VkDeviceSize bufferSize = sizeof(content[0]) * content.size() * 10;
		vulkanBuffer.size = bufferSize;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(VC, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(VC->vkDevice, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
		memcpy(data, content.data(), static_cast<size_t>(bufferSize/10));
		vkUnmapMemory(VC->vkDevice, stagingBufferMemory);

		if (buffertype == BufferTypes::VertexBuffer)
		{
			CreateBuffer(VC, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanBuffer.buffer, vulkanBuffer.memory);
		}

		CopyBuffer(VC, stagingBuffer, vulkanBuffer.buffer, bufferSize);

		vkDestroyBuffer(VC->vkDevice, stagingBuffer, nullptr);
		vkFreeMemory(VC->vkDevice, stagingBufferMemory, nullptr);

		return vulkanBuffer;
	}

	template<typename T>
	inline void UpdateBuffer(std::shared_ptr<VulkanCore> VC, VulkanBuffer& vulkanBuffer, std::vector<T>& content, BufferTypes bufferType) {
		VkDeviceSize bufferSize = sizeof(content[0]) * content.size();

		if (bufferSize > vulkanBuffer.size)
		{
			vkDestroyBuffer(VC->vkDevice, vulkanBuffer.buffer, nullptr);
			vkFreeMemory(VC->vkDevice, vulkanBuffer.memory, nullptr);
			vulkanBuffer = CreateAndAllocateBuffer(VC, content, bufferType);
			return;
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(VC, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(VC->vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, content.data(), (size_t)bufferSize);
		vkUnmapMemory(VC->vkDevice, stagingBufferMemory);

		CopyBuffer(VC, stagingBuffer, vulkanBuffer.buffer, bufferSize);

		vkDestroyBuffer(VC->vkDevice, stagingBuffer, nullptr);
		vkFreeMemory(VC->vkDevice, stagingBufferMemory, nullptr);
	}
}