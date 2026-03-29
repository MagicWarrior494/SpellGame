#pragma once
#include "Shader.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace GraphicsCore {
    class VulkanShader : public IShader {
    public:
        VulkanShader(VkDevice device, const ShaderDesc& desc);
        ~VulkanShader();

        const ShaderDesc& GetDesc() const override { return m_desc; }
        void* GetNativeHandle() const override { return (void*)m_shaderModule; }

        VkShaderModule GetShaderModule() const { return m_shaderModule; }

    private:
        ShaderDesc m_desc;
        VkDevice m_device;
        VkShaderModule m_shaderModule;
        std::vector<uint8_t> m_bytecodeStorage;
    };

    class VulkanResourceLayout : public IResourceLayout {
    public:
        VulkanResourceLayout(VkDevice device, const ResourceLayoutDesc& desc);
        ~VulkanResourceLayout();

        void* GetNativeHandle() const override { return (void*)m_descriptorSetLayout; }
        VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }
        const ResourceLayoutDesc& GetDesc() const { return m_desc; }

    private:
        ResourceLayoutDesc m_desc;
        VkDevice m_device;
        VkDescriptorSetLayout m_descriptorSetLayout;
    };

    class VulkanResourceSet : public IResourceSet {
    public:
        VulkanResourceSet(VkDevice device, VkDescriptorPool pool, VulkanResourceLayout* layout);
        ~VulkanResourceSet();

        void UpdateBuffer(uint32_t binding, IBuffer* buffer, size_t offset, size_t range) override;
        void UpdateTexture(uint32_t binding, ITexture* texture, ISampler* sampler) override;

        VkDescriptorSet GetDescriptorSet() const { return m_descriptorSet; }

    private:
        VkDevice m_device;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSet m_descriptorSet;
        VulkanResourceLayout* m_layout;
    };
}
