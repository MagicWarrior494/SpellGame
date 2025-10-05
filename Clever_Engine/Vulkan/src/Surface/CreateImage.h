#pragma once
#include "Context/ContextVulkanData.h"
#include <stdexcept>
#include "Buffers/CreateBuffer.h"
namespace Vulkan {

	enum class ImageType : uint8_t {
		Depth,
		DepthStencil,
		Color,
		Texture,
		CubeMap,
		Multisampled,
		Storage
	};

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

	inline std::vector<VulkanImage> initImageByType(
		VulkanCore& vulkanCore,
		ImageType type,
		uint32_t width,
		uint32_t height,
		uint32_t count = 1,
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
		VkFormat overrideFormat = VK_FORMAT_UNDEFINED)
	{
		std::vector<VulkanImage> images(count);

		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageUsageFlags usage = 0;
		VkImageAspectFlags aspectMask = 0;

		switch (type) {
		case ImageType::Color:
			format = VK_FORMAT_B8G8R8A8_UNORM;
			usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			break;

		case ImageType::Depth:
			format = VK_FORMAT_D32_SFLOAT;
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;

		case ImageType::DepthStencil:
			format = VK_FORMAT_D24_UNORM_S8_UINT;
			usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		}

		// If user provided a custom format, use it instead
		if (overrideFormat != VK_FORMAT_UNDEFINED) {
			format = overrideFormat;
		}

		for (uint32_t i = 0; i < count; i++) {
			createImage(
				vulkanCore,
				width,
				height,
				format,
				VK_IMAGE_TILING_OPTIMAL,
				usage,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				samples,
				1,
				0,
				images[i]
			);

			images[i].view = createImageView(
				vulkanCore,
				images[i].image,
				format,
				aspectMask,
				VK_IMAGE_VIEW_TYPE_2D,
				1
			);
		}

		return images;
	}

	inline std::vector<VulkanImage> CreateDummyImagesWithTransition(
		std::shared_ptr<VulkanCore> vulkanCore,
		VkExtent2D extent,
		uint32_t count,
		VkFormat format,
		VkSampler sampler)
	{
		VulkanCore& VC = *vulkanCore;
		std::vector<VulkanImage> dummyImages(count);

		VkCommandBuffer cmd = BeginSingleTimeCommands(vulkanCore); // helper to begin a temp cmd buffer

		for (uint32_t i = 0; i < count; ++i)
		{
			VulkanImage& img = dummyImages[i];

			// --- 1. Create the image ---
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = extent.width;
			imageInfo.extent.height = extent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateImage(VC.vkDevice, &imageInfo, nullptr, &img.image) != VK_SUCCESS)
				throw std::runtime_error("Failed to create dummy image!");

			// --- 2. Allocate memory ---
			VkMemoryRequirements memReqs;
			vkGetImageMemoryRequirements(VC.vkDevice, img.image, &memReqs);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memReqs.size;
			allocInfo.memoryTypeIndex = FindMemoryType(VC.vkPhysicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			if (vkAllocateMemory(VC.vkDevice, &allocInfo, nullptr, &img.memory) != VK_SUCCESS)
				throw std::runtime_error("Failed to allocate dummy image memory!");

			vkBindImageMemory(VC.vkDevice, img.image, img.memory, 0);

			// --- 3. Create image view ---
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = img.image;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(VC.vkDevice, &viewInfo, nullptr, &img.view) != VK_SUCCESS)
				throw std::runtime_error("Failed to create dummy image view!");

			img.format = format;
			img.extent = { extent.width, extent.height, 1 };
			img.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			// --- 4. Transition layout to SHADER_READ_ONLY_OPTIMAL ---
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = img.image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				cmd,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			img.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		EndSingleTimeCommands(vulkanCore, cmd); // helper to submit and wait

		return dummyImages;
	}

	inline void TransitionImageLayout(
		std::shared_ptr<VulkanCore> VC,
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT)
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands(VC);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		// Set access masks and pipeline stages
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else {
			// Add more cases as needed
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(VC, commandBuffer);
	}
}