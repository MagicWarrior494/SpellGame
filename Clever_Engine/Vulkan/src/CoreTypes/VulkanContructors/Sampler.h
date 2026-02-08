#pragma once
#include <stdexcept>

#include "CoreTypes/VulkanCore.h" 

namespace Vulkan
{
	struct SamplerConfig
	{
		VkFilter magFilter = VK_FILTER_LINEAR;
		VkFilter minFilter = VK_FILTER_LINEAR;
		VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkBool32 anisotropyEnable = VK_TRUE;
		float maxAnisotropy = 16.0f;
		VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		VkBool32 unnormalizedCoordinates = VK_FALSE;
		VkBool32 compareEnable = VK_FALSE;
		VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
		VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		float mipLodBias = 0.0f;
		float minLod = 0.0f;
		float maxLod = VK_LOD_CLAMP_NONE;

		// Preset configurations
		static SamplerConfig Offscreen()
		{
			SamplerConfig config;
			config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			config.anisotropyEnable = VK_FALSE;
			config.maxAnisotropy = 1.0f;
			config.maxLod = 0.0f;
			return config;
		}

		static SamplerConfig Nearest()
		{
			SamplerConfig config;
			config.magFilter = VK_FILTER_NEAREST;
			config.minFilter = VK_FILTER_NEAREST;
			config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			return config;
		}
	};

	inline VkSampler CreateSampler(VulkanCore* vulkanCore, const SamplerConfig& config = SamplerConfig())
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = config.magFilter;
		samplerInfo.minFilter = config.minFilter;
		samplerInfo.addressModeU = config.addressModeU;
		samplerInfo.addressModeV = config.addressModeV;
		samplerInfo.addressModeW = config.addressModeW;
		samplerInfo.anisotropyEnable = config.anisotropyEnable;
		samplerInfo.maxAnisotropy = config.maxAnisotropy;
		samplerInfo.borderColor = config.borderColor;
		samplerInfo.unnormalizedCoordinates = config.unnormalizedCoordinates;
		samplerInfo.compareEnable = config.compareEnable;
		samplerInfo.compareOp = config.compareOp;
		samplerInfo.mipmapMode = config.mipmapMode;
		samplerInfo.mipLodBias = config.mipLodBias;
		samplerInfo.minLod = config.minLod;
		samplerInfo.maxLod = config.maxLod;

		VkSampler sampler;
		if (vkCreateSampler(vulkanCore->vkDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
		return sampler;
	}
}