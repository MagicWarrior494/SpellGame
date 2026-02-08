#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

// Forward declarations to avoid heavy Vulkan headers here

namespace Vulkan
{

    struct VulkanCore;
    struct VulkanWindow;
    struct VulkanScene;

    class VulkanWrapper
    {
    public:
        VulkanWrapper();
        ~VulkanWrapper() = default;

        // ---- Core ----
        void CreateCore();
        VulkanCore* GetCore() const;

        // ---- Windows ----
        int CreateWindow(void* windowHandle);
        void DestroyWindow(int windowId);
        VulkanWindow* GetWindow(int windowId) const;

        // ---- Scenes ----
        int CreateScene(void* windowHandle);
        void DestroyScene(int sceneId);
        VulkanScene* GetScene(int sceneId) const;

    private:
        VulkanWrapper(const VulkanWrapper&) = delete;
        VulkanWrapper& operator=(const VulkanWrapper&) = delete;

    private:
        std::unique_ptr<VulkanCore> m_core;
        std::unordered_map<int, std::unique_ptr<VulkanWindow>> m_windows;
        std::unordered_map<int, std::unique_ptr<VulkanScene>> m_scenes;

        std::map<int, VulkanBuffer> m_sharedBuffers;
		std::map<int, VulkanImage> m_sharedImages;
    };

}