#include "VulkanCommandList.h"
#include "../VulkanRenderer.h"
#include "../Resources/VulkanBuffer.h"
#include "../Resources/VulkanTexture.h"
#include "../Resources/VulkanPipeline.h"
#include "../Resources/VulkanShader.h"
#include <stdexcept>

namespace GraphicsCore {

    static VkAttachmentLoadOp GetVulkanLoadOp(AttachmentLoadOp op) {
        switch (op) {
        case AttachmentLoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case AttachmentLoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }

    static VkAttachmentStoreOp GetVulkanStoreOp(AttachmentStoreOp op) {
        switch (op) {
        case AttachmentStoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
        case AttachmentStoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default: return VK_ATTACHMENT_STORE_OP_STORE;
        }
    }

    VulkanCommandList::VulkanCommandList(VulkanRenderer* renderer)
        : m_renderer(renderer), m_commandBuffer(VK_NULL_HANDLE), m_isRecording(false), 
          m_currentPipelineLayout(VK_NULL_HANDLE)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = renderer->GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkResult result = vkAllocateCommandBuffers(renderer->GetDevice(), &allocInfo, &m_commandBuffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffer");
        }
    }

    VulkanCommandList::~VulkanCommandList() {
        if (m_commandBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(m_renderer->GetDevice(), m_renderer->GetCommandPool(), 1, &m_commandBuffer);
        }
    }

