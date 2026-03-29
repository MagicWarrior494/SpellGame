#pragma once
#include "Sampler.h"
#include <vulkan/vulkan.h>

namespace GraphicsCore {
    class VulkanSampler : public ISampler {
    public:
        VulkanSampler(VkDevice device, const SamplerDesc& desc);
        ~VulkanSampler();

        const SamplerDesc& GetDesc() const override { return m_desc; }
        void* GetNativeHandle() const override { return (void*)m_sampler; }

        VkSampler GetSampler() const { return m_sampler; }

    private:
        SamplerDesc m_desc;
        VkDevice m_device;
        VkSampler m_sampler;
    };
}
