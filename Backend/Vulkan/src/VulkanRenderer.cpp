#define VMA_IMPLEMENTATION
#include "VulkanRenderer.h"
#include "Resources/VulkanBuffer.h"
#include "Resources/VulkanTexture.h"
#include "Resources/VulkanShader.h"
#include "Resources/VulkanSampler.h"
#include "Resources/VulkanPipeline.h"
#include "Core/VulkanCommandList.h"
#include "Core/VulkanWindow.h"
#include <stdexcept>
#include <vector>
#include <cstring>

namespace GraphicsCore {

#ifdef _DEBUG
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            // Log or handle the validation message
        }
        return VK_FALSE;
    }
#endif

    VulkanRenderer::VulkanRenderer()
        : m_instance(VK_NULL_HANDLE), m_physicalDevice(VK_NULL_HANDLE), 
          m_device(VK_NULL_HANDLE), m_allocator(VK_NULL_HANDLE), 
          m_graphicsQueue(VK_NULL_HANDLE), m_commandPool(VK_NULL_HANDLE),
          m_graphicsQueueFamily(0)
#ifdef _DEBUG
        , m_debugMessenger(VK_NULL_HANDLE)
#endif
    {
        CreateInstance();
        SelectPhysicalDevice();
        CreateLogicalDevice();
        CreateAllocator();
        CreateCommandPool();
    }

    VulkanRenderer::~VulkanRenderer() {
        if (m_device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(m_device);
        }

        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        }

        if (m_allocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(m_allocator);
        }

        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
        }

#ifdef _DEBUG
        if (m_debugMessenger != VK_NULL_HANDLE) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                m_instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(m_instance, m_debugMessenger, nullptr);
            }
        }
#endif

        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, nullptr);
        }
    }

    void VulkanRenderer::CreateInstance() {
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "GraphicsCore Application";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "GraphicsCore";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
        };

#ifdef _DEBUG
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        std::vector<const char*> layers;
#ifdef _DEBUG
        layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance");
        }

#ifdef _DEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            m_instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
        }
#endif
    }

    void VulkanRenderer::SelectPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        // Just pick the first discrete GPU, or the first device
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                m_physicalDevice = device;
                return;
            }
        }

        m_physicalDevice = devices[0];
    }

    void VulkanRenderer::CreateLogicalDevice() {
        // Find graphics queue family
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_graphicsQueueFamily = i;
                break;
            }
        }

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        // Vulkan 1.3 features for dynamic rendering
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkPhysicalDeviceVulkan13Features vulkan13Features = {};
        vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        vulkan13Features.dynamicRendering = VK_TRUE;
        vulkan13Features.synchronization2 = VK_TRUE;

        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &vulkan13Features;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device");
        }

        vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
    }

    void VulkanRenderer::CreateAllocator() {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorInfo.physicalDevice = m_physicalDevice;
        allocatorInfo.device = m_device;
        allocatorInfo.instance = m_instance;

        VkResult result = vmaCreateAllocator(&allocatorInfo, &m_allocator);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create VMA allocator");
        }
    }

    void VulkanRenderer::CreateCommandPool() {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_graphicsQueueFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool");
        }
    }

    // Resource Creation
    IBuffer* VulkanRenderer::CreateBuffer(const BufferDesc& desc) {
        return new VulkanBuffer(m_device, m_allocator, desc);
    }

    void VulkanRenderer::DestroyBuffer(IBuffer* buffer) {
        delete buffer;
    }

    ITexture* VulkanRenderer::CreateTexture(const TextureDesc& desc) {
        return new VulkanTexture(m_device, m_allocator, desc);
    }

    void VulkanRenderer::DestroyTexture(ITexture* texture) {
        delete texture;
    }

    IShader* VulkanRenderer::CreateShader(const ShaderDesc& desc) {
        return new VulkanShader(m_device, desc);
    }

    void VulkanRenderer::DestroyShader(IShader* shader) {
        delete shader;
    }

    IPipeline* VulkanRenderer::CreatePipeline(const PipelineDesc& desc) {
        return new VulkanPipeline(m_device, desc);
    }

    void VulkanRenderer::DestroyPipeline(IPipeline* pipeline) {
        delete pipeline;
    }

    ICommandList* VulkanRenderer::CreateCommandList() {
        return new VulkanCommandList(this);
    }

    void VulkanRenderer::DestroyCommandList(ICommandList* commandList) {
        delete commandList;
    }

    ISampler* VulkanRenderer::CreateSampler(const SamplerDesc& desc) {
        return new VulkanSampler(m_device, desc);
    }

    void VulkanRenderer::DestroySampler(ISampler* sampler) {
        delete sampler;
    }

    IWindow* VulkanRenderer::CreateWindow(const WindowDesc& desc) {
        return new VulkanWindow(this, desc);
    }

    void VulkanRenderer::DestroyWindow(IWindow* window) {
        delete window;
    }

    IResourceLayout* VulkanRenderer::CreateResourceLayout(const ResourceLayoutDesc& desc) {
        return new VulkanResourceLayout(m_device, desc);
    }

    void VulkanRenderer::DestroyResourceLayout(IResourceLayout* layout) {
        delete layout;
    }

    // Descriptor pool for resource sets (simplified - in production you'd have a pool manager)
    static VkDescriptorPool g_descriptorPool = VK_NULL_HANDLE;

    IResourceSet* VulkanRenderer::CreateResourceSet(IResourceLayout* layout) {
        // Create descriptor pool if needed
        if (g_descriptorPool == VK_NULL_HANDLE) {
            VkDescriptorPoolSize poolSizes[] = {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 }
            };

            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolInfo.maxSets = 1000;
            poolInfo.poolSizeCount = 4;
            poolInfo.pPoolSizes = poolSizes;

            vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &g_descriptorPool);
        }

        VulkanResourceLayout* vkLayout = static_cast<VulkanResourceLayout*>(layout);
        return new VulkanResourceSet(m_device, g_descriptorPool, vkLayout);
    }

    void VulkanRenderer::DestroyResourceSet(IResourceSet* set) {
        delete set;
    }

    // Data Upload
    void* VulkanRenderer::MapBuffer(IBuffer* buffer) {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
        void* data = nullptr;
        vmaMapMemory(m_allocator, vkBuffer->GetAllocation(), &data);
        return data;
    }

    void VulkanRenderer::UnmapBuffer(IBuffer* buffer) {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
        vmaUnmapMemory(m_allocator, vkBuffer->GetAllocation());
    }

    // Execution
    void VulkanRenderer::Submit(ICommandList* commandList) {
        VulkanCommandList* vkCommandList = static_cast<VulkanCommandList*>(commandList);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkCommandBuffer commandBuffer = vkCommandList->GetCommandBuffer();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    }

    void VulkanRenderer::Present(IWindow* window) {
        VulkanWindow* vkWindow = static_cast<VulkanWindow*>(window);

        uint32_t imageIndex = vkWindow->AcquireNextImage();

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        VkSemaphore signalSemaphores[] = { vkWindow->GetRenderFinishedSemaphore() };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = { vkWindow->GetSwapchain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
    }

    void VulkanRenderer::WaitIdle() {
        vkDeviceWaitIdle(m_device);
    }
}