    void VulkanCommandList::Begin() {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkResult result = vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin command buffer");
        }
        m_isRecording = true;
    }

    void VulkanCommandList::End() {
        VkResult result = vkEndCommandBuffer(m_commandBuffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to end command buffer");
        }
        m_isRecording = false;
    }

    void VulkanCommandList::BeginRendering(uint32_t colorAttachmentCount, const ColorAttachment* colorAttachments,
                                          const DepthStencilAttachment* depthAttachment) {
        std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos;
        colorAttachmentInfos.reserve(colorAttachmentCount);

        for (uint32_t i = 0; i < colorAttachmentCount; ++i) {
            const ColorAttachment& colorAttach = colorAttachments[i];
            VulkanTexture* texture = static_cast<VulkanTexture*>(colorAttach.texture);

            VkRenderingAttachmentInfo attachmentInfo = {};
            attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            attachmentInfo.imageView = texture->GetImageView();
            attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachmentInfo.loadOp = GetVulkanLoadOp(colorAttach.loadOp);
            attachmentInfo.storeOp = GetVulkanStoreOp(colorAttach.storeOp);
            attachmentInfo.clearValue.color = { 
                colorAttach.clearColor[0], 
                colorAttach.clearColor[1], 
                colorAttach.clearColor[2], 
                colorAttach.clearColor[3] 
            };

            colorAttachmentInfos.push_back(attachmentInfo);
        }

        VkRenderingAttachmentInfo depthAttachmentInfo = {};
        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = { 0, 0 };
        if (colorAttachmentCount > 0) {
            VulkanTexture* firstTexture = static_cast<VulkanTexture*>(colorAttachments[0].texture);
            renderingInfo.renderArea.extent = { firstTexture->GetDesc().width, firstTexture->GetDesc().height };
        }
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfos.size());
        renderingInfo.pColorAttachments = colorAttachmentInfos.data();

        if (depthAttachment && depthAttachment->texture) {
            VulkanTexture* depthTexture = static_cast<VulkanTexture*>(depthAttachment->texture);
            depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachmentInfo.imageView = depthTexture->GetImageView();
            depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentInfo.loadOp = GetVulkanLoadOp(depthAttachment->loadOp);
            depthAttachmentInfo.storeOp = GetVulkanStoreOp(depthAttachment->storeOp);
            depthAttachmentInfo.clearValue.depthStencil = { depthAttachment->clearDepth, depthAttachment->clearStencil };

            renderingInfo.pDepthAttachment = &depthAttachmentInfo;
        }

        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void VulkanCommandList::EndRendering() {
        vkCmdEndRendering(m_commandBuffer);
    }

    void VulkanCommandList::BindPipeline(IPipeline* pipeline) {
        VulkanPipeline* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
        VkPipelineBindPoint bindPoint = vkPipeline->IsCompute() ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
        vkCmdBindPipeline(m_commandBuffer, bindPoint, vkPipeline->GetPipeline());
        m_currentPipelineLayout = vkPipeline->GetPipelineLayout();
    }

    void VulkanCommandList::SetViewport(const Viewport& viewport) {
        VkViewport vp = {};
        vp.x = viewport.x;
        vp.y = viewport.y;
        vp.width = viewport.width;
        vp.height = viewport.height;
        vp.minDepth = viewport.minDepth;
        vp.maxDepth = viewport.maxDepth;
        vkCmdSetViewport(m_commandBuffer, 0, 1, &vp);
    }

    void VulkanCommandList::SetScissor(const Scissor& scissor) {
        VkRect2D sc = {};
        sc.offset = { scissor.x, scissor.y };
        sc.extent = { scissor.width, scissor.height };
        vkCmdSetScissor(m_commandBuffer, 0, 1, &sc);
    }

    void VulkanCommandList::BindVertexBuffer(uint32_t binding, IBuffer* buffer, size_t offset) {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
        VkBuffer vertexBuffers[] = { vkBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { offset };
        vkCmdBindVertexBuffers(m_commandBuffer, binding, 1, vertexBuffers, offsets);
    }

    void VulkanCommandList::BindIndexBuffer(IBuffer* buffer, size_t offset, bool use16BitIndices) {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
        VkIndexType indexType = use16BitIndices ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(m_commandBuffer, vkBuffer->GetBuffer(), offset, indexType);
    }

    void VulkanCommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
        vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void VulkanCommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                       int32_t vertexOffset, uint32_t firstInstance) {
        vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void VulkanCommandList::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        vkCmdDispatch(m_commandBuffer, groupCountX, groupCountY, groupCountZ);
    }

    void VulkanCommandList::CopyBuffer(IBuffer* srcBuffer, IBuffer* dstBuffer, size_t size, size_t srcOffset, size_t dstOffset) {
        VulkanBuffer* src = static_cast<VulkanBuffer*>(srcBuffer);
        VulkanBuffer* dst = static_cast<VulkanBuffer*>(dstBuffer);

        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        copyRegion.size = size;

        vkCmdCopyBuffer(m_commandBuffer, src->GetBuffer(), dst->GetBuffer(), 1, &copyRegion);
    }

    void VulkanCommandList::TextureBarrier(ITexture* texture, TextureUsageFlags oldUsage, TextureUsageFlags newUsage) {
        (void)oldUsage; // Currently unused, but kept for API consistency
        VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = vkTexture->GetCurrentLayout();
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vkTexture->GetImage();

        if (texture->GetDesc().usage & TextureUsage_DepthStencil) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = texture->GetDesc().mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = texture->GetDesc().arrayLayers;

        // Determine new layout based on usage
        if (newUsage & TextureUsage_RenderTarget) {
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else if (newUsage & TextureUsage_DepthStencil) {
            barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else if (newUsage & TextureUsage_ShaderResource) {
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else if (newUsage & TextureUsage_TransferDst) {
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        } else if (newUsage & TextureUsage_TransferSrc) {
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        } else {
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        vkCmdPipelineBarrier(m_commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        vkTexture->SetCurrentLayout(barrier.newLayout);
    }

    void VulkanCommandList::BindResourceSet(uint32_t setIndex, IResourceSet* resourceSet) {
        VulkanResourceSet* vkResourceSet = static_cast<VulkanResourceSet*>(resourceSet);
        VkDescriptorSet descriptorSet = vkResourceSet->GetDescriptorSet();
        vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                               m_currentPipelineLayout, setIndex, 1, &descriptorSet, 0, nullptr);
    }

    void VulkanCommandList::PushConstants(IShader* shader, uint32_t offset, uint32_t size, const void* data) {
        VulkanShader* vkShader = static_cast<VulkanShader*>(shader);
        VkShaderStageFlags stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        
        switch (vkShader->GetDesc().stage) {
        case ShaderStage::Vertex: stageFlags = VK_SHADER_STAGE_VERTEX_BIT; break;
        case ShaderStage::Fragment: stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; break;
        case ShaderStage::Compute: stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; break;
        default: break;
        }

        vkCmdPushConstants(m_commandBuffer, m_currentPipelineLayout, stageFlags, offset, size, data);
    }
}
