#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanTexture.h"
#include <stdexcept>
#include <vector>

namespace GraphicsCore {

    static VkFormat GetVulkanFormat(TextureFormat format); // Forward declaration from VulkanTexture.cpp

    static VkPrimitiveTopology GetVulkanTopology(PrimitiveTopology topology) {
        switch (topology) {
        case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    static VkCullModeFlags GetVulkanCullMode(CullMode mode) {
        switch (mode) {
        case CullMode::None: return VK_CULL_MODE_NONE;
        case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
        case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
        default: return VK_CULL_MODE_BACK_BIT;
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

    static VkBlendFactor GetVulkanBlendFactor(BlendFactor factor) {
        switch (factor) {
        case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        default: return VK_BLEND_FACTOR_ZERO;
        }
    }

    static VkBlendOp GetVulkanBlendOp(BlendOp op) {
        switch (op) {
        case BlendOp::Add: return VK_BLEND_OP_ADD;
        case BlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
        case BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::Min: return VK_BLEND_OP_MIN;
        case BlendOp::Max: return VK_BLEND_OP_MAX;
        default: return VK_BLEND_OP_ADD;
        }
    }

    static VkFormat GetVulkanAttributeFormat(TextureFormat format) {
        switch (format) {
        case TextureFormat::R32F: return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::RG32F: return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RGB32F: return VK_FORMAT_R32G32B32_SFLOAT;
        case TextureFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        default: return VK_FORMAT_R32G32B32_SFLOAT;
        }
    }

    VulkanPipeline::VulkanPipeline(VkDevice device, const PipelineDesc& desc)
        : m_desc(desc), m_device(device), m_pipeline(VK_NULL_HANDLE), m_pipelineLayout(VK_NULL_HANDLE)
    {
        if (desc.computeShader != nullptr) {
            CreateComputePipeline();
        } else {
            CreateGraphicsPipeline();
        }
    }

    VulkanPipeline::~VulkanPipeline() {
        if (m_pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device, m_pipeline, nullptr);
        }
        if (m_pipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        }
    }

    void VulkanPipeline::CreateGraphicsPipeline() {
        // Create pipeline layout
        std::vector<VkDescriptorSetLayout> setLayouts;
        for (auto* layout : m_desc.layouts) {
            VulkanResourceLayout* vkLayout = static_cast<VulkanResourceLayout*>(layout);
            setLayouts.push_back(vkLayout->GetDescriptorSetLayout());
        }

        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        layoutInfo.pSetLayouts = setLayouts.data();

        VkResult result = vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_pipelineLayout);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }

        // Shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        if (m_desc.vertexShader) {
            VulkanShader* vs = static_cast<VulkanShader*>(m_desc.vertexShader);
            VkPipelineShaderStageCreateInfo vertStage = {};
            vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertStage.module = vs->GetShaderModule();
            vertStage.pName = vs->GetDesc().entryPoint ? vs->GetDesc().entryPoint : "main";
            shaderStages.push_back(vertStage);
        }

        if (m_desc.fragmentShader) {
            VulkanShader* fs = static_cast<VulkanShader*>(m_desc.fragmentShader);
            VkPipelineShaderStageCreateInfo fragStage = {};
            fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragStage.module = fs->GetShaderModule();
            fragStage.pName = fs->GetDesc().entryPoint ? fs->GetDesc().entryPoint : "main";
            shaderStages.push_back(fragStage);
        }

        // Vertex input
        std::vector<VkVertexInputBindingDescription> bindingDescs;
        for (const auto& binding : m_desc.vertexBindings) {
            VkVertexInputBindingDescription desc = {};
            desc.binding = binding.binding;
            desc.stride = binding.stride;
            desc.inputRate = binding.inputRateInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescs.push_back(desc);
        }

        std::vector<VkVertexInputAttributeDescription> attributeDescs;
        for (const auto& attr : m_desc.vertexAttributes) {
            VkVertexInputAttributeDescription desc = {};
            desc.location = attr.location;
            desc.binding = attr.binding;
            desc.format = GetVulkanAttributeFormat(attr.format);
            desc.offset = attr.offset;
            attributeDescs.push_back(desc);
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescs.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescs.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = GetVulkanTopology(m_desc.topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Viewport state (dynamic)
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = m_desc.rasterizerState.depthClipEnable ? VK_FALSE : VK_TRUE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.cullMode = GetVulkanCullMode(m_desc.rasterizerState.cullMode);
        rasterizer.frontFace = m_desc.rasterizerState.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        rasterizer.lineWidth = 1.0f;

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Depth-stencil
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = m_desc.depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = m_desc.depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = GetVulkanCompareOp(m_desc.depthStencilState.depthCompareOp);
        depthStencil.stencilTestEnable = m_desc.depthStencilState.stencilTestEnable ? VK_TRUE : VK_FALSE;

        // Color blend
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.blendEnable = m_desc.blendState.blendEnable ? VK_TRUE : VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = GetVulkanBlendFactor(m_desc.blendState.srcColorBlendFactor);
        colorBlendAttachment.dstColorBlendFactor = GetVulkanBlendFactor(m_desc.blendState.dstColorBlendFactor);
        colorBlendAttachment.colorBlendOp = GetVulkanBlendOp(m_desc.blendState.colorBlendOp);
        colorBlendAttachment.srcAlphaBlendFactor = GetVulkanBlendFactor(m_desc.blendState.srcAlphaBlendFactor);
        colorBlendAttachment.dstAlphaBlendFactor = GetVulkanBlendFactor(m_desc.blendState.dstAlphaBlendFactor);
        colorBlendAttachment.alphaBlendOp = GetVulkanBlendOp(m_desc.blendState.alphaBlendOp);
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(m_desc.colorAttachmentCount, colorBlendAttachment);

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments = colorBlendAttachments.data();

        // Dynamic state
        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        // Rendering info (Vulkan 1.3+ dynamic rendering)
        std::vector<VkFormat> colorFormats;
        for (uint32_t i = 0; i < m_desc.colorAttachmentCount; ++i) {
            colorFormats.push_back(GetVulkanAttributeFormat(m_desc.colorAttachmentFormats[i]));
        }

        VkPipelineRenderingCreateInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorFormats.size());
        renderingInfo.pColorAttachmentFormats = colorFormats.data();
        if (m_desc.depthStencilFormat != TextureFormat::RGBA8) {
            renderingInfo.depthAttachmentFormat = GetVulkanAttributeFormat(m_desc.depthStencilFormat);
        }

        // Create graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &renderingInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;

        result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline");
        }
    }

    void VulkanPipeline::CreateComputePipeline() {
        // Create pipeline layout
        std::vector<VkDescriptorSetLayout> setLayouts;
        for (auto* layout : m_desc.layouts) {
            VulkanResourceLayout* vkLayout = static_cast<VulkanResourceLayout*>(layout);
            setLayouts.push_back(vkLayout->GetDescriptorSetLayout());
        }

        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        layoutInfo.pSetLayouts = setLayouts.data();

        VkResult result = vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &m_pipelineLayout);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }

        // Compute shader stage
        VulkanShader* cs = static_cast<VulkanShader*>(m_desc.computeShader);
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.module = cs->GetShaderModule();
        shaderStage.pName = cs->GetDesc().entryPoint ? cs->GetDesc().entryPoint : "main";

        // Create compute pipeline
        VkComputePipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = shaderStage;
        pipelineInfo.layout = m_pipelineLayout;

        result = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline");
        }
    }
}
