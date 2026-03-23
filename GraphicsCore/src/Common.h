#pragma once

namespace GraphicsCore
{
    enum class ShaderStage
    {
        Vertex,
        Fragment,
        Compute,
        Geometry,
        TessellationControl,
        TessellationEvaluation
    };

    enum class BufferUsage
    {
        Vertex,
        Index,
        Uniform,
        Storage,
        Staging
    };

    enum class TextureType
    {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube
    };

    enum class TextureFormat
    {
        R8,
        RG8,
        RGB8,
        RGBA8,
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,
        Depth24Stencil8,
        Depth32F
    };

    enum TextureUsageFlags : uint32_t {
        TextureUsage_ShaderResource = 1 << 0, // Can be read in a shader
        TextureUsage_RenderTarget = 1 << 1, // Can be a color output
        TextureUsage_DepthStencil = 1 << 2, // Can be a depth/stencil buffer
        TextureUsage_Storage = 1 << 3, // For Compute Shader R/W
        TextureUsage_TransferSrc = 1 << 4, // Can be copied FROM
        TextureUsage_TransferDst = 1 << 5  // Can be copied TO
    };

    enum class FilterMode
    {
        Nearest,
        Linear
    };

    enum class WrapMode
    {
        Repeat,
        ClampToEdge,
        MirroredRepeat,
        ClampToBorder
    };

    enum class CompareOp
    {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    enum class PrimitiveTopology
    {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan
    };

    enum class CullMode
    {
        None,
        Front,
        Back,
        FrontAndBack
    };

    enum class BlendFactor
    {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha
    };

    enum class BlendOp
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };
}
