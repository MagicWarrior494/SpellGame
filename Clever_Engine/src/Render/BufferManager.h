#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <stack>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "Context/VulkanContext.h"
#include "Buffers/CreateBuffer.h"


class StorageBufferManager
{
public:
	StorageBufferManager(std::shared_ptr<Vulkan::VulkanContext>vulkanContext)
		: storageBuffer(vulkanContext->GetObjectMatrixBuffer())
	{
		mappedPtr = static_cast<glm::mat4*>(storageBuffer.mappedPtr);
	}
	~StorageBufferManager() = default;

	template<typename T>
	void SyncDeletions(const std::vector<uint32_t>& removedCpuIndices)
	{
		T* dataPtr = static_cast<T*>(mappedPtr);

		for (uint32_t deletedCpuIndex : removedCpuIndices)
		{
			//Find the GPU index corresponding to the deleted CPU index
			auto it = cpuToGpuMap.find(deletedCpuIndex);
			if(it == cpuToGpuMap.end())
				continue;
			//Get the GPU index of the deleted object
			uint32_t deletedGpuIndex = it->second;

			//Get the last active GPU index
			uint32_t lastGPUIndex = storageBuffer.activeCount - 1;

			if(deletedGpuIndex != lastGPUIndex)
			{
				uint32_t lastCpuIndex = gpuToCpuMap[lastGPUIndex];

				dataPtr[deletedGpuIndex] = dataPtr[lastGPUIndex];

				cpuToGpuMap[lastCpuIndex] = deletedGpuIndex;
				gpuToCpuMap[deletedGpuIndex] = lastCpuIndex;
			}

			cpuToGpuMap.erase(it);
			storageBuffer.activeCount--;
		}
	}

	template<typename T, typename Func>
	void SyncUpdates(const std::unordered_map<uint32_t, T>& indexData, Func extractor)
	{

		using TargetType = decltype(extractor(std::declval<T&>()));

		//Make sure to call EnsureCapacity before this to avoid overflow
		//If overflow happens we skip any new additions so dont be dumb

		TargetType* dataPtr = static_cast<TargetType*>(mappedPtr);

		for (auto& [index, data] : indexData)
		{
			TargetType value = extractor(data);
			if (cpuToGpuMap.contains(index))
			{
				//Gets the Gpu index from the cpu index
				uint32_t gpuIndex = cpuToGpuMap[index];

				//Updates the data at the gpu index
				dataPtr[gpuIndex] = value;
			}
			else
			{
				size_t maxElements = storageBuffer.capacity / sizeof(TargetType);
				//Because capacity is in bytes we need to divide by sizeof(T) to get the number of T elements that can fit
				if (storageBuffer.activeCount >= storageBuffer.capacity / maxElements)
				{
					continue; //Skip new additions if overflow would occur
				}

				uint32_t newIndex = storageBuffer.activeCount;
				dataPtr[newIndex] = value;
				cpuToGpuMap[index] = newIndex;

				if(gpuToCpuMap.size() <= newIndex)
				{
					gpuToCpuMap.resize(std::max(static_cast<uint32_t>(newIndex * 1.5f), newIndex + 10));
				}
				gpuToCpuMap[newIndex] = index;

				storageBuffer.activeCount++;
			}
		}
	}
private:
	Vulkan::VulkanBuffer& storageBuffer;
	void* mappedPtr = nullptr;

	//This is the CPU's Vector(or Array i guess) index to the dense GPU index
	std::unordered_map<uint32_t, uint32_t> cpuToGpuMap;
	//This is the dense GPU index to the CPU's Vector index
	//The index of the vector is the GPU index, the value is the CPU index
	//Because the GPU index is dense we can use a vector for this
	std::vector<uint32_t> gpuToCpuMap;

};

class UniformBufferManager {
public:
	/**
	 * @param buffer The VulkanBuffer (must be persistently mapped).
	 * @param minAlignment Physical device's minUniformBufferOffsetAlignment.
	 * @param structSize The size of the specific struct this manager handles (e.g., sizeof(SceneData)).
	 */
	UniformBufferManager(Vulkan::VulkanBuffer& buffer, size_t minAlignment, size_t structSize)
		: m_Buffer(buffer), m_MinAlignment(minAlignment), m_StructSize(structSize)
	{
		m_MappedPtr = static_cast<uint8_t*>(buffer.mappedPtr);
		assert(m_MappedPtr != nullptr && "Buffer must be persistently mapped!");

		// Calculate aligned stride: the distance from the start of slot 0 to slot 1
		m_AlignedStride = (m_StructSize + minAlignment - 1) & ~(minAlignment - 1);
	}

	~UniformBufferManager() = default;

	// Returns a recycled index or a new one
	uint32_t AllocateSlot() {
		if (!m_FreeSlots.empty()) {
			uint32_t slot = m_FreeSlots.top();
			m_FreeSlots.pop();
			return slot;
		}

		uint32_t newSlot = m_NextAvailableSlot++;

		// Safety check to ensure we don't exceed allocated GPU memory
		assert((newSlot + 1) * m_AlignedStride <= m_Buffer.capacity && "Uniform Buffer capacity exceeded!");

		return newSlot;
	}

	// Returns the slot to the pool for reuse
	void FreeSlot(uint32_t slotIndex) {
		m_FreeSlots.push(slotIndex);
	}

	/**
	 * Updates the GPU memory for a specific slot.
	 * Use this in your Update/Render loops.
	 */
	void UpdateSlot(uint32_t slotIndex, const void* data) {
		size_t offset = slotIndex * m_AlignedStride;
		// We only copy the actual struct size, ignoring the padding bytes
		memcpy(m_MappedPtr + offset, data, m_StructSize);
	}

	// Returns the offset required for VkDescriptorBufferInfo
	size_t GetOffset(uint32_t slotIndex) const {
		return slotIndex * m_AlignedStride;
	}

	// Returns the size of the actual data (the 'range' in Vulkan descriptors)
	size_t GetStructSize() const {
		return m_StructSize;
	}

private:
	Vulkan::VulkanBuffer& m_Buffer;
	uint8_t* m_MappedPtr;

	size_t m_MinAlignment;
	size_t m_StructSize;
	size_t m_AlignedStride;

	uint32_t m_NextAvailableSlot = 0;
	std::stack<uint32_t> m_FreeSlots;
};