#pragma once
#include <cstdint>
#include <vector>
#include <glm.hpp>

using TextureHandle = uint32_t;
using BufferHandle = uint32_t;
using ShaderHandle = uint32_t;

class GraphicsAPI
{
public:
	GraphicsAPI() = default;
	virtual ~GraphicsAPI() = default;

	enum class BufferUsage { Vertex, Index, Uniform, Storage };
    enum class ShaderStage { Vertex, Fragment, Compute };
    enum class TextureFormat { RGBA8, R32_FLOAT, SRGB8, Depth32, FloatingPoint16 };

    enum class SamplerFilterMode { Linear, Nearest };
    enum class SamplerWrapMode { Repeat, Clamp };

    struct TextureDescriptor {
        uint32_t width;
        uint32_t height;
        TextureFormat format;
        bool generateMipmaps = false;
    };

    struct SamplerDescriptor {
        SamplerFilterMode filter;
        SamplerWrapMode wrap;
    };

	virtual BufferHandle CreateBuffer(size_t size, BufferUsage usage) = 0;
    virtual void UpdateBuffer(BufferHandle bufferHandle, const void* data, size_t sizeInBytes) = 0;

    template<typename T>
    void UpdateCustomBuffer(BufferHandle bufferHandle, const std::vector<T>& dataList) {
        size_t calculatedSize = dataList.size() * sizeof(T);
        UpdateBuffer(bufferHandle, dataList.data(), calculatedSize);
    }
	virtual void DeleteBuffer(BufferHandle bufferHandle) = 0;

    virtual ShaderHandle CreateShader(ShaderStage stage, const std::vector<char>& code) = 0;
    virtual void DeleteShader(ShaderHandle shaderHandle) = 0;

    virtual TextureHandle CreateTexture(const TextureDescriptor& desc) = 0;

    // Note: used void* here because texture data is usually loaded 
    // from a library like stb_image as a raw byte array.
    virtual void UploadTextureData(TextureHandle textureHandle, const void* data, size_t sizeInBytes) = 0;
    virtual void UpdateTexture(uint32_t textureHandle, const void* data, size_t sizeInBytes) = 0;
	virtual void UpdateTextureRegion(TextureHandle textureHandle, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, const void* data, size_t sizeInBytes) = 0;
    virtual void DeleteTexture(TextureHandle textureHandle) = 0;

    virtual uint32_t CreateSampler(const SamplerDescriptor& desc) = 0;
    virtual void DeleteSampler(uint32_t samplerHandle) = 0;
};