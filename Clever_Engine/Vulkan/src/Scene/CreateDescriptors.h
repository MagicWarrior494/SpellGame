#pragma once
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include "Context/ContextVulkanData.h"

namespace Vulkan {
    inline void UpdateDescriptorSets(std::shared_ptr<VulkanCore> VC, const DescriptorSetInfo& info, DescriptorResult& result)
    {
        VkDevice device = VC->vkDevice;
        const uint32_t framesInFlight = info.maxSets;

        for (uint32_t frame = 0; frame < framesInFlight; ++frame) {
            std::vector<VkWriteDescriptorSet> writes;
            std::vector<std::vector<VkDescriptorImageInfo>> imageStorage;
            std::vector<std::vector<VkDescriptorBufferInfo>> bufferStorage;

            imageStorage.reserve(info.bindings.size());
            bufferStorage.reserve(info.bindings.size());

            for (const auto& b : info.bindings) {
                uint32_t perSetCount = 0;

                if (!b.images.empty()) {
                    perSetCount = static_cast<uint32_t>(b.images.size() / framesInFlight);
                    imageStorage.emplace_back(perSetCount);

                    for (uint32_t scene = 0; scene < perSetCount; ++scene) {
                        uint32_t idx = scene * framesInFlight + frame;
                        imageStorage.back()[scene] = b.images[idx];
                    }

                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = result.sets[frame];
                    write.dstBinding = b.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = b.type;
                    write.descriptorCount = perSetCount;
                    write.pImageInfo = imageStorage.back().data();
                    writes.push_back(write);
                }
                else if (!b.buffers.empty()) {
                    perSetCount = static_cast<uint32_t>(b.buffers.size() / framesInFlight);
                    bufferStorage.emplace_back(perSetCount);

                    for (uint32_t scene = 0; scene < perSetCount; ++scene) {
                        uint32_t idx = scene * framesInFlight + frame;
                        bufferStorage.back()[scene] = b.buffers[idx];
                    }

                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = result.sets[frame];
                    write.dstBinding = b.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = b.type;
                    write.descriptorCount = perSetCount;
                    write.pBufferInfo = bufferStorage.back().data();
                    writes.push_back(write);
                }
                else {
                    perSetCount = b.count;
                    if (perSetCount == 0) continue;

                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = result.sets[frame];
                    write.dstBinding = b.binding;
                    write.dstArrayElement = 0;
                    write.descriptorType = b.type;
                    write.descriptorCount = perSetCount;
                    write.pBufferInfo = nullptr;
                    write.pImageInfo = nullptr;
                    writes.push_back(write);
                }
            }

            if (!writes.empty()) {
                vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
            }
        }
    }

    inline DescriptorResult CreateDescriptors(std::shared_ptr<VulkanCore> VC, const DescriptorSetInfo& info)
    {
        DescriptorResult result{};
        if (info.bindings.empty()) return result;

        VkDevice device = VC->vkDevice;
        const uint32_t framesInFlight = info.maxSets;
        if (framesInFlight == 0) throw std::runtime_error("CreateDescriptors: info.maxSets must be > 0");

        // Ensure unique binding numbers
        {
            std::unordered_set<uint32_t> seen;
            for (const auto& b : info.bindings) {
                if (!seen.insert(b.binding).second)
                    throw std::runtime_error("CreateDescriptors: duplicate binding number detected");
            }
        }

        // --- Descriptor Set Layout ---
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(info.bindings.size());

        for (const auto& b : info.bindings) {
            uint32_t perSetCount = !b.images.empty() ? b.images.size() / framesInFlight
                : !b.buffers.empty() ? b.buffers.size() / framesInFlight
                : b.count;

            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = b.binding;
            layoutBinding.descriptorType = b.type;
            layoutBinding.descriptorCount = perSetCount;
            layoutBinding.stageFlags = b.stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &result.layout) != VK_SUCCESS)
            throw std::runtime_error("CreateDescriptors: vkCreateDescriptorSetLayout failed");

        // --- Descriptor Pool ---
        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.reserve(info.bindings.size());
        for (const auto& b : info.bindings) {
            uint32_t perSetCount = !b.images.empty() ? b.images.size() / framesInFlight
                : !b.buffers.empty() ? b.buffers.size() / framesInFlight
                : b.count;

            VkDescriptorPoolSize poolSize{};
            poolSize.type = b.type;
            poolSize.descriptorCount = perSetCount * framesInFlight;
            poolSizes.push_back(poolSize);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = framesInFlight;

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &result.pool) != VK_SUCCESS)
            throw std::runtime_error("CreateDescriptors: vkCreateDescriptorPool failed");

        // --- Allocate Descriptor Sets ---
        std::vector<VkDescriptorSetLayout> layouts(framesInFlight, result.layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = result.pool;
        allocInfo.descriptorSetCount = framesInFlight;
        allocInfo.pSetLayouts = layouts.data();

        result.sets.resize(framesInFlight);
        if (vkAllocateDescriptorSets(device, &allocInfo, result.sets.data()) != VK_SUCCESS)
            throw std::runtime_error("CreateDescriptors: vkAllocateDescriptorSets failed");

        // --- Finally, update descriptor sets ---
        UpdateDescriptorSets(VC, info, result);

        return result;
    }
}