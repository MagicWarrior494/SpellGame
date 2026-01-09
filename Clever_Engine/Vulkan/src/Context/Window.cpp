#include "Window.h"
#include "Buffers/CreateBuffer.h"
#include <iostream>

namespace Vulkan {
	Window::Window(std::shared_ptr<VulkanCore> core, SurfaceFlags flags, uint8_t id) : 
		vulkanCore(std::move(core)), flags(flags), surfaceId(id)
	{
		//vulkanSurface = VulkanSurface{};
	}

	void Window::InitWindow(GLFWwindow* glfwWindowptr)
	{
		vulkanSurface.p_GLFWWindow = glfwWindowptr;

		if ((flags & SurfaceFlags::EnableTripleBuffer) != SurfaceFlags::None) {
			vulkanSurface.MAX_FRAMES_IN_FLIGHT = 3;
		}

        vulkanSurface.CreateSurfaceResources(vulkanCore, glfwWindowptr);
		if (vulkanScenes.size() > 0) return;
		//CREATING FIRST SCENE OF Window, might want to make a way to create a new Window without making a new scene

	}
	void Window::CloseWindow()
	{
        vulkanSurface.Destroy(vulkanCore);
	}

    void Window::AddRandomTriangle()
    {
        for (auto& [sceneID, scene_ptr] : vulkanScenes)
        {
			auto& scene = *scene_ptr;
            // Random generator
            std::random_device rd;
            std::mt19937 gen(rd());

            // X/Y in normalized device coordinates [-1, 1], Z in [0,1]
            std::uniform_real_distribution<float> distXY(-1.0f, 1.0f);
            //std::uniform_real_distribution<float> distZ(0.0f, 1.0f);

            for (int i = 0; i < 3; ++i) {
                Vertex vertex{};
                vertex.pos = glm::vec3(distXY(gen), distXY(gen), 0.5f);
                scene.vertexData.push_back(vertex);
            }

            if (scene.vertexData.size() == 3)
            {
                scene.sceneBuffers.push_back(CreateAndAllocateBuffer(vulkanCore, scene.vertexData, BufferTypes::VertexBuffer));
            }
            else
            {
                vkWaitForFences(vulkanCore->vkDevice, scene.sceneFences.size(), scene.sceneFences.data(), VK_TRUE, UINT64_MAX);
                UpdateBuffer(vulkanCore, scene.sceneBuffers[0], scene.vertexData, BufferTypes::VertexBuffer);
            }
        }
    }
	
    void Window::resizeScenes()
    {
        for (VkFence fence : vulkanSurface.surfaceFences)
        {
            vkWaitForFences(vulkanCore->vkDevice, 1, &fence, VK_TRUE, UINT64_MAX);
        }

        size_t sceneCount = vulkanScenes.size();
        if (sceneCount == 0) return;

        int actualWidth, actualHeight;
        glfwGetWindowSize(vulkanSurface.p_GLFWWindow, &actualWidth, &actualHeight);

        // Calculate grid size (rows and columns)
        size_t cols = static_cast<size_t>(ceil(sqrt(sceneCount)));
        size_t rows = static_cast<size_t>(ceil(double(sceneCount) / cols));

        uint32_t sceneWidth = actualWidth / cols;
        uint32_t sceneHeight = actualHeight / rows;

        for (size_t i = 0; i < sceneCount; ++i)
        {
            auto& scene = vulkanScenes[i];
            scene->width = sceneWidth;
            scene->height = sceneHeight;
            // Optionally, you can also set position if needed
            scene->xoffset = (i % cols) * sceneWidth;
            scene->yoffset = (i / cols) * sceneHeight;
            scene->ResizeScene(vulkanCore, &vulkanSurface, sceneWidth, sceneHeight, scene->xoffset, scene->yoffset);
        }
    }

    uint8_t Window::CreateNewScene(uint32_t width, uint32_t height, uint32_t posx, uint32_t posy)
    {
        width = (width == 0) ? vulkanSurface.windowSize.x : width;
		height = (height == 0) ? vulkanSurface.windowSize.y : height;
		posx = (posx == 0) ? 0 : posx;
		posy = (posy == 0) ? 0 : posy;

        // --- 1. Create a shared_ptr directly ---
        auto scenePtr = std::make_shared<VulkanScene>();

        // --- 2. Assign ID and offsets/size ---
        scenePtr->sceneID = GetNextSceneID();
        scenePtr->width = width;
        scenePtr->height = height;
        scenePtr->xoffset = posx;
        scenePtr->yoffset = posy;

        // --- 3. Create Vulkan resources ---
        scenePtr->CreateSceneResources(vulkanCore, &vulkanSurface);

        // --- 4. Add to scene list ---
        vulkanScenes.insert({ scenePtr->sceneID, scenePtr });

        // --- 5. Return the scene ID ---
        return scenePtr->sceneID;
    }

