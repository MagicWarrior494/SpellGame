#pragma once
#include <cstdint>
#include <vector>
#include <glm.hpp>
#include <string>
#include <GLFW/glfw3.h>

#include "World/Assets/ShaderReflected.h"

struct TextureHandle { uint32_t value; };
struct BufferHandle  { uint32_t value; };
struct SamplerHandle { uint32_t value; };
struct ShaderHandle  { uint32_t value; };
struct PipelineLayoutHandle { uint32_t value; };

inline bool operator==(const BufferHandle& a, const BufferHandle& b) { return a.value == b.value; }
inline bool operator!=(const BufferHandle& a, const BufferHandle& b) { return a.value != b.value; }
inline bool operator==(const TextureHandle& a, const TextureHandle& b) { return a.value == b.value; }
inline bool operator!=(const TextureHandle& a, const TextureHandle& b) { return a.value != b.value; }
inline bool operator==(const SamplerHandle& a, const SamplerHandle& b) { return a.value == b.value; }
inline bool operator!=(const SamplerHandle& a, const SamplerHandle& b) { return a.value != b.value; }
inline bool operator==(const ShaderHandle& a, const ShaderHandle& b) { return a.value == b.value; }
inline bool operator!=(const ShaderHandle& a, const ShaderHandle& b) { return a.value != b.value; }
inline bool operator==(const PipelineLayoutHandle& a, const PipelineLayoutHandle& b) { return a.value == b.value; }
inline bool operator!=(const PipelineLayoutHandle& a, const PipelineLayoutHandle& b) { return a.value != b.value; }

namespace std {
    template<> struct hash<BufferHandle>  { size_t operator()(const BufferHandle& h)  const { return hash<uint32_t>()(h.value); } };
    template<> struct hash<TextureHandle> { size_t operator()(const TextureHandle& h) const { return hash<uint32_t>()(h.value); } };
    template<> struct hash<SamplerHandle> { size_t operator()(const SamplerHandle& h) const { return hash<uint32_t>()(h.value); } };
    template<> struct hash<ShaderHandle>  { size_t operator()(const ShaderHandle& h)  const { return hash<uint32_t>()(h.value); } };
	template<> struct hash<PipelineLayoutHandle> { size_t operator()(const PipelineLayoutHandle& h)  const { return hash<uint32_t>()(h.value); } };
}

enum class BufferUsage { Vertex, Index, Uniform, Storage };

class Registry;
class AssetManager;

class GraphicsAPI
{
public:
    GraphicsAPI() = default;
    virtual ~GraphicsAPI() = default;

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
    virtual void DeleteBuffer(BufferHandle bufferHandle) = 0;

    virtual ShaderHandle CreateShader(ShaderStage stage, const std::vector<uint32_t>& code) = 0;
    virtual void DeleteShader(ShaderHandle shaderHandle) = 0;

    virtual TextureHandle CreateTexture(const TextureDescriptor& desc) = 0;
    virtual void UploadTextureData(TextureHandle textureHandle, const void* data, size_t sizeInBytes) = 0;
    virtual void UpdateTexture(TextureHandle textureHandle, const void* data, size_t sizeInBytes) = 0;
    virtual void UpdateTextureRegion(TextureHandle textureHandle, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, const void* data, size_t sizeInBytes) = 0;
    virtual void DeleteTexture(TextureHandle textureHandle) = 0;

    virtual SamplerHandle CreateSampler(const SamplerDescriptor& desc) = 0;
    virtual void DeleteSampler(SamplerHandle samplerHandle) = 0;

    virtual uint32_t CreateWindow(GLFWwindow* glfwWindow) = 0;
    virtual void ResizeWindow(uint32_t windowId, int width, int height) = 0;
    virtual void CloseWindow(uint32_t windowId) = 0;
    virtual void RenderWindow(uint32_t windowId) = 0;

    virtual uint32_t CreateScene(uint32_t windowId) = 0;
    virtual void RenderScene(uint32_t sceneId, Registry& registry, AssetManager& assetManager) = 0;

    virtual void MoveScene(uint32_t sceneId, int x, int y) = 0;
    virtual void ResizeScene(uint32_t sceneId, uint32_t width, uint32_t height) = 0;
    virtual void DeleteScene(uint32_t sceneId) = 0;
    virtual void SetSceneZIndex(uint32_t sceneId, int zIndex) = 0;
    virtual void MoveSceneToWindow(uint32_t sceneId, uint32_t newWindowId) = 0;


};