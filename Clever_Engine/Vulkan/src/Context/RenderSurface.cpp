#include "RenderSurface.h"

#include "Surface/CreateVulkanSurface.h"
#include "Surface/CreateSwapChain.h"
#include "Surface/CreateImageViews.h"
#include "Surface/CreateFrameBuffers.h"
#include "Surface/CreateRenderpass.h"
#include "Surface/CreateCommandBuffers.h"
#include "Surface/CreateSyncObjects.h"
#include "Surface/CreateImage.h"
#include "Scene/CreateDescriptors.h"
#include "Scene/CreatePipelines.h"
#include "Buffers/CreateBuffer.h"

namespace Vulkan {
	RenderSurface::RenderSurface(std::shared_ptr<VulkanCore> core, SurfaceFlags flags, uint8_t id) : 
		vulkanCore(std::move(core)), flags(flags), renderSurfaceID(id)
	{
		//vulkanSurface = VulkanSurface{};
	}

	void RenderSurface::InitRenderSurface(GLFWwindow* glfwWindowptr)
	{
		vulkanSurface.p_GLFWWindow = glfwWindowptr;

		if ((flags & SurfaceFlags::EnableTripleBuffer) != SurfaceFlags::None) {
			vulkanCore->MAX_FRAMES_IN_FLIGHT = 3;
		}

		CreateVulkanRenderSurface(vulkanCore, vulkanSurface, flags);
		CreateSwapchain(vulkanCore, vulkanSurface, flags);
		CreateSwapchainImages(vulkanCore, vulkanSurface, flags);//Also makes DepthVulkanImages if Depth bit is set in flags
		CreateRenderPass(vulkanCore, vulkanSurface, flags);
		CreateFrameBuffers(vulkanCore, vulkanSurface, flags);
		CreateCommandBuffers(vulkanCore, vulkanSurface);
		CreateSyncObjects(vulkanCore, vulkanSurface);

		if (vulkanScenes.size() > 0) return;
		//CREATING FIRST SCENE OF RENDERSURFACE, might want to make a way to create a new rendersurface without making a new scene



	}
	void RenderSurface::CloseRenderSurface()
	{
		CloseVulkanSurface(vulkanCore, vulkanSurface);
	}
	void RenderSurface::RecreateSwapChain()
	{
		//glfwPollEvents();
		int width = 0, height = 0;
		glfwGetFramebufferSize(vulkanSurface.p_GLFWWindow, &width, &height);

		if (width == 0 || height == 0)
		{
			//Because window is still minimized and shouldnt be processed.
			return;
		}

		vkDeviceWaitIdle(vulkanCore->vkDevice);
		CleanUpFrameBuffers(vulkanCore, vulkanSurface);
		CleanupSwapchainImages(vulkanCore, vulkanSurface);
		CleanupSwapchain(vulkanCore, vulkanSurface);
		CreateSwapchain(vulkanCore, vulkanSurface, flags);
		CreateSwapchainImages(vulkanCore, vulkanSurface, flags);
		CreateFrameBuffers(vulkanCore, vulkanSurface, flags);
		needsToBeRecreated = false;
	}

    void RenderSurface::AddRandomTriangle(uint8_t sceneID)
    {
        // Random generator
        std::random_device rd;
        std::mt19937 gen(rd());

        // X/Y in normalized device coordinates [-1, 1], Z in [0,1]
        std::uniform_real_distribution<float> distXY(-1.0f, 1.0f);
        //std::uniform_real_distribution<float> distZ(0.0f, 1.0f);

        for (int i = 0; i < 3; ++i) {
            Vertex vertex{};
            vertex.pos = glm::vec3(distXY(gen), distXY(gen), 0.5f);
            vulkanScenes[sceneID]->vertexData.push_back(vertex);
        }

        if (vulkanScenes[sceneID]->vertexData.size() == 3)
        {
            //vulkanScenes[sceneID].sceneVkBuffers.push_back(CreateDeviceLocalBuffer(vulkanCore, sizeof(vulkanScenes[sceneID].vertexData[0]) * vulkanScenes[sceneID].vertexData.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vulkanScenes[sceneID].vertexData.data()));
            vulkanScenes[sceneID]->sceneVkBuffers.push_back(CreateAndAllocateBuffer(vulkanCore, vulkanScenes[sceneID]->vertexData, BufferTypes::VertexBuffer));
        }
        else
        {
            vkWaitForFences(vulkanCore->vkDevice, vulkanSurface.surfaceVkFences.size(), vulkanSurface.surfaceVkFences.data(), VK_TRUE, UINT64_MAX);
            UpdateBuffer(vulkanCore, vulkanScenes[sceneID]->sceneVkBuffers[0], vulkanScenes[sceneID]->vertexData, BufferTypes::VertexBuffer);
            //UpdateDeviceLocalBuffer(vulkanCore, vulkanScenes[sceneID].sceneVkBuffers[0], vulkanScenes[sceneID].vertexData.data(), sizeof(vulkanScenes[sceneID].vertexData[0]) * vulkanScenes[sceneID].vertexData.size());
        }
    }
	
