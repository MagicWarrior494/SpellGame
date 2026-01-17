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

	inline VkCommandBuffer BeginSingleTimeCommands(std::shared_ptr<VulkanCore> VC)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VC->coreCommandPool;
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

		vkFreeCommandBuffers(VC->vkDevice, VC->coreCommandPool, 1, &commandBuffer);
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

	static void CreateBufferInternal(std::shared_ptr<VulkanCore> vc, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VulkanBuffer& outBuffer)
	{
		CreateBuffer(
			vc,
			size,
			usage,
			memoryFlags,
			outBuffer.buffer,
			outBuffer.memory
		);
	}

	static void UploadViaStaging(
		std::shared_ptr<VulkanCore> vc,
		VulkanBuffer& dst,
		const void* data,
		VkDeviceSize dataSize
	)
	{
		assert(dataSize <= dst.capacity && "GPU buffer overflow");

		VkBuffer staging;
		VkDeviceMemory stagingMem;

		CreateBuffer(
			vc,
			dataSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging,
			stagingMem
		);

		void* mapped;
		vkMapMemory(vc->vkDevice, stagingMem, 0, dataSize, 0, &mapped);
		memcpy(mapped, data, static_cast<size_t>(dataSize));
		vkUnmapMemory(vc->vkDevice, stagingMem);

		CopyBuffer(vc, staging, dst.buffer, dataSize);

		vkDestroyBuffer(vc->vkDevice, staging, nullptr);
		vkFreeMemory(vc->vkDevice, stagingMem, nullptr);

		dst.size = dataSize;
	}

	static void EnsureCapacity(
		std::shared_ptr<VulkanCore> vc,
		VulkanBuffer& buffer,
		VkDeviceSize requiredSize,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags memoryFlags
	)
	{
		if (requiredSize <= buffer.capacity)
			return;

		VkDeviceSize newCapacity =
			std::max(requiredSize, buffer.capacity * 2);

		VulkanBuffer newBuffer{};
		newBuffer.capacity = newCapacity;

		CreateBufferInternal(vc, newCapacity, usage, memoryFlags, newBuffer);

		// Copy old data
		if (buffer.size > 0)
		{
			CopyBuffer(vc, buffer.buffer, newBuffer.buffer, buffer.size);
		}

		// Destroy old buffer
		vkDestroyBuffer(vc->vkDevice, buffer.buffer, nullptr);
		vkFreeMemory(vc->vkDevice, buffer.memory, nullptr);

		buffer = newBuffer;
	}

	// ---------- UNIFORM BUFFER ----------
	static VulkanBuffer CreateUniformBuffer(std::shared_ptr<VulkanCore> vc, VkDeviceSize sizeBytes)
	{
		VulkanBuffer buffer{};
		buffer.capacity = sizeBytes;
		buffer.size = sizeBytes;

		CreateBufferInternal(
			vc,
			sizeBytes,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffer
		);

		return buffer;
	}

	static void UpdateUniform(
		std::shared_ptr<VulkanCore> vc,
		VulkanBuffer& buffer,
		const void* data,
		VkDeviceSize dataSize
	)
	{
		assert(dataSize <= buffer.capacity);

		void* mapped;
		vkMapMemory(vc->vkDevice, buffer.memory, 0, dataSize, 0, &mapped);
		memcpy(mapped, data, static_cast<size_t>(dataSize));
		vkUnmapMemory(vc->vkDevice, buffer.memory);

		buffer.size = dataSize;
	}

	// ---------- VERTEX BUFFER ----------
	static VulkanBuffer CreateVertexBuffer(
		std::shared_ptr<VulkanCore> vc,
		VkDeviceSize capacityBytes
	)
	{
		VulkanBuffer buffer{};
		buffer.capacity = capacityBytes;

		CreateBufferInternal(
			vc,
			capacityBytes,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			buffer
		);

		return buffer;
	}

	static void UpdateVertexBuffer(
		std::shared_ptr<VulkanCore> vc,
		VulkanBuffer& buffer,
		const void* data,
		VkDeviceSize dataSize
	)
	{
		EnsureCapacity(
			vc,
			buffer,
			dataSize, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		UploadViaStaging(vc, buffer, data, dataSize);
	}

	// ---------- INDEX BUFFER ----------
	static VulkanBuffer CreateIndexBuffer(
		std::shared_ptr<VulkanCore> vc,
		VkDeviceSize capacityBytes
	)
	{
		VulkanBuffer buffer{};
		buffer.capacity = capacityBytes;

		CreateBufferInternal(
			vc,
			capacityBytes,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			buffer
		);

		return buffer;
	}

	static void UpdateIndex(
		std::shared_ptr<VulkanCore> vc,
		VulkanBuffer& buffer,
		const void* data,
		VkDeviceSize dataSize
	)
	{
		EnsureCapacity(
			vc,
			buffer, 
			dataSize, 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		UploadViaStaging(vc, buffer, data, dataSize);
	}

	// ---------- INSTANCE BUFFER ----------
	static VulkanBuffer CreateInstanceBuffer(
		std::shared_ptr<VulkanCore> vc,
		VkDeviceSize capacityBytes
	)
	{
		VulkanBuffer buffer{};
		buffer.capacity = capacityBytes;

		CreateBufferInternal(
			vc,
			capacityBytes,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			buffer
		);

		return buffer;
	}

	static void UpdateInstance(
		std::shared_ptr<VulkanCore> vc,
		VulkanBuffer& buffer,
		const void* data,
		VkDeviceSize dataSize
	)
	{
		EnsureCapacity(
			vc, 
			buffer, 
			dataSize, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		UploadViaStaging(vc, buffer, data, dataSize);
	}

	// ---------- STORAGE BUFFER (SSBO) ----------
	static VulkanBuffer CreateStorageBuffer(
		std::shared_ptr<VulkanCore> vc,
		VkDeviceSize capacityBytes
	)
	{
		VulkanBuffer buffer{};
		buffer.capacity = capacityBytes;

		CreateBufferInternal(
			vc,
			capacityBytes,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffer
		);
		vkMapMemory(vc->vkDevice, buffer.memory, 0, capacityBytes, 0, &buffer.mappedPtr);
		return buffer;
	}

	static void UpdateStorage(
		std::shared_ptr<VulkanCore> vc,
		VulkanBuffer& buffer,
		const void* data,
		VkDeviceSize dataSize
	)
	{
		UploadViaStaging(vc, buffer, data, dataSize);
	}
}