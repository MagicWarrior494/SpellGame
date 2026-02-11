#pragma once
#include <variant>
#include <map>

#include "Buffer/VulkanBuffer.h"
#include "Image/VulkanImage.h"


namespace Vulkan
{
    struct Resource {
        // The variant holds the actual Vulkan handles + metadata
        // You could also use std::any, but variant is safer and faster
        std::variant<VulkanBuffer*, VulkanImage*, VkSampler*> data;

        // Helper to check what's inside
        bool isBuffer() const { return std::holds_alternative<VulkanBuffer*>(data); }
        bool isImage() const { return std::holds_alternative<VulkanImage*>(data); }

        VulkanBuffer* getBuffer() { return std::get<VulkanBuffer*>(data); }
        VulkanImage* getImage() { return std::get<VulkanImage*>(data); }
        VkSampler* getSampler() { return std::get<VkSampler*>(data); }
    };

    using ResourceMap = std::map<std::string, Resource>;
}