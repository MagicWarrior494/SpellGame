#pragma once
#include "Context/ContextVulkanData.h"
#include "CreateImage.h"

#include <vector>

namespace Vulkan {
	enum class RenderPassType {
		Surface,   // Render directly to swapchain
		Offscreen  // Render to offscreen image(s)
	};

	inline VkFormat findSupportedFormat(std::shared_ptr<VulkanCore> vulkanCore, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(vulkanCore->vkPhysicalDevice, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	inline VkFormat findDepthFormat(std::shared_ptr<VulkanCore> vulkanCore) {
		return findSupportedFormat
		(
			vulkanCore,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

    inline VkRenderPass CreateRenderPass(
        std::shared_ptr<VulkanCore> vulkanCore,
        VkFormat colorFormat,
        bool useDepth,
        VkFormat depthFormat,
        RenderPassType type)
    {
        std::vector<VkAttachmentDescription> attachments{};
        std::vector<VkAttachmentReference> colorAttachmentRefs{};

        // --- Color attachment ---
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = colorFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (type == RenderPassType::Surface) {
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        else {
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        attachments.push_back(colorAttachment);

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = static_cast<uint32_t>(attachments.size() - 1);
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentRefs.push_back(colorAttachmentRef);

        // --- Depth attachment (optional) ---
        VkAttachmentReference depthAttachmentRef{};
        bool hasDepth = useDepth;
        if (hasDepth) {
            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = depthFormat;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachments.push_back(depthAttachment);

            depthAttachmentRef.attachment = static_cast<uint32_t>(attachments.size() - 1);
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        // --- Subpass ---
        VkSubpassDescription subpassDesc{};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
        subpassDesc.pColorAttachments = colorAttachmentRefs.data();
        subpassDesc.pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr;

        // --- Subpass dependencies (TWO) ---
        VkSubpassDependency dependencyInit{};
        dependencyInit.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencyInit.dstSubpass = 0;
        dependencyInit.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencyInit.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencyInit.srcAccessMask = 0;
        dependencyInit.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencyInit.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


        VkSubpassDependency dependencyRelease{};
        dependencyRelease.srcSubpass = 0;
        dependencyRelease.dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencyRelease.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencyRelease.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencyRelease.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencyRelease.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencyRelease.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        std::array<VkSubpassDependency, 2> dependencies = { dependencyInit, dependencyRelease };

        // --- Create RenderPass ---
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDesc;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        VkRenderPass renderPass = VK_NULL_HANDLE;
        if (vkCreateRenderPass(vulkanCore->vkDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create RenderPass");
        }

        return renderPass;
    }
}