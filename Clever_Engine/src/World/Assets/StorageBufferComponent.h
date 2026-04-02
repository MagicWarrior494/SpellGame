#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include "IRenderer.h"
#include "ShaderBinding.h"

// One named SSBO entry owned by StorageBufferComponent.
// Holds the CPU-side bytes and the managed GPU buffer.
struct StorageBufferEntry
{
    std::string             name;           // Must match the GLSL instance name
    std::vector<uint8_t>    data;           // Raw CPU bytes
    GraphicsCore::IBuffer*  gpuBuffer = nullptr;
    bool                    dirty     = true;

    // Set new data from any trivially-copyable type vector.
    template<typename T>
    void SetData(const std::vector<T>& src)
    {
        static_assert(std::is_trivially_copyable_v<T>,
            "StorageBufferEntry::SetData requires a trivially copyable element type");
        data.resize(src.size() * sizeof(T));
        std::memcpy(data.data(), src.data(), data.size());
        dirty = true;
    }

    // Upload to GPU — creates or resizes the buffer as needed.
    void Upload(GraphicsCore::IRenderer& renderer)
    {
        if (!dirty || data.empty())
            return;

        // Recreate if size changed or not yet created
        if (gpuBuffer && gpuBuffer->GetDesc().size != data.size())
        {
            renderer.DestroyBuffer(gpuBuffer);
            gpuBuffer = nullptr;
        }

        if (!gpuBuffer)
        {
            GraphicsCore::BufferDesc desc{};
            desc.size          = data.size();
            desc.usage         = GraphicsCore::BufferUsage::Storage;
            desc.cpuAccessible = true;
            gpuBuffer = renderer.CreateBuffer(desc);
        }

        void* mapped = renderer.MapBuffer(gpuBuffer);
        std::memcpy(mapped, data.data(), data.size());
        renderer.UnmapBuffer(gpuBuffer);

        dirty = false;
    }

    void Destroy(GraphicsCore::IRenderer& renderer)
    {
        if (gpuBuffer)
        {
            renderer.DestroyBuffer(gpuBuffer);
            gpuBuffer = nullptr;
        }
    }
};

// Per-entity component that manages one or more named SSBOs.
// Usage:
//   StorageBufferComponent ssbo;
//   ssbo.Bind("instancePositions", positions);   // vector<glm::vec4>
//   ssbo.Bind("lightData",         lights);      // vector<LightData>
//   registry.Set<StorageBufferComponent>(entity, ssbo);
//
// The render loop calls Upload() each frame and merges the resulting
// BufferBindings into the descriptor set resolution pass automatically.
struct StorageBufferComponent
{
    std::vector<StorageBufferEntry> entries;

    // Bind or replace an SSBO by name with data from any trivially-copyable vector.
    template<typename T>
    StorageBufferComponent& Bind(const std::string& name, const std::vector<T>& data)
    {
        for (auto& e : entries)
        {
            if (e.name == name)
            {
                e.SetData(data);
                return *this;
            }
        }
        StorageBufferEntry entry{};
        entry.name = name;
        entry.SetData(data);
        entries.push_back(std::move(entry));
        return *this;
    }

    // Upload all dirty entries to the GPU. Called automatically by the render loop.
    void Upload(GraphicsCore::IRenderer& renderer)
    {
        for (auto& e : entries)
            e.Upload(renderer);
    }

    // Produce a BufferBinding list for the descriptor set resolution pass.
    std::vector<BufferBinding> GetBindings() const
    {
        std::vector<BufferBinding> result;
        result.reserve(entries.size());
        for (const auto& e : entries)
        {
            if (e.gpuBuffer)
                result.push_back({ e.name, e.gpuBuffer, 0, 0 });
        }
        return result;
    }

    // Must be called before the component is destroyed or removed.
    void Destroy(GraphicsCore::IRenderer& renderer)
    {
        for (auto& e : entries)
            e.Destroy(renderer);
        entries.clear();
    }
};
