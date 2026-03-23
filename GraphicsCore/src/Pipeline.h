#pragma once
#include "Common.h"
#include "Shader.h"
#include <vector>

namespace GraphicsCore
{
    struct BlendState
    {
        bool blendEnable;
        BlendFactor srcColorBlendFactor;
        BlendFactor dstColorBlendFactor;
        BlendOp colorBlendOp;
        BlendFactor srcAlphaBlendFactor;
        BlendFactor dstAlphaBlendFactor;
        BlendOp alphaBlendOp;
    };

    struct RasterizerState
    {
        CullMode cullMode;
        bool frontCounterClockwise;
        bool depthClipEnable;
    };

    struct DepthStencilState
    {
        bool depthTestEnable;
        bool depthWriteEnable;
        CompareOp depthCompareOp;
        bool stencilTestEnable;
    };

    struct VertexAttribute
    {
        uint32_t location;    // Matches 'layout(location = X)' in shader
        uint32_t binding;     // Which vertex buffer slot this comes from
        TextureFormat format; // Reuse TextureFormat for data types (e.g., RGB32F for vec3)
        uint32_t offset;      // Offset in bytes from start of vertex
    };

    struct VertexBinding
    {
        uint32_t binding;     // The slot index
        uint32_t stride;      // Total bytes per vertex
        bool inputRateInstance; // False = per vertex, True = per instance
    };

    struct PipelineDesc
    {
        IShader* vertexShader = nullptr;
        IShader* fragmentShader = nullptr;
        IShader* geometryShader = nullptr;
        IShader* computeShader = nullptr;

        // Vertex Input Layout
        std::vector<VertexBinding> vertexBindings;
        std::vector<VertexAttribute> vertexAttributes;

        PrimitiveTopology topology;
        RasterizerState rasterizerState;
        DepthStencilState depthStencilState;
        BlendState blendState;

        // For Dynamic Rendering (Vulkan 1.3+)
        // This tells the pipeline what kind of targets it will be used with
        uint32_t colorAttachmentCount;
        TextureFormat colorAttachmentFormats[8];
        TextureFormat depthStencilFormat;

        std::vector<IResourceLayout*> layouts; // The binding "blueprints" used by this pipeline
    };

    class IPipeline
    {
    public:
        virtual ~IPipeline() = default;
        virtual const PipelineDesc& GetDesc() const = 0;
        virtual void* GetNativeHandle() const = 0;
    };
}
