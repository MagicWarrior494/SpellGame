#include "VulkanWindow.h"
#include "../VulkanRenderer.h"
#include "../Resources/VulkanTexture.h"
#include <stdexcept>
#include <algorithm>

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
          m_imageAvailableSemaphore(VK_NULL_HANDLE), m_renderFinishedSemaphore(VK_NULL_HANDLE),
          m_inFlightFence(VK_NULL_HANDLE), m_framebufferResized(false)
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

    uint32_t VulkanWindow::AcquireNextImage() {
        vkWaitForFences(m_renderer->GetDevice(), 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(m_renderer->GetDevice(), m_swapchain, UINT64_MAX, 
                             m_imageAvailableSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return m_currentImageIndex;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swapchain image");
        }

        vkResetFences(m_renderer->GetDevice(), 1, &m_inFlightFence);
        return m_currentImageIndex;
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

        // Determine number of images (prefer triple buffering)
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
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

        // Wait while window is minimized
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

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
