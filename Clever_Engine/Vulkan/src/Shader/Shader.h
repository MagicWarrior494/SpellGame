#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "CoreTypes/VulkanCore.h"
#include "Coretypes/Helper/spirv_reflect.h"
#include <vector>

namespace Vulkan
{
    inline std::vector<char> ReadFile(const std::string& filename) {
        // Open at the end (ate) to get size, and binary mode is MUST for SPIR-V
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            // Adding the filename to the error makes life 10x easier
            throw std::runtime_error("Vulkan IO Error: Could not find or open file: " + filename);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    inline std::vector<uint32_t> ReadSPIRV(const std::string& filename) {
        auto bytes = ReadFile(filename);

        // SPIR-V files must be a multiple of 4 bytes (size of uint32_t)
        if (bytes.size() % sizeof(uint32_t) != 0) {
            throw std::runtime_error("SPIR-V Error: File size is not a multiple of 4: " + filename);
        }

        size_t wordCount = bytes.size() / sizeof(uint32_t);
        std::vector<uint32_t> spirv(wordCount);
        std::memcpy(spirv.data(), bytes.data(), bytes.size());

        return spirv;
    }

    inline VkShaderModule CreateShaderModule(VulkanCore* VC, const std::vector<uint32_t>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(VC->vkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module!");

        return shaderModule;
    }


    VkDescriptorType ConvertType(SpvReflectDescriptorType type) {
        return static_cast<VkDescriptorType>(type);
    }

    Vulkan::ShaderMetadata ReflectCombinedShaders(const std::vector<uint32_t>& vertCode, const std::vector<uint32_t>& fragCode) {
        Vulkan::ShaderMetadata meta;

        auto ProcessStage = [&](const std::vector<uint32_t>& code, VkShaderStageFlags stage) {
            SpvReflectShaderModule module;
            spvReflectCreateShaderModule(code.size() * sizeof(uint32_t), code.data(), &module);

            uint32_t count = 0;
            spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
            std::vector<SpvReflectDescriptorBinding*> bindings(count);
            spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

            for (auto* b : bindings) {
                // Check if we already found this binding in a previous stage
                bool found = false;
                for (auto& existing : meta.bindings) {
                    if (existing.binding == b->binding) {
                        existing.stage |= stage; // Merge stages (e.g. Vertex | Fragment)
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    Vulkan::ShaderBinding sb;
                    sb.binding = b->binding;
                    sb.name = b->name; // e.g. "u_ElementTable"
                    sb.type = ConvertType(b->descriptor_type);
                    sb.stage = stage;
                    sb.count = b->count;
                    meta.bindings.push_back(sb);
                }
            }


            uint32_t pcCount = 0;
            spvReflectEnumeratePushConstantBlocks(&module, &pcCount, nullptr);
            std::vector<SpvReflectBlockVariable*> pcs(pcCount);
            spvReflectEnumeratePushConstantBlocks(&module, &pcCount, pcs.data());

            for (auto* p : pcs) {
                Vulkan::ShaderMetadata::PushConstant pc;
                pc.name = p->name;
                pc.size = p->size;
                pc.offset = p->offset;
                pc.stageFlags = stage;
                meta.pushConstants.push_back(pc);
            }

            spvReflectDestroyShaderModule(&module);
        };

        ProcessStage(vertCode, VK_SHADER_STAGE_VERTEX_BIT);
        ProcessStage(fragCode, VK_SHADER_STAGE_FRAGMENT_BIT);

        return meta;
    }
}