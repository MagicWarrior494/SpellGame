#pragma once
#include <variant>
#include <map>
#include <string>
#include <vector>
#include <cstdint>

#include "Render/Graphics/GraphicsAPI.h"

// A generic resource bound to a shader (buffer, texture, or sampler)
struct Resource
{
    std::variant<BufferHandle, TextureHandle, SamplerHandle> data;

    bool isBuffer() const { return std::holds_alternative<BufferHandle>(data); }
    bool isTexture() const { return std::holds_alternative<TextureHandle>(data); }
    bool isSampler() const { return std::holds_alternative<SamplerHandle>(data); }

    // Update the getter methods
    BufferHandle getBuffer() const { return std::get<BufferHandle>(data); }
    TextureHandle getTexture() const { return std::get<TextureHandle>(data); }
    SamplerHandle getSampler() const { return std::get<SamplerHandle>(data); }
};

using ResourceMap = std::map<std::string, Resource>;

enum class BindingType
{
    UniformBuffer,
    StorageBuffer,
    SampledImage,
    CombinedImageSampler
};

// Individual shader binding info
struct ShaderBinding
{
    uint32_t binding;             // GLSL binding = X
    uint32_t set;                 // GLSL set = Y
    std::string name;             // Variable name (e.g., "u_MVP")
    ShaderStage stage; // Shader stage (Vertex/Fragment/Compute)
	BindingType type;           // Type of resource
    uint32_t count;               // Array size (usually 1)
};

struct ShaderDataType
{
    enum class BaseType
    {
        Float,
        Vec2,
        Vec3,
        Vec4,
        Mat4,
        Int,
        IVec2,
        IVec3,
        IVec4
    };
    BaseType baseType;
    uint32_t arraySize;
    uint32_t GetSizeInBytes() const
    {
        uint32_t baseSize = 0;
        switch (baseType)
        {
            case BaseType::Float:  baseSize = 4; break;
            case BaseType::Vec2:   baseSize = 8; break;
            case BaseType::Vec3:   baseSize = 12; break;
            case BaseType::Vec4:   baseSize = 16; break;
            case BaseType::Mat4:   baseSize = 64; break;
            case BaseType::Int:    baseSize = 4; break;
            case BaseType::IVec2:  baseSize = 8; break;
            case BaseType::IVec3:  baseSize = 12; break;
            case BaseType::IVec4:  baseSize = 16; break;
        }
        return baseSize * (arraySize > 1 ? arraySize : 1);
	}
};

struct PushConstantMember
{
    std::string name;
    uint32_t offset;
    uint32_t size;
    ShaderDataType type;
};

struct PushConstantLayout
{
    uint32_t totalSize;
    std::vector<PushConstantMember> members;
};

    // Metadata for a shader
struct ShaderMetadata
{
    std::vector<ShaderBinding> bindings;
    PushConstantLayout pushConstantLayout;
    /*
    *This struct hold all the information about the shader bindings and push constants
	* it came from the reflection of the shader code
    * The goal is the write a resolver which uses the stringname
    * of the binding or push constant to find the user given data at render time
    * This here though is just the template
    */
    

    // Helper to find a binding by name
    const ShaderBinding* FindBinding(const std::string& name) const
    {
        for (const auto& b : bindings)
        {
            if (b.name == name) return &b;
        }
        return nullptr;
    }
};