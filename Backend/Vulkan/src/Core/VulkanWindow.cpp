#include "VulkanWindow.h"
#include "../VulkanRenderer.h"
#include "../Resources/VulkanTexture.h"
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <thread>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace GraphicsCore {

    VulkanWindow::VulkanWindow(VulkanRenderer* renderer, const WindowDesc& desc)
        : m_desc(desc), m_renderer(renderer), m_platformHandle(nullptr), 
          m_surface(VK_NULL_HANDLE), m_swapchain(VK_NULL_HANDLE), 
          m_swapchainFormat(VK_FORMAT_B8G8R8A8_UNORM), m_currentImageIndex(0),
          m_imageAvailableSemaphore(VK_NULL_HANDLE),
          m_renderFinishedSemaphore(VK_NULL_HANDLE), m_inFlightFence(VK_NULL_HANDLE),
          m_framebufferResized(false), m_frameReady(false)
    {
        CreatePlatformWindow();
        CreateSurface();
        CreateSwapchain();

        // Create synchronization objects
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkDevice device = renderer->GetDevice();
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore);
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore);
        vkCreateFence(device, &fenceInfo, nullptr, &m_inFlightFence);
    }

    VulkanWindow::~VulkanWindow() {
        VkDevice device = m_renderer->GetDevice();

        vkDeviceWaitIdle(device);

        if (m_imageAvailableSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, m_imageAvailableSemaphore, nullptr);
        }
        if (m_renderFinishedSemaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, m_renderFinishedSemaphore, nullptr);
        }
        if (m_inFlightFence != VK_NULL_HANDLE) {
            vkDestroyFence(device, m_inFlightFence, nullptr);
        }

        CleanupSwapchain();

        if (m_surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(m_renderer->GetInstance(), m_surface, nullptr);
        }

        if (m_platformHandle) {
            GLFWwindow* window = static_cast<GLFWwindow*>(m_platformHandle);
            glfwDestroyWindow(window);
        }
    }

    void* VulkanWindow::GetNativeHandle() const {
#ifdef _WIN32
        if (m_platformHandle) {
            return (void*)glfwGetWin32Window(static_cast<GLFWwindow*>(m_platformHandle));
        }
#endif
        return nullptr;
    }

    ITexture* VulkanWindow::GetCurrentBackbuffer() {
        if (m_currentImageIndex < m_swapchainTextures.size()) {
            return m_swapchainTextures[m_currentImageIndex];
        }
        return nullptr;
    }

    void VulkanWindow::BeginFrame() {
        m_frameReady = false;

        VkDevice device = m_renderer->GetDevice();

        // Wait for the previous frame to finish (3 second timeout to detect hangs)
        VkResult fenceResult = vkWaitForFences(device, 1, &m_inFlightFence, VK_TRUE, 3000000000ULL);

        if (fenceResult == VK_TIMEOUT)
        {
            OutputDebugStringA("[BeginFrame] vkWaitForFences TIMED OUT — fence was never signalled. "
                               "SubmitBlit likely never ran or the GPU is hung.\n");
            fprintf(stderr,    "[BeginFrame] vkWaitForFences TIMED OUT — fence was never signalled. "
                               "SubmitBlit likely never ran or the GPU is hung.\n");
            fflush(stderr);
            return;
        }

        if (fenceResult != VK_SUCCESS)
            return;

        // Reset the fence only after we know it was signalled successfully.
        vkResetFences(device, 1, &m_inFlightFence);

        // Handle pending resize before acquiring
        if (m_framebufferResized) {
            RecreateSwapchain();
            m_framebufferResized = false;
        }

        // Acquire the next swapchain image (3 second timeout to detect hangs)
        VkResult result = vkAcquireNextImageKHR(device, m_swapchain,
                                               3000000000ULL, m_imageAvailableSemaphore, VK_NULL_HANDLE,
                                               &m_currentImageIndex);

        if (result == VK_TIMEOUT)
        {
            OutputDebugStringA("[BeginFrame] vkAcquireNextImageKHR TIMED OUT — "
                               "swapchain image was never available.\n");
            fprintf(stderr,    "[BeginFrame] vkAcquireNextImageKHR TIMED OUT — "
                               "swapchain image was never available.\n");
            fflush(stderr);
            // Re-signal the fence so next BeginFrame doesn't deadlock.
            vkQueueSubmit(m_renderer->GetGraphicsQueue(), 0, nullptr, m_inFlightFence);
            return;
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            // Re-signal the fence so the next BeginFrame doesn't deadlock on a reset fence.
            vkQueueSubmit(m_renderer->GetGraphicsQueue(), 0, nullptr, m_inFlightFence);
            RecreateSwapchain();
            return;
        }

        if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
            m_frameReady = true;
        }
    }

    uint32_t VulkanWindow::AcquireNextImage() {
        return m_currentImageIndex;
    }

    void VulkanWindow::SetTitle(const char* title) {
        if (m_platformHandle)
            glfwSetWindowTitle(static_cast<GLFWwindow*>(m_platformHandle), title);
    }

    void VulkanWindow::GetPosition(int& x, int& y) const {
        if (m_platformHandle)
            glfwGetWindowPos(static_cast<GLFWwindow*>(m_platformHandle), &x, &y);
        else
            x = y = 0;
    }

    void VulkanWindow::SetPosition(int x, int y) {
        if (m_platformHandle)
            glfwSetWindowPos(static_cast<GLFWwindow*>(m_platformHandle), x, y);
    }

    void VulkanWindow::SetSize(uint32_t width, uint32_t height) {
        if (!m_platformHandle) return;
        glfwSetWindowSize(static_cast<GLFWwindow*>(m_platformHandle),
                          static_cast<int>(width), static_cast<int>(height));
        m_desc.width  = width;
        m_desc.height = height;
        m_framebufferResized = true;
    }

    void VulkanWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* w = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        if (w->m_eventSink) w->m_eventSink->OnKey(key, scancode, action, mods);
    }

    void VulkanWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        auto* w = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        if (w->m_eventSink) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            w->m_eventSink->OnMouseButton(button, action, x, y, mods);
        }
    }

    void VulkanWindow::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
        auto* w = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        if (w->m_eventSink) w->m_eventSink->OnMouseMove(xpos, ypos);
    }

    void VulkanWindow::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        auto* w = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        if (w->m_eventSink) w->m_eventSink->OnMouseScroll(xoffset, yoffset);
    }

    void VulkanWindow::WindowCloseCallback(GLFWwindow* window) {
        auto* w = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (w->m_eventSink) w->m_eventSink->OnClose();
    }

    void VulkanWindow::WindowFocusCallback(GLFWwindow* window, int focused) {
        auto* w = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        if (w->m_eventSink) w->m_eventSink->OnFocus(focused == GLFW_TRUE);
    }

    void VulkanWindow::HandleResize() {
        if (m_framebufferResized) {
            RecreateSwapchain();
            m_framebufferResized = false;
        }
    }

    void VulkanWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto vkWindow = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
        vkWindow->m_framebufferResized = true;
        vkWindow->m_desc.width = static_cast<uint32_t>(width);
        vkWindow->m_desc.height = static_cast<uint32_t>(height);
    }

    void VulkanWindow::CreatePlatformWindow() {
        // Initialize GLFW if not already initialized
        static bool glfwInitialized = false;
        if (!glfwInitialized) {
            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW");
            }
            glfwInitialized = true;
        }

        // Tell GLFW not to create an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Handle fullscreen
        GLFWmonitor* monitor = m_desc.fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        // Create the window
        GLFWwindow* window = glfwCreateWindow(
            m_desc.width,
            m_desc.height,
            m_desc.title,
            monitor,
            nullptr
        );

        if (!window) {
            throw std::runtime_error("Failed to create GLFW window");
        }

        m_platformHandle = window;

        // Set user pointer for callbacks
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetCursorPosCallback(window, MouseMoveCallback);
        glfwSetScrollCallback(window, MouseScrollCallback);
        glfwSetWindowCloseCallback(window, WindowCloseCallback);
        glfwSetWindowFocusCallback(window, WindowFocusCallback);
    }

    void VulkanWindow::CreateSurface() {
        GLFWwindow* window = static_cast<GLFWwindow*>(m_platformHandle);

        VkResult result = glfwCreateWindowSurface(
            m_renderer->GetInstance(),
            window,
            nullptr,
            &m_surface
        );

        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface");
        }
    }

    void VulkanWindow::CreateSwapchain() {
        VkPhysicalDevice physicalDevice = m_renderer->GetPhysicalDevice();
        VkDevice device = m_renderer->GetDevice();

        // Query surface capabilities
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &capabilities);

        // Query surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, formats.data());

        // Query present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, presentModes.data());

        // Choose surface format (prefer BGRA8 with SRGB color space)
        VkSurfaceFormatKHR selectedFormat = formats[0];
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && 
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                selectedFormat = format;
                break;
            }
        }
        m_swapchainFormat = selectedFormat.format;

        // Choose present mode
        // NOTE: Currently hardcoded to VSync (FIFO). In the future, this could be extended to support:
        // - VK_PRESENT_MODE_IMMEDIATE_KHR (no VSync, tearing allowed)
        // - VK_PRESENT_MODE_MAILBOX_KHR (triple buffering, no tearing)
        // - VK_PRESENT_MODE_FIFO_RELAXED_KHR (adaptive VSync)
        VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR; // VSync enabled
        if (!m_desc.vsync) {
            // Try to find immediate mode for no VSync
            for (const auto& mode : presentModes) {
                if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                    selectedPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                    break;
                }
            }
        }

        // Choose swap extent (window resolution)
        if (capabilities.currentExtent.width != UINT32_MAX) {
            m_swapchainExtent = capabilities.currentExtent;
        } else {
            m_swapchainExtent.width = std::clamp(m_desc.width, 
                capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            m_swapchainExtent.height = std::clamp(m_desc.height,
                capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        // Use minImageCount + 1 to give the swapchain one extra image.
        // This allows the application to acquire the next image without blocking
        // while the GPU is still rendering the previous frame, improving responsiveness.
        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        // Create swapchain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = m_swapchainFormat;
        createInfo.imageColorSpace = selectedFormat.colorSpace;
        createInfo.imageExtent = m_swapchainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = selectedPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain");
        }

        // Get swapchain images
        uint32_t swapchainImageCount;
        vkGetSwapchainImagesKHR(device, m_swapchain, &swapchainImageCount, nullptr);
        m_swapchainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(device, m_swapchain, &swapchainImageCount, m_swapchainImages.data());

        // Create VulkanTexture wrappers for swapchain images
        m_swapchainTextures.reserve(swapchainImageCount);
        for (VkImage image : m_swapchainImages) {
            VulkanTexture* texture = new VulkanTexture(
                device, 
                image, 
                m_swapchainFormat, 
                m_swapchainExtent.width, 
                m_swapchainExtent.height
            );
            m_swapchainTextures.push_back(texture);
        }
    }

    void VulkanWindow::RecreateSwapchain() {
        int width = 0, height = 0;
        GLFWwindow* window = static_cast<GLFWwindow*>(m_platformHandle);
        glfwGetFramebufferSize(window, &width, &height);

        // If the window is minimized (0x0), skip recreation this frame.
        // BeginFrame will return with m_frameReady=false and the caller will retry
        // next tick, by which point the window will have been un-minimized.
        if (width == 0 || height == 0)
            return;

        vkDeviceWaitIdle(m_renderer->GetDevice());

        CleanupSwapchain();
        CreateSwapchain();
    }

    void VulkanWindow::CleanupSwapchain() {
        VkDevice device = m_renderer->GetDevice();

        for (auto* texture : m_swapchainTextures) {
            delete texture;
        }
        m_swapchainTextures.clear();
        m_swapchainImages.clear();

        if (m_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, m_swapchain, nullptr);
            m_swapchain = VK_NULL_HANDLE;
        }
    }
}

