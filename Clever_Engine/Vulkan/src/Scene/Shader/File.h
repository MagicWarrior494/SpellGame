#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "Context/ContextVulkanData.h" 

namespace Vulkan
{
	inline std::vector<char> ReadFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open file!");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	inline std::vector<uint32_t> ReadSPIRV(const std::string& filename) {
		auto bytes = ReadFile(filename);
		size_t wordCount = bytes.size() / sizeof(uint32_t);
		std::vector<uint32_t> spirv(wordCount);
		memcpy(spirv.data(), bytes.data(), bytes.size());
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