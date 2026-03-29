#include "VulkanSampler.h"
#include <stdexcept>

namespace GraphicsCore {

    static VkFilter GetVulkanFilter(FilterMode filter) {
        switch (filter) {
        case FilterMode::Nearest: return VK_FILTER_NEAREST;
        case FilterMode::Linear: return VK_FILTER_LINEAR;
        default: return VK_FILTER_LINEAR;
        }
    }

    static VkSamplerMipmapMode GetVulkanMipmapMode(FilterMode filter) {
        switch (filter) {
        case FilterMode::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case FilterMode::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }

    static VkSamplerAddressMode GetVulkanAddressMode(WrapMode wrap) {
        switch (wrap) {
        case WrapMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case WrapMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case WrapMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case WrapMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    static VkCompareOp GetVulkanCompareOp(CompareOp op) {
        switch (op) {
        case CompareOp::Never: return VK_COMPARE_OP_NEVER;
        case CompareOp::Less: return VK_COMPARE_OP_LESS;
        case CompareOp::Equal: return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater: return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        default: return VK_COMPARE_OP_ALWAYS;
        }
    }

    VulkanSampler::VulkanSampler(VkDevice device, const SamplerDesc& desc)
        : m_desc(desc), m_device(device), m_sampler(VK_NULL_HANDLE)
    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = GetVulkanFilter(desc.magFilter);
        samplerInfo.minFilter = GetVulkanFilter(desc.minFilter);
        samplerInfo.mipmapMode = GetVulkanMipmapMode(desc.mipFilter);
        samplerInfo.addressModeU = GetVulkanAddressMode(desc.wrapU);
        samplerInfo.addressModeV = GetVulkanAddressMode(desc.wrapV);
        samplerInfo.addressModeW = GetVulkanAddressMode(desc.wrapW);
        samplerInfo.mipLodBias = desc.mipLodBias;
        samplerInfo.anisotropyEnable = desc.anisotropyEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = desc.maxAnisotropy;
        samplerInfo.compareEnable = desc.compareEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.compareOp = GetVulkanCompareOp(desc.compareOp);
        samplerInfo.minLod = desc.minLod;
        samplerInfo.maxLod = desc.maxLod;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        VkResult result = vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan sampler");
        }
    }

    VulkanSampler::~VulkanSampler() {
        if (m_sampler != VK_NULL_HANDLE) {
            vkDestroySampler(m_device, m_sampler, nullptr);
        }
    }
}
