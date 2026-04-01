#pragma once
#include "Buffer.h"
#include "Texture.h"
#include "Pipeline.h"
#include <cstdint>

namespace GraphicsCore
{
    struct Viewport
    {
        float x;
        float y;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };

    struct Scissor
    {
        int32_t x;
        int32_t y;
        uint32_t width;
        uint32_t height;
    };

    // Add these to CommandList.h (inside namespace GraphicsCore)

    enum class AttachmentLoadOp {
        Load,      // Keep existing contents of the texture
        Clear,     // Clear the texture to a specific value
        DontCare   // We don't care (fastest, used if we overwrite every pixel)
    };

    enum class AttachmentStoreOp {
        Store,     // Save results to memory (use this to see the result!)
        DontCare   // Discard results (use for transient depth buffers)
    };

    struct ColorAttachment {
        ITexture* texture = nullptr;
        AttachmentLoadOp loadOp = AttachmentLoadOp::Clear;
        AttachmentStoreOp storeOp = AttachmentStoreOp::Store;
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    };

    struct DepthStencilAttachment {
        ITexture* texture = nullptr;
        AttachmentLoadOp loadOp = AttachmentLoadOp::Clear;
        AttachmentStoreOp storeOp = AttachmentStoreOp::Store;
        float clearDepth = 1.0f;
        uint32_t clearStencil = 0;
    };

    class ICommandList
    {
    public:
        virtual ~ICommandList() = default;

        // --- Lifecycle ---
        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual bool IsRecording() const = 0;

        // --- Rendering ---
        // Maps to vkCmdBeginRendering (Vulkan 1.3+) / ID3D12GraphicsCommandList::OMSetRenderTargets
        virtual void BeginRendering(uint32_t colorAttachmentCount, const ColorAttachment* colorAttachments, const DepthStencilAttachment* depthAttachment = nullptr) = 0;
        virtual void EndRendering() = 0;

        // --- State & Pipeline ---
        virtual void BindPipeline(IPipeline* pipeline) = 0;
        virtual void SetViewport(const Viewport& viewport) = 0;
        virtual void SetScissor(const Scissor& scissor) = 0;

        // --- Geometry Binding ---
        virtual void BindVertexBuffer(uint32_t binding, IBuffer* buffer, size_t offset) = 0;
        virtual void BindIndexBuffer(IBuffer* buffer, size_t offset, bool use16BitIndices) = 0;

        // Note: Shader Resource Binding (Descriptors) will go here next!

        // --- Draw Commands ---
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;

        // --- Compute Commands ---
        virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

        // --- Transfers & Data Movement ---
        virtual void CopyBuffer(IBuffer* srcBuffer, IBuffer* dstBuffer, size_t size, size_t srcOffset, size_t dstOffset) = 0;
        virtual void CopyBufferToTexture(IBuffer* srcBuffer, ITexture* dstTexture, uint32_t width, uint32_t height) = 0;
        virtual void TextureBarrier(ITexture* texture, TextureUsageFlags oldUsage, TextureUsageFlags newUsage) = 0;
        virtual void BlitTexture(ITexture* src, ITexture* dst) = 0;
        virtual void BlitTexture(ITexture* src, ITexture* dst, int32_t dstX, int32_t dstY) = 0;

        virtual void BindResourceSet(uint32_t setIndex, IResourceSet* resourceSet) = 0;
        virtual void PushConstants(IShader* shader, uint32_t offset, uint32_t size, const void* data) = 0;

        virtual void* GetNativeHandle() const = 0;
    };
}