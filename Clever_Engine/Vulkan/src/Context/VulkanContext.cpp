#include "VulkanContext.h"
#include "Core/Instance.h"
#include "Core/LogicalDevice.h"

#include "Core/PhysicalDevice.h"
#include "Core/CommandPool.h"


#include "Surface/CreateVulkanSurface.h"
#include "Surface/CreateSwapChain.h"
#include "Surface/CreateImageViews.h"
#include "Surface/CreateFrameBuffers.h"
#include "Surface/CreateRenderpass.h"
#include "Surface/CreateCommandBuffers.h"
#include "Surface/CreateSyncObjects.h"
#include "Surface/CreateImage.h"


namespace Vulkan {
	void VulkanContext::Init() {
		vulkanCore = std::make_shared<VulkanCore>();

		CreateVulkanInstance(vulkanCore);
		CreatePhysicalDevice(vulkanCore);
		CreateLogicalDevice(vulkanCore);//This also makes the DebugUtilsMessengerEXT object and the graphics and present Queue
		CreateCommandPool(vulkanCore);
	}

	VkResult VulkanContext::RenderWindow(std::shared_ptr<Window> window)
	{
		VulkanSurface& vulkanSurface = window->renderSurface.vulkanSurface;

		vkWaitForFences(
			vulkanCore->vkDevice,
			1,
			&vulkanSurface.surfaceVkFences[0],
			VK_TRUE,
			UINT64_MAX
		);

		// Acquire image
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(
			vulkanCore->vkDevice,
			vulkanSurface.vkSwapChain,
			UINT64_MAX,
			vulkanSurface.imageAvailableSemaphores[0],
			VK_NULL_HANDLE,
			&imageIndex
		);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			window->renderSurface.resized = true;
			return result; // Swapchain out of date, needs recreation
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swapchain image!");
		}

		// --- Record a tiny command buffer to transition image layout ---
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vulkanCore->VkCommandPools[0];
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(vulkanCore->vkDevice, &allocInfo, &cmdBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &beginInfo);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = vulkanSurface.surfaceSwapchainImages[imageIndex].image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		// No need to wait on anything if it’s undefined - present
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkEndCommandBuffer(cmdBuffer);

		// --- Submit the layout transition before presenting ---
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { vulkanSurface.imageAvailableSemaphores[0] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VkSemaphore signalSemaphores[] = { vulkanSurface.renderFinishedSemaphores[0] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(vulkanCore->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit layout transition!");
		}

		// Present the image
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &vulkanSurface.vkSwapChain;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(vulkanCore->presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			window->renderSurface.resized = true;
			return VK_ERROR_OUT_OF_DATE_KHR;
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swapchain image!");
		}

		vkQueueWaitIdle(vulkanCore->presentQueue);
		vkFreeCommandBuffers(vulkanCore->vkDevice, vulkanCore->VkCommandPools[0], 1, &cmdBuffer);

		return VK_SUCCESS;
	}

	void VulkanContext::Update()
	{
		if (windows.size() == 0)
			return;

		glfwPollEvents();
		for (auto it = windows.begin(); it != windows.end();) {
			RenderSurface& surface = it->second->renderSurface;
			if (glfwWindowShouldClose(surface.vulkanSurface.p_GLFWWindow)) {
				CloseVulkanRenderSurface(vulkanCore, surface.vulkanSurface);
				it = windows.erase(it);
			}
			else {
				it++;
			}
		}
		for (auto it = windows.begin(); it != windows.end(); it++) {
			RenderSurface& surface = it->second->renderSurface;
			if (surface.resized) {
				int width = 0, height = 0;
				glfwGetFramebufferSize(surface.vulkanSurface.p_GLFWWindow, &width, &height);

				// Wait until window is not minimized
				while (width == 0 || height == 0) {
					glfwWaitEvents();
					glfwGetFramebufferSize(surface.vulkanSurface.p_GLFWWindow, &width, &height);
				}
				vkDeviceWaitIdle(vulkanCore->vkDevice);
				CleanUpFrameBuffers(vulkanCore, surface.vulkanSurface);
				CleanupSwapchainImages(vulkanCore, surface.vulkanSurface);
				CleanupSwapchain(vulkanCore, surface.vulkanSurface);
				CreateSwapchain(vulkanCore, surface.vulkanSurface, surface.flags);
				CreateSwapchainImages(vulkanCore, surface.vulkanSurface, surface.flags);
				CreateFrameBuffers(vulkanCore, surface.vulkanSurface, surface.flags);
				surface.resized = false;
			}
		}
	}

	uint8_t VulkanContext::CreateNewWindow(SurfaceFlags flags)
	{
		std::shared_ptr<Window> window = std::make_shared<Window>(vulkanCore, flags);
		windows.insert({ nextWindowID, std::move(window)});
		nextWindowID++;
		return nextWindowID - 1;
	}
}