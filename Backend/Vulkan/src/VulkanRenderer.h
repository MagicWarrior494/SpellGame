#pragma once

// On Windows, must include Windows.h before Vulkan headers
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "IRenderer.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <vector>

// Windows.h defines CreateWindow as a macro, which conflicts with our method names
// Undefine it after including Vulkan headers (which may include Windows.h)
#ifdef CreateWindow
#undef CreateWindow
#endif

namespace GraphicsCore {
    class VulkanRenderer : public IRenderer {
    public:
        VulkanRenderer();
        ~VulkanRenderer();

        // Resource Creation
        IBuffer* CreateBuffer(const BufferDesc& desc) override;
        void DestroyBuffer(IBuffer* buffer) override;

        ITexture* CreateTexture(const TextureDesc& desc) override;
        void DestroyTexture(ITexture* texture) override;

        IShader* CreateShader(const ShaderDesc& desc) override;
        void DestroyShader(IShader* shader) override;

        IPipeline* CreatePipeline(const PipelineDesc& desc) override;
        void DestroyPipeline(IPipeline* pipeline) override;

        ICommandList* CreateCommandList() override;
        void DestroyCommandList(ICommandList* commandList) override;

        ISampler* CreateSampler(const SamplerDesc& desc) override;
        void DestroySampler(ISampler* sampler) override;

        IWindow* CreateWindow(const WindowDesc& desc) override;
        void DestroyWindow(IWindow* window) override;

        IResourceLayout* CreateResourceLayout(const ResourceLayoutDesc& desc) override;
        void DestroyResourceLayout(IResourceLayout* layout) override;

        IResourceSet* CreateResourceSet(IResourceLayout* layout) override;
        void DestroyResourceSet(IResourceSet* set) override;

        // Data Upload
        void* MapBuffer(IBuffer* buffer) override;
        void UnmapBuffer(IBuffer* buffer) override;

        // Execution
        void Submit(ICommandList* commandList) override;
        void Present(IWindow* window) override;
        void WaitIdle() override;

        // Internal Vulkan objects
        VkInstance GetInstance() const { return m_instance; }
        VkDevice GetDevice() const { return m_device; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
        VmaAllocator GetAllocator() const { return m_allocator; }
        VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
        VkCommandPool GetCommandPool() const { return m_commandPool; }
        uint32_t GetGraphicsQueueFamily() const { return m_graphicsQueueFamily; }

    private:
        void CreateInstance();
        void SelectPhysicalDevice();
        void CreateLogicalDevice();
        void CreateAllocator();
        void CreateCommandPool();

        VkInstance m_instance;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;
        VmaAllocator m_allocator;
        VkQueue m_graphicsQueue;
        VkCommandPool m_commandPool;
        uint32_t m_graphicsQueueFamily;

#ifdef _DEBUG
        VkDebugUtilsMessengerEXT m_debugMessenger;
#endif
    };
}