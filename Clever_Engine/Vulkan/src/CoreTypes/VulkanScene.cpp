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

#include "Shader/Shader.h"

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
        for (size_t i = 0; i < sceneImageAvailableSemaphores.size(); i++) {
            vkDestroySemaphore(vulkanCore->vkDevice, sceneImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanCore->vkDevice, sceneRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(vulkanCore->vkDevice, sceneFences[i], nullptr);
        }

        // 3. Framebuffers (Must be destroyed before the Render Pass and Image Views)
        for (auto framebuffer : sceneFrameBuffers) {
            vkDestroyFramebuffer(vulkanCore->vkDevice, framebuffer, nullptr);
        }

        // 4. Graphics Pipeline & Layout
        if (scenePipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(vulkanCore->vkDevice, scenePipeline, nullptr);
        }
        if (scenePipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vulkanCore->vkDevice, scenePipelineLayout, nullptr);
        }

        // 5. Render Pass
        if (sceneRenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(vulkanCore->vkDevice, sceneRenderPass, nullptr);
        }

        for (auto depthAttachment : scenedepthAttachment)
        {
            if (depthAttachment.view != VK_NULL_HANDLE) {
                depthAttachment.Destory(vulkanCore->vkDevice);
            }
        }

        // 7. Descriptor Pool
        if (sceneDescriptorResult.pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(vulkanCore->vkDevice, sceneDescriptorResult.pool, nullptr);
        }
    }
    bool VulkanScene::Render(Registry& registry)
    {
        vkWaitForFences(vulkanCore->vkDevice, 1, &sceneFences[frameIndex], VK_TRUE, UINT64_MAX);
        vkResetFences(vulkanCore->vkDevice, 1, &sceneFences[frameIndex]);

        VkCommandBuffer cmd = sceneCommandBuffers[frameIndex];
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) return false;

        // 3. Begin the Scene Render Pass (Renders to the scene offscreen image)
        VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassInfo.renderPass = sceneRenderPass;
        renderPassInfo.framebuffer = sceneOffscreenFrameBuffers[frameIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { width, height };

        // Set clear colors (Background of the scene)
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { { 0.01f, 0.01f, 0.01f, 1.0f } }; // Dark gray
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 4. Set Dynamic State (Standard for any custom size scene)
        VkViewport viewport{ 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        VkRect2D scissor{ {0, 0}, {width, height} };
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        for (auto& system : m_systems) {
            system->Record(cmd, frameIndex, registry, m_resources, SceneDescriptorResult);
        }

        vkCmdEndRenderPass(cmd);

        // 6. Finish and Submit
        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) return false;

        // We don't call vkQueueSubmit here—the Window will do that! 
        // The window needs to composite multiple scene results together.
        return true;
    }

    void VulkanScene::ConstructPipeline(
        VulkanCore* VC,
        PipelineInfo& info,
        const ResourceMap& resourceMap)
    {
        // 1. Reflect Shaders to get Metadata
        auto vertCode = ReadSPIRV(info.vertShaderPath);
        auto fragCode = ReadSPIRV(info.fragShaderPath);

        ShaderMetadata meta = ReflectCombinedShaders(vertCode, fragCode);

        // 2. Create Descriptors from the ResourceMap
        sceneDescriptorResult = CreateDescriptorsFromResources(VC, meta, resourceMap, static_cast<uint32_t>(parentWindow->GetMaxFramesInFlight()));

        // 3. Prepare Pipeline Layout Info
        PipelineLayoutInfo layoutInfo{};

        if (sceneDescriptorResult.layout != VK_NULL_HANDLE) {
            layoutInfo.setLayouts.push_back(sceneDescriptorResult.layout);
        }

        for (const auto& pc : meta.pushConstants) {
            VkPushConstantRange range{};
            range.stageFlags = pc.stageFlags;
            range.offset = pc.offset;
            range.size = pc.size;
            layoutInfo.pushConstants.push_back(range);
        }

        scenePipelineLayout = CreatePipelineLayout(VC, layoutInfo);
        info.pipelineLayout = scenePipelineLayout;

        scenePipeline = CreateGraphicsPipeline(VC, info);
    }
}
