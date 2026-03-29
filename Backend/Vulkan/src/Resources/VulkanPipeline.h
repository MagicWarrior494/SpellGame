#pragma once
#include "Pipeline.h"
#include <vulkan/vulkan.h>

namespace GraphicsCore {
    class VulkanPipeline : public IPipeline {
    public:
        VulkanPipeline(VkDevice device, const PipelineDesc& desc);
        ~VulkanPipeline();

        const PipelineDesc& GetDesc() const override { return m_desc; }
        void* GetNativeHandle() const override { return (void*)m_pipeline; }

        VkPipeline GetPipeline() const { return m_pipeline; }
        VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
        bool IsCompute() const { return m_desc.computeShader != nullptr; }

    private:
        void CreateGraphicsPipeline();
        void CreateComputePipeline();

        PipelineDesc m_desc;
        VkDevice m_device;
        VkPipeline m_pipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}