    void Window::Render()
    {
        // --- 0. Update window size ---
        int width, height;
        glfwGetWindowSize(vulkanSurface.p_GLFWWindow, &width, &height);
        vulkanSurface.windowSize = { width, height };

        VkDevice device = vulkanCore->vkDevice;

        // --- 1. Recreate swapchain if needed ---
        if (needsToBeRecreated)
        {
            vulkanSurface.RecreateSwapchain(vulkanCore);
            needsToBeRecreated = false;
        }

        // --- 2. Wait for fence for this frame ---
        VkFence frameFence = vulkanSurface.surfaceFences[vulkanSurface.imageFrameCounter];
        vkWaitForFences(device, 1, &frameFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &frameFence);

        // --- 3. Acquire next swapchain image ---
        uint32_t swapchainImageIndex;
        VkResult acquireResult = vkAcquireNextImageKHR(
            device,
            vulkanSurface.surfaceSwapChain,
            UINT64_MAX,
            vulkanSurface.surfaceImageAvailableSemaphores[vulkanSurface.imageFrameCounter],
            VK_NULL_HANDLE,
            &swapchainImageIndex
        );

        if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR)
        {
            needsToBeRecreated = true;
            return;
        }
        else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        // --- 4. Render each scene to offscreen framebuffer ---
        std::vector<VkSemaphore> sceneFinishedSemaphores;
        for (auto& [sceneID, scene] : vulkanScenes)
        {
            if (!scene) continue;
            VkCommandBuffer sceneCmd = scene->sceneCommandBuffers[*scene->imageFrameCounter];
            VkFence sceneFence = scene->sceneFences[*scene->imageFrameCounter];

            vkWaitForFences(device, 1, &sceneFence, VK_TRUE, UINT64_MAX);
            vkResetFences(device, 1, &sceneFence);
            vkResetCommandBuffer(sceneCmd, 0);

            // Begin offscreen command buffer
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(sceneCmd, &beginInfo);

            // Offscreen render pass
            VkRenderPassBeginInfo offscreenPassInfo{};
            offscreenPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            offscreenPassInfo.renderPass = scene->sceneRenderPass;
            offscreenPassInfo.framebuffer = scene->sceneOffscreenFrameBuffers[*scene->imageFrameCounter];
            offscreenPassInfo.renderArea.offset = { 0, 0 };
            offscreenPassInfo.renderArea.extent = { scene->width, scene->height };

            VkClearValue clearValues[2];
            clearValues[0].color = { 0.0f, 0.0f, 0.4f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };
            offscreenPassInfo.clearValueCount = 2;
            offscreenPassInfo.pClearValues = clearValues;

            vkCmdBeginRenderPass(sceneCmd, &offscreenPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{ 0.0f, 0.0f, static_cast<float>(scene->width), static_cast<float>(scene->height), 0.0f, 1.0f };
            vkCmdSetViewport(sceneCmd, 0, 1, &viewport);

            VkRect2D scissor{ {0,0}, {scene->width, scene->height} };
            vkCmdSetScissor(sceneCmd, 0, 1, &scissor);

            vkCmdBindPipeline(sceneCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, scene->scenePipelines[0]);
            if (scene->SceneDescriptorResult.layout != VK_NULL_HANDLE)
            {
                vkCmdBindDescriptorSets(
                    sceneCmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    scene->scenePipelineLayouts[0],
                    0, 1,
                    &scene->SceneDescriptorResult.sets[*scene->imageFrameCounter],
                    0, nullptr
                );
            }

            if (!scene->vertexData.empty())
            {
                VkBuffer vertexBuffers[] = { scene->sceneBuffers[0].buffer };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(sceneCmd, 0, 1, vertexBuffers, offsets);
                vkCmdDraw(sceneCmd, static_cast<uint32_t>(scene->vertexData.size()), 1, 0, 0);
            }

            vkCmdEndRenderPass(sceneCmd);

            vkEndCommandBuffer(sceneCmd);

            // Submit offscreen command buffer
            VkSubmitInfo sceneSubmitInfo{};
            sceneSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            sceneSubmitInfo.commandBufferCount = 1;
            sceneSubmitInfo.pCommandBuffers = &sceneCmd;
            sceneSubmitInfo.signalSemaphoreCount = 1;
            sceneSubmitInfo.pSignalSemaphores = &scene->sceneRenderFinishedSemaphores[*scene->imageFrameCounter];

            vkQueueSubmit(vulkanCore->graphicsQueue, 1, &sceneSubmitInfo, sceneFence);
            sceneFinishedSemaphores.push_back(scene->sceneRenderFinishedSemaphores[*scene->imageFrameCounter]);
        }

        // --- 5. Record surface command buffer ---
        VkCommandBuffer cmdBuffer = vulkanSurface.surfacePresentCommandBuffers[vulkanSurface.imageFrameCounter];
        vkResetCommandBuffer(cmdBuffer, 0);

        VkCommandBufferBeginInfo beginInfoSurface{};
        beginInfoSurface.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfoSurface.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuffer, &beginInfoSurface);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkanSurface.surfaceRenderPass;
        renderPassInfo.framebuffer = vulkanSurface.surfaceFrameBuffers[swapchainImageIndex];
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = { vulkanSurface.windowSize.x, vulkanSurface.windowSize.y };

