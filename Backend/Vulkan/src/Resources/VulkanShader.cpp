#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanSampler.h"
#include <stdexcept>
#include <cstring>

namespace GraphicsCore {

    VulkanShader::VulkanShader(VkDevice device, const ShaderDesc& desc)
        : m_desc(desc), m_device(device), m_shaderModule(VK_NULL_HANDLE)
    {
        // Store a copy of the bytecode
        m_bytecodeStorage.resize(desc.bytecodeSize);
        std::memcpy(m_bytecodeStorage.data(), desc.bytecode, desc.bytecodeSize);
        m_desc.bytecode = m_bytecodeStorage.data();

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = desc.bytecodeSize;
        createInfo.pCode = reinterpret_cast<const uint32_t*>(desc.bytecode);

        VkResult result = vkCreateShaderModule(m_device, &createInfo, nullptr, &m_shaderModule);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module");
        }
    }

    VulkanShader::~VulkanShader() {
        if (m_shaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
        }
    }

    // Resource Layout Implementation

    static VkDescriptorType GetVulkanDescriptorType(ResourceType type) {
        switch (type) {
        case ResourceType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case ResourceType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case ResourceType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case ResourceType::Texture: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }

    static VkShaderStageFlags GetVulkanShaderStage(ShaderStage stage) {
        switch (stage) {
        case ShaderStage::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
        case ShaderStage::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::TessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        default: return VK_SHADER_STAGE_ALL;
        }
    }

    VulkanResourceLayout::VulkanResourceLayout(VkDevice device, const ResourceLayoutDesc& desc)
        : m_desc(desc), m_device(device), m_descriptorSetLayout(VK_NULL_HANDLE)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(desc.bindings.size());

        for (const auto& binding : desc.bindings) {
            VkDescriptorSetLayoutBinding vkBinding = {};
            vkBinding.binding = binding.binding;
            vkBinding.descriptorType = GetVulkanDescriptorType(binding.type);
            vkBinding.descriptorCount = 1;
            vkBinding.stageFlags = GetVulkanShaderStage(binding.stage);
            bindings.push_back(vkBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult result = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout");
        }
    }

    VulkanResourceLayout::~VulkanResourceLayout() {
        if (m_descriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
        }
    }

    // Resource Set Implementation

    VulkanResourceSet::VulkanResourceSet(VkDevice device, VkDescriptorPool pool, VulkanResourceLayout* layout)
        : m_device(device), m_descriptorPool(pool), m_descriptorSet(VK_NULL_HANDLE), m_layout(layout)
    {
        VkDescriptorSetLayout setLayout = layout->GetDescriptorSetLayout();

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &setLayout;

        VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, &m_descriptorSet);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor set");
        }
    }

    VulkanResourceSet::~VulkanResourceSet() {
        if (m_descriptorSet != VK_NULL_HANDLE) {
            vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &m_descriptorSet);
        }
    }

    void VulkanResourceSet::UpdateBuffer(uint32_t binding, IBuffer* buffer, size_t offset, size_t range) {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = vkBuffer->GetBuffer();
        bufferInfo.offset = offset;
        bufferInfo.range  = (range > 0) ? range : VK_WHOLE_SIZE;

        // Determine the correct descriptor type from the layout
        VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        for (const auto& rb : m_layout->GetDesc().bindings)
        {
            if (rb.binding == binding)
            {
                if (rb.type == ResourceType::StorageBuffer)
                    descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            }
        }

        VkWriteDescriptorSet write = {};
        write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet          = m_descriptorSet;
        write.dstBinding      = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType  = descriptorType;
        write.pBufferInfo     = &bufferInfo;

        vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
    }

    void VulkanResourceSet::UpdateTexture(uint32_t binding, ITexture* texture, ISampler* sampler) {
        VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);
        VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vkTexture->GetImageView();
        imageInfo.sampler = vkSampler->GetSampler();

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_descriptorSet;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
    }
}
