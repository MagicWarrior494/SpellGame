#pragma once
#include <memory>
#include <stdexcept>
#include <glm.hpp>


#include "Context/ContextVulkanData.h"
#include "Shader/File.h"

namespace Vulkan 
{
	
    inline VkPipelineLayout CreatePipelineLayout(std::shared_ptr<VulkanCore> VC, const PipelineLayoutInfo& info)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(info.setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = info.setLayouts.data();

        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(info.pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = info.pushConstants.data();

        VkPipelineLayout layout;
        if (vkCreatePipelineLayout(VC->vkDevice, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout");
        }

        return layout;
    }

    inline VkPipeline CreateGraphicsPipeline(std::shared_ptr<VulkanCore> VC, const PipelineInfo& info)
    {
        // 1. Load shaders
        auto vertShaderCode = ReadSPIRV(info.vertShaderPath);
        auto fragShaderCode = ReadSPIRV(info.fragShaderPath);
        VkShaderModule vertShaderModule = CreateShaderModule(VC, vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(VC, fragShaderCode);

        VkPipelineShaderStageCreateInfo vertStage{};
        vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module = vertShaderModule;
        vertStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragStage{};
        fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module = fragShaderModule;
        fragStage.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


        if (info.attributeDescriptions.empty())
        {
            // No vertex input
            vertexInput.vertexBindingDescriptionCount = 0;
            vertexInput.pVertexBindingDescriptions = nullptr;
            vertexInput.vertexAttributeDescriptionCount = 0;
            vertexInput.pVertexAttributeDescriptions = nullptr;
        }
        else
        {
            vertexInput.vertexBindingDescriptionCount = 1;
            vertexInput.pVertexBindingDescriptions = &info.bindingDescription;
            vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(info.attributeDescriptions.size());
            vertexInput.pVertexAttributeDescriptions = info.attributeDescriptions.data();
        }
       

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = info.topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = info.polygonMode;
        rasterizer.cullMode = info.cullMode;
        rasterizer.frontFace = info.frontFace;
        rasterizer.lineWidth = 1.0f;

        // 6. Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = info.samples;

        // 7. Color blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = info.enableBlending ? VK_TRUE : VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // 8. Dynamic state
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(info.dynamicStates.size());
        dynamicState.pDynamicStates = info.dynamicStates.data();

        // 9. Depth stencil (optional)
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        if (info.enableDepthTest) {
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.stencilTestEnable = VK_FALSE;
        }

        // 10. Build pipeline create info
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = info.pipelineLayout;
        pipelineInfo.renderPass = info.renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.pDepthStencilState = info.enableDepthTest ? &depthStencil : nullptr;

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(VC->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline");
        }

        vkDestroyShaderModule(VC->vkDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(VC->vkDevice, vertShaderModule, nullptr);

        return pipeline;
    }
}