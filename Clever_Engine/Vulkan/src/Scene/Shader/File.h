#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Context/ContextVulkanData.h" 

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

	inline VkShaderModule CreateShaderModule(std::shared_ptr<VulkanCore> VC, const std::vector<uint32_t>& code)
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
}