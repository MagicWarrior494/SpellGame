#include "VulkanWindow.h"
#include <GLFW/glfw3.h>
#include <filesystem>

#include "VulkanCore.h"
#include "VulkanContructors/Surface.h"
#include "VulkanContructors/Renderpass.h"
#include "VulkanContructors/Swapchain.h"
#include "VulkanContructors/FrameBuffer.h"
#include "VulkanContructors/CommandPool.h"
#include "VulkanContructors/CommandBuffer.h"
#include "VulkanContructors/SyncObjects.h"
#include "VulkanContructors/Sampler.h"
#include "VulkanContructors/Descriptors.h"
#include "VulkanContructors/Pipeline.h"

namespace Vulkan
{
	VulkanWindow::VulkanWindow(std::shared_ptr<VulkanCore> vulkanCore, void* windowPtr)
	{
        this->vulkanCore = vulkanCore;
		this->windowPtr = windowPtr;

		if ((flags & SurfaceFlags::EnableTripleBuffer) != SurfaceFlags::None) {
			MAX_FRAMES_IN_FLIGHT = 3;
		}

		//Sets Window Size based on the actual window from GLFW
		int width = 0, height = 0;
		glfwGetFramebufferSize((GLFWwindow*)windowPtr, &width, &height);
		windowSize = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		CreateVulkanSurface(vulkanCore.get(), windowPtr, surfaceSurface);
		surfaceRenderPass = CreateRenderPass(vulkanCore.get(), VK_FORMAT_B8G8R8A8_UNORM,
			(flags & SurfaceFlags::EnableDepth) != SurfaceFlags::None,
			VK_FORMAT_D32_SFLOAT,
			RenderPassType::Onscreen);

		SwapChainCreateInfo swapChainCreateInfo{};
		swapChainCreateInfo.windowPtr = windowPtr;
		swapChainCreateInfo.surface = surfaceSurface;
		swapChainCreateInfo.windowSize = windowSize;
		swapChainCreateInfo.swapChainImageFormat = surfaceSwapChainImageFormat;
		swapChainCreateInfo.flags = flags;
		CreateSwapchain(vulkanCore.get(), swapChainCreateInfo, surfaceSwapChain);

		surfaceColorImages = CreateSwapchainImages(vulkanCore.get(), surfaceSwapChain, surfaceSwapChainImageFormat,
			windowSize, surfaceType);

		surfaceFrameBuffers = CreateFrameBuffers(
            vulkanCore.get(),
			surfaceRenderPass,
			surfaceColorImages,
			surfaceDepthImages,
			windowSize.x,
			windowSize.y
		);
		surfaceCommandPool = CreateCommandPool(vulkanCore.get());
		surfacePresentCommandBuffers = CreateCommandBuffers(
            vulkanCore.get(),
			surfaceCommandPool,
			static_cast<uint32_t>(surfaceFrameBuffers.size())
		);
		CreateSyncObjects(
            vulkanCore.get(),
			MAX_FRAMES_IN_FLIGHT,
			surfaceImageAvailableSemaphores,
			surfaceRenderFinishedSemaphores,
			surfaceFences
		);

		offscreenSampler = CreateSampler(vulkanCore.get(), SamplerConfig::Offscreen());

		descriptorSetInfo.bindings.clear();
		descriptorSetInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		DescriptorBindingInfo bindinginfo{};
		bindinginfo.binding = 0;
		bindinginfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindinginfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindinginfo.count = 0;

		descriptorSetInfo.bindings.push_back(bindinginfo);

		DescriptorBindingInfo& binding = descriptorSetInfo.bindings[0];

		for (int i = 0; i < MAX_SCENES; i++)
		{
			std::vector<VulkanImage> newSceneImages = initImageByType(
                vulkanCore.get(),
				ImageType::Color,
				1,
				1,
				MAX_FRAMES_IN_FLIGHT,
				VK_SAMPLE_COUNT_1_BIT,
				VK_FORMAT_UNDEFINED
			);
			for (auto& img : newSceneImages) {
				TransitionImageLayout(
                    vulkanCore.get(),
					img.image,
					img.format,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VK_IMAGE_ASPECT_COLOR_BIT
				);
			}
			sceneImages.push_back(newSceneImages);

			for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++) {
				VkDescriptorImageInfo info{};
				info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				info.imageView = newSceneImages[frame].view;
				info.sampler = offscreenSampler;

				binding.images.push_back(info);
			}
		}
		binding.count = static_cast<uint32_t>(binding.images.size());

		//This creates the offscreen descriptors
		surfaceDescriptorResult = CreateDescriptors(vulkanCore.get(), descriptorSetInfo);


		PipelineLayoutInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.setLayouts.push_back(surfaceDescriptorResult.layout);

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SurfacePushConstants);

		pipelineLayoutInfo.pushConstants.push_back(pushConstantRange);
		surfacePipelineLayout = CreatePipelineLayout(vulkanCore.get(), pipelineLayoutInfo);

		PipelineInfo pipelineInfo{};
		pipelineInfo.vertShaderPath = std::filesystem::current_path().string() + "\\Vulkan\\res\\surfaceVert.spv";
		pipelineInfo.fragShaderPath = std::filesystem::current_path().string() + "\\Vulkan\\res\\surfaceFrag.spv";
		pipelineInfo.pipelineLayout = surfacePipelineLayout;
		pipelineInfo.renderPass = surfaceRenderPass;
		pipelineInfo.cullMode = VK_CULL_MODE_NONE;
		surfacePipeline = CreateGraphicsPipeline(vulkanCore.get(), pipelineInfo);
	}

	bool VulkanWindow::Render()
	{
        VkDevice device = vulkanCore->vkDevice;

        VkFence frameFence = surfaceFences[imageFrameCounter];
        vkWaitForFences(device, 1, &frameFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &frameFence);

        uint32_t swapchainImageIndex;
        VkResult acquireResult = vkAcquireNextImageKHR(
            device,
            surfaceSwapChain,
            UINT64_MAX,
            surfaceImageAvailableSemaphores[imageFrameCounter],
            VK_NULL_HANDLE,
            &swapchainImageIndex
        );

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR)
        {
            return false;
        }
        else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // --- 4. Render each scene to offscreen framebuffer ---
        std::vector<VkSemaphore> sceneFinishedSemaphores;
        for (auto& scene : vulkanScenes)
        {
            if (!scene->Render())
            {
                //Scene Render failed, handle it
            }
        }

        VkCommandBuffer cmdBuffer = surfacePresentCommandBuffers[imageFrameCounter];
        vkResetCommandBuffer(cmdBuffer, 0);

        VkCommandBufferBeginInfo beginInfoSurface{};
        beginInfoSurface.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfoSurface.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuffer, &beginInfoSurface);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = surfaceRenderPass;
        renderPassInfo.framebuffer = surfaceFrameBuffers[swapchainImageIndex];
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = { windowSize.x, windowSize.y };

        VkClearValue clearValue{};
        clearValue.color = { 0.1f, 0.2f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewportSurface{ 0.0f, 0.0f, static_cast<float>(windowSize.x), static_cast<float>(windowSize.y), 0.0f, 1.0f };
        vkCmdSetViewport(cmdBuffer, 0, 1, &viewportSurface);

        VkRect2D scissorSurface{ {0,0}, {windowSize.x, windowSize.y} };
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissorSurface);

        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, surfacePipeline);
        vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            surfacePipelineLayout,
            0, 1,
            &surfaceDescriptorResult.sets[imageFrameCounter],
            0, nullptr
        );

        // Draw each scene texture
        for (auto& scene : vulkanScenes)
        {
            VkViewport sceneViewport{
                static_cast<float>(scene->sceneOffset.x),
                static_cast<float>(scene->sceneOffset.y),
                static_cast<float>(scene->sceneSize.x),
                static_cast<float>(scene->sceneSize.y),
                0.0f,
                1.0f
            };
            vkCmdSetViewport(cmdBuffer, 0, 1, &sceneViewport);

            VkRect2D sceneScissor{ {scene->sceneOffset.x, scene->sceneOffset.y}, {scene->sceneSize.x, scene->sceneSize.y} };
            vkCmdSetScissor(cmdBuffer, 0, 1, &sceneScissor);

            vkCmdPushConstants(
                cmdBuffer,
                surfacePipelineLayout,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SurfacePushConstants),
                &scene->GetAssignedWindowId()
            );

            vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
        }

        vkCmdEndRenderPass(cmdBuffer);
        vkEndCommandBuffer(cmdBuffer);

        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkPipelineStageFlags> waitStages;

        waitSemaphores.push_back(surfaceImageAvailableSemaphores[imageFrameCounter]);
        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        for (auto& sem : sceneFinishedSemaphores)
        {
            waitSemaphores.push_back(sem);
            waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &surfaceRenderFinishedSemaphores[imageFrameCounter];

        vkQueueSubmit(vulkanCore->vkGraphicsQueue, 1, &submitInfo, frameFence);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &surfaceRenderFinishedSemaphores[imageFrameCounter];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &surfaceSwapChain;
        presentInfo.pImageIndices = &swapchainImageIndex;
        vkQueuePresentKHR(vulkanCore->vkPresentQueue, &presentInfo);

        imageFrameCounter = (imageFrameCounter + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}
