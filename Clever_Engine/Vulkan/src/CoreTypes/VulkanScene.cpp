#include "VulkanScene.h"
#include "VulkanWindow.h"
#include "VulkanContructors/Renderpass.h"
#include "VulkanContructors/FrameBuffer.h"
#include "VulkanContructors/SyncObjects.h"
#include "VulkanContructors/CommandPool.h"
#include "VulkanContructors/CommandBuffer.h"
#include "Image/VulkanImage.h"

namespace Vulkan
{
    VulkanScene::VulkanScene(std::shared_ptr<VulkanCore> vulkanCore, int width, int height, int xoffset, int yoffset, VulkanWindow* parentWindow)
        : vulkanCore(vulkanCore)
        , parentWindow(parentWindow)
        , width(static_cast<uint32_t>(width))
        , height(static_cast<uint32_t>(height))
    {
        renderImageIndex = parentWindow->AddNewScene(width, height);

        sceneRenderPass = CreateRenderPass(
            vulkanCore.get(),
            VK_FORMAT_B8G8R8A8_UNORM,
            false,
            VK_FORMAT_D32_SFLOAT,
            RenderPassType::Offscreen
        );

        scenedepthAttachment = initImageByType(
            vulkanCore.get(),
            ImageType::Depth,
            this->width,
            this->height,
            parentWindow->GetMaxFramesInFlight(),
            VK_SAMPLE_COUNT_1_BIT,
            VK_FORMAT_D32_SFLOAT
        );

        sceneFrameBuffers = CreateFrameBuffers(
            vulkanCore.get(),
            sceneRenderPass,
            parentWindow->GetSceneImages(renderImageIndex),
            scenedepthAttachment,
            this->width,
            this->height
        );

        CreateSyncObjects(
            vulkanCore.get(),
            parentWindow->GetMaxFramesInFlight(),
            sceneImageAvailableSemaphores,
            sceneRenderFinishedSemaphores,
            sceneFences
        );

        // NEW: per-scene command resources
        sceneCommandPool = CreateCommandPool(vulkanCore.get());
        sceneCommandBuffers = CreateCommandBuffers(
            vulkanCore.get(),
            sceneCommandPool,
            static_cast<uint32_t>(parentWindow->GetMaxFramesInFlight())
        );
    }

    VulkanScene::~VulkanScene()
    {
        if (!vulkanCore || !vulkanCore->vkDevice) return;
        vkDeviceWaitIdle(vulkanCore->vkDevice);

        if (sceneCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(vulkanCore->vkDevice, sceneCommandPool, nullptr);
        }

        for (size_t i = 0; i < sceneImageAvailableSemaphores.size(); i++) {
            vkDestroySemaphore(vulkanCore->vkDevice, sceneImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanCore->vkDevice, sceneRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(vulkanCore->vkDevice, sceneFences[i], nullptr);
        }

        for (auto framebuffer : sceneFrameBuffers) {
            vkDestroyFramebuffer(vulkanCore->vkDevice, framebuffer, nullptr);
        }

        if (sceneRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(vulkanCore->vkDevice, sceneRenderPass, nullptr);
        }

        for (auto& depthAttachment : scenedepthAttachment) {
            depthAttachment.Destroy(vulkanCore->vkDevice);
        }
    }

    VkCommandBuffer VulkanScene::BeginFrame(uint32_t frameIndex)
    {
        vkWaitForFences(vulkanCore->vkDevice, 1, &sceneFences[frameIndex], VK_TRUE, UINT64_MAX);
        vkResetFences(vulkanCore->vkDevice, 1, &sceneFences[frameIndex]);

        VkCommandBuffer cmd = sceneCommandBuffers.at(frameIndex);
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassInfo.renderPass = sceneRenderPass;
        renderPassInfo.framebuffer = sceneFrameBuffers.at(frameIndex);
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { width, height };

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { { 0.01f, 0.01f, 0.01f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{ {0, 0}, {width, height} };
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        return cmd;
    }

    void VulkanScene::EndFrame(VkCommandBuffer cmd)
    {
        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);
    }
}