        VkClearValue clearValue{};
        clearValue.color = { 0.1f, 0.2f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewportSurface{ 0.0f, 0.0f, static_cast<float>(vulkanSurface.windowSize.x), static_cast<float>(vulkanSurface.windowSize.y), 0.0f, 1.0f };
        vkCmdSetViewport(cmdBuffer, 0, 1, &viewportSurface);

        VkRect2D scissorSurface{ {0,0}, {vulkanSurface.windowSize.x, vulkanSurface.windowSize.y} };
        vkCmdSetScissor(cmdBuffer, 0, 1, &scissorSurface);

        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanSurface.surfacePipeline);
        vkCmdBindDescriptorSets(
            cmdBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vulkanSurface.surfacePipelineLayout,
            0, 1,
            &vulkanSurface.SurfaceDescriptorResult.sets[vulkanSurface.imageFrameCounter],
            0, nullptr
        );

        // Draw each scene texture
        for (auto& [sceneID, scene] : vulkanScenes)
        {
            VkViewport sceneViewport{
                static_cast<float>(scene->xoffset),
                static_cast<float>(scene->yoffset),
                static_cast<float>(scene->width),
                static_cast<float>(scene->height),
                0.0f,
                1.0f
            };
            vkCmdSetViewport(cmdBuffer, 0, 1, &sceneViewport);

            VkRect2D sceneScissor{ {scene->xoffset, scene->yoffset}, {scene->width, scene->height} };
            vkCmdSetScissor(cmdBuffer, 0, 1, &sceneScissor);

            vkCmdPushConstants(
                cmdBuffer,
                vulkanSurface.surfacePipelineLayout,
                VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SurfacePushConstants),
                &scene->sceneID
            );

            vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
        }

        vkCmdEndRenderPass(cmdBuffer);
        vkEndCommandBuffer(cmdBuffer);

        // --- 6. Submit surface command buffer ---
        std::vector<VkSemaphore> waitSemaphores;
        std::vector<VkPipelineStageFlags> waitStages;

        // Wait for swapchain image
        waitSemaphores.push_back(vulkanSurface.surfaceImageAvailableSemaphores[vulkanSurface.imageFrameCounter]);
        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        // Wait for all scene offscreen passes
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
        submitInfo.pSignalSemaphores = &vulkanSurface.surfaceRenderFinishedSemaphores[vulkanSurface.imageFrameCounter];

        vkQueueSubmit(vulkanCore->graphicsQueue, 1, &submitInfo, frameFence);

        // --- 7. Present ---
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &vulkanSurface.surfaceRenderFinishedSemaphores[vulkanSurface.imageFrameCounter];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &vulkanSurface.surfaceSwapChain;
        presentInfo.pImageIndices = &swapchainImageIndex;
        vkQueuePresentKHR(vulkanCore->presentQueue, &presentInfo);

        vulkanSurface.imageFrameCounter = (vulkanSurface.imageFrameCounter + 1) % vulkanSurface.MAX_FRAMES_IN_FLIGHT;
    }
}