	uint8_t RenderSurface::CreateNewScene(uint8_t width, uint8_t height, uint8_t posx, uint8_t posy)
	{
        std::shared_ptr<VulkanScene> vulkanScene = std::make_shared<VulkanScene>();
        CreateDescriptorPool(vulkanCore, vulkanScene);
        CreateDescriptorSetLayouts(vulkanCore, vulkanScene);
        CreateDescriptorSets(vulkanCore, vulkanScene);
        CreatePipelineLayouts(vulkanCore, vulkanScene);
        CreatePipeline(vulkanCore, vulkanSurface, vulkanScene);

        vulkanScene->sceneID = nextAvailableSceneID;
        nextAvailableSceneID += 1;
        vulkanScenes.push_back(vulkanScene);
        return vulkanScene->sceneID;
	}

	void RenderSurface::RenderScene(uint8_t sceneID)
	{
        int width, height;
        glfwGetWindowSize(vulkanSurface.p_GLFWWindow, &width, &height);

        vulkanSurface.windowSize = { width, height };

        uint32_t imageIndex;
        vkAcquireNextImageKHR(
            vulkanCore->vkDevice,
            vulkanSurface.vkSwapChain,
            UINT64_MAX,
            vulkanSurface.imageAvailableSemaphores[vulkanSurface.imageFrameCounter],
            VK_NULL_HANDLE,
            &imageIndex
        );

        VkCommandBuffer cmdBuffer = vulkanSurface.surfaceVkCommandBuffers[vulkanSurface.imageFrameCounter];

        VkFence fence = vulkanSurface.surfaceVkFences[vulkanSurface.imageFrameCounter];

        vkWaitForFences(vulkanCore->vkDevice, 1, &fence, VK_TRUE, UINT64_MAX);
        vkResetFences(vulkanCore->vkDevice, 1, &fence);
        vkResetCommandBuffer(cmdBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &beginInfo);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkanSurface.vkRenderPass;
        renderPassInfo.framebuffer = vulkanSurface.surfaceVkFrameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { vulkanSurface.windowSize.x, vulkanSurface.windowSize.y };

        VkClearValue clearValue{};
        clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} }; // black
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vulkanSurface.windowSize.x);
        viewport.height = static_cast<float>(vulkanSurface.windowSize.y);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { vulkanSurface.windowSize.x, vulkanSurface.windowSize.y };
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanScenes[sceneID]->sceneVkPipelines[0]);

        if (vulkanScenes[sceneID]->vertexData.size() > 0)
        {
            VkBuffer vertexBuffers[] = { vulkanScenes[sceneID]->sceneVkBuffers[0].buffer }; // assuming first buffer is vertex
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdDraw(cmdBuffer, static_cast<uint32_t>(vulkanScenes[sceneID]->vertexData.size()), 1, 0, 0); // example: draw 3 vertices (triangle)
        }

        vkCmdEndRenderPass(cmdBuffer);

        if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }

        VkSemaphore waitSemaphores[] = { vulkanSurface.imageAvailableSemaphores[vulkanSurface.imageFrameCounter] };
        VkSemaphore signalSemaphores[] = { vulkanSurface.renderFinishedSemaphores[vulkanSurface.imageFrameCounter] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        vkResetFences(vulkanCore->vkDevice, 1, &fence);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;


        vkQueueSubmit(vulkanCore->graphicsQueue, 1, &submitInfo, vulkanSurface.surfaceVkFences[vulkanSurface.imageFrameCounter]);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vulkanSurface.vkSwapChain;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(vulkanCore->presentQueue, &presentInfo);

        // Advance frame counter
        vulkanSurface.imageFrameCounter = (vulkanSurface.imageFrameCounter + 1) % vulkanCore->MAX_FRAMES_IN_FLIGHT;

	}
}