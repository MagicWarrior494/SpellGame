#pragma once

#include <unordered_map>
#include <vector>
#include <glm.hpp>

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
