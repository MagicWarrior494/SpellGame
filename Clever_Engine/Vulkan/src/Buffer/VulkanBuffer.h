#pragma once
#include <vulkan/vulkan.h>
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace Vulkan {

	struct VulkanCore;

	struct VulkanBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkDeviceSize size = 0;
		VkDeviceSize capacity = 0;

		void* mappedPtr = nullptr;

		VkBufferUsageFlags usageFlags = 0;
		VkMemoryPropertyFlags memoryFlags = 0;

		// ============================================================
		// CREATE
		// ============================================================

		void Create(
			VulkanCore* vc,
			VkDeviceSize initialSize,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties)
		{
			Destroy(vc); // safety

			usageFlags = usage;
			memoryFlags = properties;
			capacity = std::max((VkDeviceSize)1, initialSize);
			size = initialSize;

			CreateBufferInternal(vc, capacity, usageFlags, memoryFlags);

			// If host visible → map once
			if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMapMemory(vc->vkDevice, memory, 0, capacity, 0, &mappedPtr);
			}
		}

		// ============================================================
		// UPDATE
		// ============================================================

		void Update(
			VulkanCore* vc,
			const void* data,
			VkDeviceSize dataSize)
		{
			if (dataSize > capacity)
			{
				Resize(vc, dataSize);
			}

			if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				memcpy(mappedPtr, data, (size_t)dataSize);
			}
			else
			{
				// device local → staging upload
				UploadViaStaging(vc, data, dataSize);
			}

			size = dataSize;
		}

		// ============================================================
		// DESTROY
		// ============================================================

		void Destroy(VulkanCore* vc)
		{
			if (mappedPtr)
			{
				vkUnmapMemory(vc->vkDevice, memory);
				mappedPtr = nullptr;
			}

			if (buffer != VK_NULL_HANDLE)
			{
				vkDestroyBuffer(vc->vkDevice, buffer, nullptr);
				buffer = VK_NULL_HANDLE;
			}

			if (memory != VK_NULL_HANDLE)
			{
				vkFreeMemory(vc->vkDevice, memory, nullptr);
				memory = VK_NULL_HANDLE;
			}

			size = 0;
			capacity = 0;
		}

		// ============================================================
		// INTERNAL HELPERS
		// ============================================================

	private:

		void CreateBufferInternal(
			VulkanCore* vc,
			VkDeviceSize bufferSize,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties)
		{
			VkBufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.size = bufferSize;
			info.usage = usage;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(vc->vkDevice, &info, nullptr, &buffer) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create buffer");
			}

			VkMemoryRequirements memReq;
			vkGetBufferMemoryRequirements(vc->vkDevice, buffer, &memReq);

			VkMemoryAllocateInfo alloc{};
			alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc.allocationSize = memReq.size;
			alloc.memoryTypeIndex = FindMemoryType(
				vc->physicalDeviceData.vkPhysicalDevice,
				memReq.memoryTypeBits,
				properties
			);

			if (vkAllocateMemory(vc->vkDevice, &alloc, nullptr, &memory) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate buffer memory");
			}

			vkBindBufferMemory(vc->vkDevice, buffer, memory, 0);
		}

		void Resize(
			VulkanCore* vc,
			VkDeviceSize newSize)
		{
			VkDeviceSize newCapacity = std::max(newSize, capacity * 2);

			VulkanBuffer newBuffer;
			newBuffer.Create(vc, newCapacity, usageFlags, memoryFlags);

			// Copy old data
			if (size > 0)
			{
				CopyBuffer(vc, buffer, newBuffer.buffer, size);
			}

			Destroy(vc);

			*this = std::move(newBuffer);
		}

		void UploadViaStaging(
			VulkanCore* vc,
			const void* data,
			VkDeviceSize dataSize)
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;

			CreateBuffer(
				vc,
				dataSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer,
				stagingMemory
			);

			void* mapped = nullptr;
			vkMapMemory(vc->vkDevice, stagingMemory, 0, dataSize, 0, &mapped);
			memcpy(mapped, data, (size_t)dataSize);
			vkUnmapMemory(vc->vkDevice, stagingMemory);

			CopyBuffer(vc, stagingBuffer, buffer, dataSize);

			vkDestroyBuffer(vc->vkDevice, stagingBuffer, nullptr);
			vkFreeMemory(vc->vkDevice, stagingMemory, nullptr);
		}
	};

	inline void CreateBuffer(VulkanCore* VC, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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
		allocInfo.memoryTypeIndex = FindMemoryType(VC->physicalDeviceData.vkPhysicalDevice, memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(VC->vkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}

		vkBindBufferMemory(VC->vkDevice, buffer, bufferMemory, 0);
	}

	inline void CopyBuffer(VulkanCore* VC, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(VC);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(VC, commandBuffer);
	}

	static void CreateBufferInternal(VulkanCore* vc, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, VulkanBuffer& outBuffer)
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
		VulkanCore* vc,
		VulkanBuffer& dst,
		const void* data,
		VkDeviceSize dataSize
	)
	{
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
		VulkanCore* vc,
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
	inline uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) 
	{ 
		VkPhysicalDeviceMemoryProperties memProperties; 
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties); 
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{ 
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{ 
				return i;
			} 
		} 
		throw std::runtime_error("Failed to find suitable memory type!");
	}
} 