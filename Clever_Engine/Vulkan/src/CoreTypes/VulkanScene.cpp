#include "VulkanScene.h"
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

#include "Management/RecreateSwapchain.h"

namespace Vulkan
{
    VulkanScene::VulkanScene(std::shared_ptr<VulkanCore> vulkanCore, int width, int height, int xoffset, int yoffset, VulkanWindow* parentWindow)
    {
		this->height = height;
		this->width = width;
		this->xoffset = xoffset;
		this->yoffset = yoffset;
        this->vulkanCore = vulkanCore;
		this->parentWindow = parentWindow;

        renderImageIndex = parentWindow->AddNewScene(width, height);

        sceneRenderPass = CreateRenderPass(
            vulkanCore.get(),
            VK_FORMAT_B8G8R8A8_UNORM,
            false,
            VK_FORMAT_D32_SFLOAT,
            RenderPassType::Offscreen
        );

        sceneFrameBuffers = CreateFrameBuffers(
            vulkanCore.get(),
            sceneRenderPass,
            parentWindow->GetSceneImages(renderImageIndex),
            { scenedepthAttachment },
            width,
            height
        );

        CreateSyncObjects(
            vulkanCore.get(),
            parentWindow->GetMaxFramesInFlight(),
            sceneImageAvailableSemaphores,
            sceneRenderFinishedSemaphores,
            sceneFences
        );
    }

    VulkanScene::~VulkanScene()
    {
        if (!vulkanCore || !vulkanCore->vkDevice) return;

        // 1. Wait for GPU to finish all pending work
        vkDeviceWaitIdle(vulkanCore->vkDevice);

        // 2. Synchronization Primitives (Semaphores and Fences)
        for (size_t i = 0; i < surfaceImageAvailableSemaphores.size(); i++) {
            vkDestroySemaphore(vulkanCore->vkDevice, surfaceImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanCore->vkDevice, surfaceRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(vulkanCore->vkDevice, surfaceFences[i], nullptr);
        }

        // 3. Command Pool (Automatically destroys all command buffers allocated from it)
        if (surfaceCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(vulkanCore->vkDevice, surfaceCommandPool, nullptr);
        }

        // 4. Framebuffers (Must be destroyed before the Render Pass and Image Views)
        for (auto framebuffer : surfaceFrameBuffers) {
            vkDestroyFramebuffer(vulkanCore->vkDevice, framebuffer, nullptr);
        }

        // 5. Graphics Pipeline & Layout
        if (surfacePipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(vulkanCore->vkDevice, surfacePipeline, nullptr);
        }
        if (surfacePipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vulkanCore->vkDevice, surfacePipelineLayout, nullptr);
        }

        // 6. Render Pass
        if (surfaceRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(vulkanCore->vkDevice, surfaceRenderPass, nullptr);
        }

        for (auto& imageframe : sceneImages) {
            for (auto& image : imageframe) {
                image.Destory(vulkanCore->vkDevice);
            }
        }

        for (auto& img : surfaceColorImages) {
            if (img.view != VK_NULL_HANDLE) {
                vkDestroyImageView(vulkanCore->vkDevice, img.view, nullptr);
            }
        }

        // Depth images ARE created by you, so they MUST be destroyed.
        for (auto& img : surfaceDepthImages) {
            img.Destory(vulkanCore->vkDevice);
        }

        if (offscreenSampler != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanCore->vkDevice, offscreenSampler, nullptr);
        }

        if (surfaceSwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vulkanCore->vkDevice, surfaceSwapChain, nullptr);
        }

        if (surfaceSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(vulkanCore->vkInstance, surfaceSurface, nullptr);
        }

        vkDestroyDescriptorPool(vulkanCore->vkDevice, surfaceDescriptorResult.pool, nullptr);
    }
    bool VulkanScene::Render()
    {
        VkDevice device = vulkanCore->vkDevice;

        VkFence frameFence = surfaceFences[imageFrameCounter];
        vkWaitForFences(device, 1, &frameFence, VK_TRUE, UINT64_MAX);


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
            glfwGetWindowSize((GLFWwindow*)windowPtr, (int*)&windowSize.x, (int*)&windowSize.y);
            RecreateSwapchain();
            return false;
        }
        else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        vkResetFences(device, 1, &frameFence);

        std::vector<VkSemaphore> sceneFinishedSemaphores;
        //for (auto& scene : vulkanScenes)
        //{
        //    if (!scene->Render())
        //    {
        //        //Scene Render failed, handle it
        //    }
        //}

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
        /*for (auto& scene : vulkanScenes)
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
        }*/

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
