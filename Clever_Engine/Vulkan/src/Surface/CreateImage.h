#pragma once
#include "Context/ContextVulkanData.h"
#include <stdexcept>

namespace Vulkan {

	inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("Failed to find suitable memory type!");
	}

	inline void createImage(
		VulkanCore& vulkanCore,
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkSampleCountFlagBits samples,
		uint32_t layers,
		VkImageCreateFlags flags,
		VulkanImage& outImage)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = layers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = samples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = flags;

		if (vkCreateImage(vulkanCore.vkDevice, &imageInfo, nullptr, &outImage.image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(vulkanCore.vkDevice, outImage.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(vulkanCore.vkPhysicalDevice, memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(vulkanCore.vkDevice, &allocInfo, nullptr, &outImage.memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(vulkanCore.vkDevice, outImage.image, outImage.memory, 0);
	}

	inline VkImageView createImageView(
		VulkanCore& vulkanCore,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		VkImageViewType viewType,
		uint32_t layers)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = viewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = layers;

		VkImageView imageView;
		if (vkCreateImageView(vulkanCore.vkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
		return imageView;
	}

	inline std::vector<VulkanImage> initImageByType(VulkanCore& vulkanCore, ImageType type, uint32_t width, uint32_t height, uint8_t count = 1) {
		std::vector<VulkanImage> out(count);
		VkFormat format;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspect;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		uint32_t layers = 1;
		VkImageCreateFlags flags = 0;
		for (int i = 0; i < count; i++) {
			switch (type) {
			case ImageType::Depth:
				format = VK_FORMAT_D32_SFLOAT;
				usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
				aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
				break;
			case ImageType::Color:
				format = VK_FORMAT_R8G8B8A8_UNORM;
				usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
				break;
			case ImageType::Texture:
				format = VK_FORMAT_R8G8B8A8_UNORM;
				usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
				break;
			case ImageType::CubeMap:
				format = VK_FORMAT_R8G8B8A8_UNORM;
				usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
				layers = 6;
				flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
				break;
			case ImageType::Multisampled:
				format = VK_FORMAT_R8G8B8A8_UNORM;
				usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
				samples = VK_SAMPLE_COUNT_4_BIT; // example MSAA 4x
				break;
			case ImageType::Storage:
				format = VK_FORMAT_R8G8B8A8_UNORM;
				usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				aspect = VK_IMAGE_ASPECT_COLOR_BIT;
				break;
			default:
				throw std::runtime_error("Unknown image type");
			}

			createImage(vulkanCore, width, height, format, VK_IMAGE_TILING_OPTIMAL, usage,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, samples, layers, flags, out[i]);

			VkImageViewType viewType = (type == ImageType::CubeMap)
				? VK_IMAGE_VIEW_TYPE_CUBE
				: VK_IMAGE_VIEW_TYPE_2D;

			out[i].view = createImageView(vulkanCore, out[i].image, format, aspect, viewType, layers);
		}
		return out;
	}
}