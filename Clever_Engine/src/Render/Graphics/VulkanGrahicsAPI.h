#pragma once
#include "GraphicsAPI.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include "Buffer/VulkanBuffer.h"
#include "Image/VulkanImage.h"

#include "CoreTypes/VulkanCore.h"
#include "CoreTypes/VulkanWindow.h"
#include "CoreTypes/VulkanScene.h"
#include "Shader/Shader.h"
#include "World/Assets/Resource.h"

#include "World/ECS/Registry.h"
#include "World/AssetManager.h"

class VulkanGraphicsAPI : public GraphicsAPI
{
private:
    uint32_t m_nextHandle = 1;

    std::unordered_map<uint32_t, Vulkan::VulkanBuffer> m_buffers;
    std::unordered_map<uint32_t, Vulkan::VulkanImage> m_textures;
    std::unordered_map<uint32_t, VkSampler> m_samplers;
    std::unordered_map<uint32_t, VkShaderModule> m_shaders;

    std::unordered_map<uint32_t, ResourceMap> m_shaderResources;

    std::unordered_map<uint32_t, std::unique_ptr<Vulkan::VulkanWindow>> m_windows;
    std::unordered_map<uint32_t, Vulkan::VulkanScene> m_scenes;

	std::unordered_map<ShaderHandle, Vulkan::DescriptorResult> m_descriptorResults;

public:
    VulkanGraphicsAPI();
    ~VulkanGraphicsAPI() override;

    BufferHandle CreateBuffer(size_t size, BufferUsage usage) override;
    void UpdateBuffer(BufferHandle bufferHandle, const void* data, size_t sizeInBytes) override;
    void DeleteBuffer(BufferHandle bufferHandle) override {};

    ShaderHandle CreateShader(ShaderStage stage, const std::vector<uint32_t>& code) override;
    void DeleteShader(ShaderHandle shaderHandle) override {};

    TextureHandle CreateTexture(const TextureDescriptor& desc) override { return TextureHandle{ 0 }; };
    void UploadTextureData(TextureHandle textureHandle, const void* data, size_t sizeInBytes) override {};
    void UpdateTexture(TextureHandle textureHandle, const void* data, size_t sizeInBytes) override {};
    void UpdateTextureRegion(TextureHandle textureHandle, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, const void* data, size_t sizeInBytes) override {};
    void DeleteTexture(TextureHandle textureHandle) override {};

    SamplerHandle CreateSampler(const SamplerDescriptor& desc) override { return SamplerHandle{ 0 }; };
    void DeleteSampler(SamplerHandle samplerHandle) override {};

    uint32_t CreateWindow(GLFWwindow* glfwWindow) override;
    void ResizeWindow(uint32_t windowId, int width, int height) override {};
    void CloseWindow(uint32_t windowId) override;
    void RenderWindow(uint32_t windowId) override;

    uint32_t CreateScene(uint32_t windowId) override;
    void RenderScene(uint32_t sceneId, Registry& registry, AssetManager& assetManager) override;

    void MoveScene(uint32_t sceneId, int x, int y) override {};
    void ResizeScene(uint32_t sceneId, uint32_t width, uint32_t height) override {};
    void DeleteScene(uint32_t sceneId) override {};
    void SetSceneZIndex(uint32_t sceneId, int zIndex) override {};
    void MoveSceneToWindow(uint32_t sceneId, uint32_t newWindowId) override {};

    const ResourceMap* GetShaderResources(ShaderHandle shaderHandle) const;

private:
    CachedPipeline& GetOrCreatePipeline(Vulkan::VulkanScene& scene, ShaderHandle vert, ShaderHandle frag);

    std::shared_ptr<Vulkan::VulkanCore> m_vulkanCore;
};