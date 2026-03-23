#pragma once
#include <vulkan/vulkan.h>
#include <glm.hpp>
#include <vector>
#include <stdint.h>
#include <memory>

#include "Flags.h"
#include "Image/VulkanImage.h"
#include "VulkanCore.h"

namespace Vulkan
{
    // Forward declaration
    class VulkanScene;

    struct DescriptorBindingInfo
    {
        uint32_t binding = 0;
        VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        std::vector<VkDescriptorBufferInfo> buffers;
        std::vector<VkDescriptorImageInfo> images;
        uint32_t count = 1;
    };

    struct DescriptorSetInfo {
        std::vector<DescriptorBindingInfo> bindings;
        uint32_t maxSets = 1;
    };

    struct DescriptorResult {
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> sets;
    };

    struct SurfacePushConstants
    {
        uint32_t sceneIndex = 0;
    };

    class VulkanWindow
    {
    public:
        VulkanWindow() = default;
        VulkanWindow(std::shared_ptr<VulkanCore> vulkanCore, void* windowPtr);
        ~VulkanWindow();

        int AddNewScene(int width, int height);
        void RecreateSwapchain();
        bool Render();

        std::vector<VulkanImage>& GetSceneImages(int sceneIndex) { return sceneImages.at(sceneIndex); }
        int GetMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }
        int GetMaxScenes() const { return MAX_SCENES; }

		glm::uvec2 GetWindowSize() const { return windowSize; }

    private:
        std::shared_ptr<VulkanCore> vulkanCore = nullptr;

        glm::uvec2 windowSize{ 0, 0 };
        int MAX_FRAMES_IN_FLIGHT = 2;
        int MAX_SCENES = 16;

        SurfaceFlags flags = SurfaceFlags::None;
        SwapchainAttachmentType surfaceType = SwapchainAttachmentType::ColorOnly;

        uint8_t imageFrameCounter = 0;
        void* windowPtr = nullptr;

        VkSurfaceKHR surfaceSurface = VK_NULL_HANDLE;
        VkSwapchainKHR surfaceSwapChain = VK_NULL_HANDLE;
        VkFormat surfaceSwapChainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

        std::vector<VulkanImage> surfaceColorImages{};
        std::vector<VulkanImage> surfaceDepthImages{};
        VkRenderPass surfaceRenderPass = VK_NULL_HANDLE;

        std::vector<VkFramebuffer> surfaceFrameBuffers{};

        VkCommandPool surfaceCommandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> surfacePresentCommandBuffers{};

        std::vector<VkSemaphore> surfaceImageAvailableSemaphores{};
        std::vector<VkSemaphore> surfaceRenderFinishedSemaphores{};
        std::vector<VkFence> surfaceFences{};

        DescriptorSetInfo descriptorSetInfo{};
        DescriptorResult surfaceDescriptorResult{};

        VkPipelineLayout surfacePipelineLayout = VK_NULL_HANDLE;
        VkPipeline surfacePipeline = VK_NULL_HANDLE;

        VkSampler offscreenSampler = VK_NULL_HANDLE;

        std::vector<std::vector<VulkanImage>> sceneImages{};
    };
}