#pragma once
#include "GraphicsAPI.h"
#include "CoreTypes/VulkanCore.h"
#include "Buffer/VulkanBuffer.h"
#include "Image/VulkanImage.h"

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

public:
	VulkanGraphicsAPI();
	~VulkanGraphicsAPI() override;
	BufferHandle CreateBuffer(size_t size, BufferUsage usage) override;
	void UpdateBuffer(BufferHandle bufferHandle, const void* data, size_t sizeInBytes) override;
	void DeleteBuffer(BufferHandle bufferHandle) override;
	ShaderHandle CreateShader(ShaderStage stage, const std::vector<char>& code) override;
	void DeleteShader(ShaderHandle shaderHandle) override;
	TextureHandle CreateTexture(const TextureDescriptor& desc) override;
	void UploadTextureData(TextureHandle textureHandle, const void* data, size_t sizeInBytes) override;
	void UpdateTexture(uint32_t textureHandle, const void* data, size_t sizeInBytes) override;
	void UpdateTextureRegion(TextureHandle textureHandle, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, const void* data, size_t sizeInBytes) override;
	void DeleteTexture(TextureHandle textureHandle) override;
	uint32_t CreateSampler(const SamplerDescriptor& desc) override;
	void DeleteSampler(uint32_t samplerHandle) override;

private:
	std::unique_ptr<Vulkan::VulkanCore> m_vulkanCore;
};