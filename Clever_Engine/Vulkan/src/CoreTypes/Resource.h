#pragma once
#include <variant>
#include <map>
#include <string>

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

        VulkanBuffer* getBuffer() const { return std::get<VulkanBuffer*>(data); }
        VulkanImage* getImage() const { return std::get<VulkanImage*>(data); }
        VkSampler* getSampler() const { return std::get<VkSampler*>(data); }
    };

    using ResourceMap = std::map<std::string, Resource>;


    // Individual binding info extracted from the SPIR-V
    struct ShaderBinding {
        uint32_t binding;           // The 'binding = X' in GLSL
        uint32_t set;               // The 'set = Y' in GLSL
        VkDescriptorType type;      // e.g., VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
        VkShaderStageFlags stage;   // e.g., VK_SHADER_STAGE_VERTEX_BIT
        std::string name;           // The variable name (e.g., "u_ElementTable")
        uint32_t count;             // Array size (usually 1, but >1 for arrays)
    };

    // Metadata for the entire shader (or a group of shaders in a pipeline)
    struct ShaderMetadata {
        // All bindings found across the shader stages
        std::vector<ShaderBinding> bindings;

        // Push constants are handled differently than Descriptor Sets
        struct PushConstant {
            uint32_t size;
            uint32_t offset;
            VkShaderStageFlags stageFlags;
            std::string name; // Name of the push constant block
        };
        std::vector<PushConstant> pushConstants;

        // Helper to find a binding by name
        const ShaderBinding* FindBinding(const std::string& name) const {
            for (const auto& b : bindings) {
                if (b.name == name) return &b;
            }
            return nullptr;
        }
    };
}