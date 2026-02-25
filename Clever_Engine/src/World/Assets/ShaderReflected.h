#pragma once

#include <string>
#include <vector>
#include <cstdint>

//
// ============================================================
//  Shader Stage Flags
// ============================================================
//

enum class ShaderStage : uint32_t
{
    None = 0,
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Compute = 1 << 2,
    Geometry = 1 << 3,
    TessControl = 1 << 4,
    TessEval = 1 << 5,

    AllGraphics = Vertex | Fragment | Geometry | TessControl | TessEval,
    All = 0xFFFFFFFF
};

inline ShaderStage operator|(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
        );
}

inline ShaderStage operator&(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
        );
}

inline ShaderStage& operator|=(ShaderStage& a, ShaderStage b)
{
    a = a | b;
    return a;
}

//
// ============================================================
//  Descriptor Resource Types (Vulkan Complete)
// ============================================================
//

enum class ShaderResourceType
{
    UniformBuffer,
    StorageBuffer,
    UniformBufferDynamic,
    StorageBufferDynamic,

    SampledImage,
    StorageImage,
    CombinedImageSampler,
    Sampler,
    InputAttachment,

    AccelerationStructure,

    Unknown
};

//
// ============================================================
//  Reflected Struct Member (UBO / SSBO Layout)
// ============================================================
//

struct ReflectedMember
{
    std::string name;   // e.g. "viewProj", "lightDir"
    uint32_t offset = 0;
    uint32_t size = 0;
};

//
// ============================================================
//  Descriptor Resource (Unified Binding Representation)
// ============================================================
//

struct ShaderResource
{
    std::string name;

    uint32_t set = 0;
    uint32_t binding = 0;

    ShaderResourceType type = ShaderResourceType::Unknown;
    ShaderStage stages = ShaderStage::None;

    uint32_t descriptorCount = 1;   // For arrays
    bool readOnly = true;           // For storage buffers/images
    bool bindless = false;          // Descriptor indexing

    // --------------------------------------------------------
    // Buffer-specific (UBO / SSBO)
    // --------------------------------------------------------

    uint32_t size = 0;   // Total buffer size (if applicable)
    std::vector<ReflectedMember> members;

    // --------------------------------------------------------
    // Image-specific (Storage images)
    // --------------------------------------------------------

    // Only relevant for StorageImage
    // Define your own ImageFormat enum in engine
    uint32_t imageFormat = 0;  // Optional engine-defined format enum
};

//
// ============================================================
//  Push Constants
// ============================================================
//

struct PushConstantMember
{
    std::string name;
    uint32_t offset = 0;
    uint32_t size = 0;

    // Define ShaderDataType in your engine
    uint32_t type = 0;
};

struct PushConstantLayout
{
    uint32_t totalSize = 0;
    ShaderStage stages = ShaderStage::None;

    std::vector<PushConstantMember> members;
};

//
// ============================================================
//  Vertex Input Reflection
// ============================================================
//

enum class VertexInputRate
{
    PerVertex,
    PerInstance
};

struct VertexAttribute
{
    std::string name;
    uint32_t location = 0;

    // Define ShaderDataType in your engine
    uint32_t type = 0;

    VertexInputRate inputRate = VertexInputRate::PerVertex;
};

struct VertexLayout
{
    std::vector<VertexAttribute> attributes;
};

//
// ============================================================
//  Specialization Constants
// ============================================================
//

struct SpecializationConstant
{
    uint32_t id = 0;
    std::string name;

    // Define ShaderDataType in your engine
    uint32_t type = 0;

    uint32_t size = 0;
};

//
// ============================================================
//  Full Shader Reflection Container
// ============================================================
//

struct ShaderReflection
{
    std::vector<ShaderResource> resources;

    PushConstantLayout pushConstants;

    VertexLayout vertexLayout;

    std::vector<SpecializationConstant> specializationConstants;
};