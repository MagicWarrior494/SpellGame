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
        void BeginFrame() override;
        void SetEventSink(GraphicsCore::IWindowEventSink* sink) override { m_eventSink = sink; }

        VkSwapchainKHR GetSwapchain() const { return m_swapchain; }
        uint32_t AcquireNextImage();
        uint32_t& GetCurrentImageIndex() { return m_currentImageIndex; }
        VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
        VkSemaphore GetSceneFinishedSemaphore()  const { return m_sceneFinishedSemaphore; }
        VkSemaphore GetRenderFinishedSemaphore() const { return m_renderFinishedSemaphore; }
        VkFence GetInFlightFence() const { return m_inFlightFence; }
        VkFence& GetInFlightFenceRef() { return m_inFlightFence; }

        // Window resize handling
        void HandleResize();
        bool WasResized() const { return m_framebufferResized; }
        void ResetResizeFlag() { m_framebufferResized = false; }
        bool IsFrameReady() const override { return m_frameReady; }

    private:
        void CreatePlatformWindow();
        void CreateSurface();
        void CreateSwapchain();
        void RecreateSwapchain();
        void CleanupSwapchain();

        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
        static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

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
        VkSemaphore m_sceneFinishedSemaphore;
        VkSemaphore m_renderFinishedSemaphore;
        VkFence m_inFlightFence;

        bool m_framebufferResized;
        bool m_frameReady;
        GraphicsCore::IWindowEventSink* m_eventSink = nullptr;
    };
}
