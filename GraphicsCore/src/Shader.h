#pragma once
#include "Common.h"
#include <cstddef>
#include <vector>

namespace GraphicsCore
{
    // Forward declarations
    class IBuffer;
    class ITexture;
    class ISampler;

    enum class ResourceType
    {
        UniformBuffer,
        StorageBuffer,
        Sampler,
        Texture
    };

    struct ResourceBinding
    {
        uint32_t binding;      // Matches 'layout(binding = X)' in shader
        ResourceType type;     // UBO, SSBO, etc.
        ShaderStage stage;     // Which shader stage uses this (Vertex, Fragment, etc.)
    };

    struct ResourceLayoutDesc
    {
        std::vector<ResourceBinding> bindings;
    };

    struct ShaderDesc
    {
        ShaderStage stage;
        const void* bytecode;
        size_t bytecodeSize;
        const char* entryPoint;
    };

    class IShader
    {
    public:
        virtual ~IShader() = default;
        virtual const ShaderDesc& GetDesc() const = 0;
        virtual void* GetNativeHandle() const = 0;
    };

    // Represents a layout of resources (e.g., "This shader needs 1 UBO at binding 0 and 1 Buffer at binding 1")
    class IResourceLayout
    {
    public:
        virtual ~IResourceLayout() = default;
        virtual void* GetNativeHandle() const = 0;
    };

    // The actual instance of the data bound to that layout
    class IResourceSet
    {
    public:
        virtual ~IResourceSet() = default;

        // GraphicsCore doesn't care about "float_1". It just cares that you are updating the whole UBO.
        virtual void UpdateBuffer(uint32_t binding, IBuffer* buffer, size_t offset, size_t range) = 0;
        virtual void UpdateTexture(uint32_t binding, ITexture* texture, ISampler* sampler) = 0;
    };
}
