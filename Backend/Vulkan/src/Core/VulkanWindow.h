#pragma once
#include "Window.h"
#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;

namespace GraphicsCore {
    class VulkanRenderer;
    class VulkanTexture;

    class VulkanWindow : public IWindow {
    public:
        VulkanWindow(VulkanRenderer* renderer, const WindowDesc& desc);
        ~VulkanWindow();

        const WindowDesc& GetDesc() const override { return m_desc; }
        void* GetNativeHandle() const override;
        void* GetPlatformHandle() const override { return m_platformHandle; }
        ITexture* GetCurrentBackbuffer() override;

        VkSwapchainKHR GetSwapchain() const { return m_swapchain; }
        uint32_t AcquireNextImage();
        VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
        VkSemaphore GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }
        VkFence GetInFlightFence() const { return m_inFlightFence; }

        // Window resize handling
        void HandleResize();
        bool WasResized() const { return m_framebufferResized; }
        void ResetResizeFlag() { m_framebufferResized = false; }

    private:
        void CreatePlatformWindow();
        void CreateSurface();
        void CreateSwapchain();
        void RecreateSwapchain();
        void CleanupSwapchain();

        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

        WindowDesc m_desc;
        VulkanRenderer* m_renderer;
        void* m_platformHandle;  // GLFWwindow*
        VkSurfaceKHR m_surface;
        VkSwapchainKHR m_swapchain;
        VkFormat m_swapchainFormat;
        VkExtent2D m_swapchainExtent;
        std::vector<VkImage> m_swapchainImages;
        std::vector<VulkanTexture*> m_swapchainTextures;
        uint32_t m_currentImageIndex;

        VkSemaphore m_imageAvailableSemaphore;
        VkSemaphore m_renderFinishedSemaphore;
        VkFence m_inFlightFence;

        bool m_framebufferResized;
    };
}
