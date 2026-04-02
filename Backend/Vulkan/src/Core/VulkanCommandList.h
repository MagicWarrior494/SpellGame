#pragma once
#include "CommandList.h"
#include <vulkan/vulkan.h>

namespace GraphicsCore {
    class VulkanRenderer;

    class VulkanCommandList : public ICommandList {
    public:
        VulkanCommandList(VulkanRenderer* renderer);
        ~VulkanCommandList();

        void Begin() override;
        void End() override;
        bool IsRecording() const override { return m_isRecording; }

        void BeginRendering(uint32_t colorAttachmentCount, const ColorAttachment* colorAttachments, 
                          const DepthStencilAttachment* depthAttachment) override;
        void EndRendering() override;

        void BindPipeline(IPipeline* pipeline) override;
        void SetViewport(const Viewport& viewport) override;
        void SetScissor(const Scissor& scissor) override;

        void BindVertexBuffer(uint32_t binding, IBuffer* buffer, size_t offset) override;
        void BindIndexBuffer(IBuffer* buffer, size_t offset, bool use16BitIndices) override;

        void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, 
                        int32_t vertexOffset, uint32_t firstInstance) override;

        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

        void CopyBuffer(IBuffer* srcBuffer, IBuffer* dstBuffer, size_t size, size_t srcOffset, size_t dstOffset) override;
        void CopyBufferToTexture(IBuffer* srcBuffer, ITexture* dstTexture, uint32_t width, uint32_t height) override;
        void TextureBarrier(ITexture* texture, TextureUsageFlags oldUsage, TextureUsageFlags newUsage) override;
        void BlitTexture(ITexture* src, ITexture* dst) override;
        void BlitTexture(ITexture* src, ITexture* dst, int32_t dstX, int32_t dstY) override;
        void ClearTexture(ITexture* texture, float r, float g, float b, float a) override;

        void BindResourceSet(uint32_t setIndex, IResourceSet* resourceSet) override;
        void PushConstants(IShader* shader, uint32_t offset, uint32_t size, const void* data) override;

        void* GetNativeHandle() const override { return (void*)m_commandBuffer; }
        VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

    private:
        VulkanRenderer* m_renderer;
        VkCommandBuffer m_commandBuffer;
        bool m_isRecording;
        VkPipelineLayout m_currentPipelineLayout;
        VkPipelineBindPoint m_currentBindPoint;
    };
}
