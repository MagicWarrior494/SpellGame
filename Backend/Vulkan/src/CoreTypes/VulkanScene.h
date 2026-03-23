#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#include "CoreTypes/VulkanCore.h"
#include "Image/VulkanImage.h"

namespace Vulkan
{
    class VulkanWindow;

    class VulkanScene
    {
    public:
        VulkanScene(std::shared_ptr<VulkanCore> vulkanCore, int width, int height, int xpos, int ypos, VulkanWindow* parentWindow);
        ~VulkanScene();

        VkCommandBuffer BeginFrame(uint32_t frameIndex);
        void EndFrame(VkCommandBuffer cmd);

        VkRenderPass GetRenderPass() const { return sceneRenderPass; }
        VkExtent2D GetExtent() const { return VkExtent2D{ width, height }; }

    private:
        std::shared_ptr<VulkanCore> vulkanCore = nullptr;
        VulkanWindow* parentWindow = nullptr;

        uint32_t width = 0;
        uint32_t height = 0;

        int renderImageIndex = 0;

        VkRenderPass sceneRenderPass = VK_NULL_HANDLE;
        std::vector<VulkanImage> scenedepthAttachment{};
        std::vector<VkFramebuffer> sceneFrameBuffers{};

        std::vector<VkSemaphore> sceneImageAvailableSemaphores{};
        std::vector<VkSemaphore> sceneRenderFinishedSemaphores{};
        std::vector<VkFence> sceneFences{};

        VkCommandPool sceneCommandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> sceneCommandBuffers{};
    };
}