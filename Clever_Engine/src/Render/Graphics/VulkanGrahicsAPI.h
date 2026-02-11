#pragma once
#include "GraphicsAPI.h"
#include <vulkan/vulkan.h>
#include "Buffer/VulkanBuffer.h"
#include "Image/VulkanImage.h"

#include "CoreTypes/VulkanCore.h"
#include "CoreTypes/VulkanWindow.h"
#include "CoreTypes/VulkanScene.h"
#include "CoreTypes/Resource.h"

class VulkanGraphicsAPI : public GraphicsAPI
{
private:
	// This counter ensures every resource gets a unique "name"
	uint32_t m_nextHandle = 1;

	// The maps that link the ID to the real Vulkan object
	std::unordered_map<uint32_t, Vulkan::VulkanBuffer> m_buffers;
	std::unordered_map<uint32_t, Vulkan::VulkanImage> m_textures;
	std::unordered_map<uint32_t, VkSampler> m_samplers;
	std::unordered_map<uint32_t, VkShaderModule> m_shaders;

	std::unordered_map<uint32_t, Vulkan::Resource> m_resources;//The handle is the same handle as the shader that needs this resource

	std::unordered_map<uint32_t, std::unique_ptr<Vulkan::VulkanWindow>> m_windows;
	std::unordered_map<uint32_t, Vulkan::VulkanScene> m_scenes;

public:
	// Data related functions
	VulkanGraphicsAPI();
	~VulkanGraphicsAPI() override;

	BufferHandle CreateBuffer(size_t size, BufferUsage usage) override { return 0; };
	void UpdateBuffer(BufferHandle bufferHandle, const void* data, size_t sizeInBytes) override {};
	void DeleteBuffer(BufferHandle bufferHandle) override {};


	ShaderHandle CreateShader(ShaderStage stage, const std::vector<char>& code) override { return 0; };
	void DeleteShader(ShaderHandle shaderHandle) override {};

	void AssignBufferToShader(ShaderHandle shaderHandle, uint32_t resourceHandle) override;
	void AssignTextureToShader(ShaderHandle shaderHandle, uint32_t resourceHandle) override;
	void AssignSamplerToShader(ShaderHandle shaderHandle, uint32_t resourceHandle) override;

	TextureHandle CreateTexture(const TextureDescriptor& desc) override { return 0; };
	void UploadTextureData(TextureHandle textureHandle, const void* data, size_t sizeInBytes) override {};
	void UpdateTexture(uint32_t textureHandle, const void* data, size_t sizeInBytes) override {};
	void UpdateTextureRegion(TextureHandle textureHandle, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, const void* data, size_t sizeInBytes) override {};
	void DeleteTexture(TextureHandle textureHandle) override {};


	uint32_t CreateSampler(const SamplerDescriptor& desc) override { return 0; };
	void DeleteSampler(uint32_t samplerHandle) override {};

	//Window related functions
	uint32_t CreateWindow(GLFWwindow* glfwWindow) override;
	void ResizeWindow(uint32_t windowId, int width, int height) override {};
	void CloseWindow(uint32_t windowId) override;
	void RenderWindow(uint32_t windowId) override;

	//Scene related functions
	uint32_t CreateScene(uint32_t WindowId) override;
	void MoveScene(uint32_t sceneId, int x, int y) override;
	void ResizeScene(uint32_t sceneId, uint32_t width, uint32_t height) override;
	void DeleteScene(uint32_t sceneId) override;
	void SetSceneZIndex(uint32_t sceneId, int zIndex) override;
	void MoveSceneToWindow(uint32_t sceneId, uint32_t newWindowId) override;
private:
	std::shared_ptr<Vulkan::VulkanCore> m_vulkanCore;
};