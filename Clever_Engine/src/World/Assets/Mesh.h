#pragma once
#include <cstdint>
#include <vector>
#include <glm.hpp>

#include "Asset.h"
#include "Vertex.h"
#include "IRenderer.h"

class Mesh : public Asset
{
public:
    ~Mesh()
    {
        // Buffers are destroyed via IRenderer by the AssetManager before shutdown
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    GraphicsCore::IBuffer* vertexBuffer = nullptr;
    GraphicsCore::IBuffer* indexBuffer  = nullptr;

    void UploadToGPU(GraphicsCore::IRenderer& renderer)
    {
        if (vertexBuffer == nullptr && !vertices.empty())
        {
            GraphicsCore::BufferDesc desc{};
            desc.size          = vertices.size() * sizeof(Vertex);
            desc.usage         = GraphicsCore::BufferUsage::Vertex;
            desc.cpuAccessible = true;

            vertexBuffer = renderer.CreateBuffer(desc);

            void* mapped = renderer.MapBuffer(vertexBuffer);
            memcpy(mapped, vertices.data(), desc.size);
            renderer.UnmapBuffer(vertexBuffer);
        }

        if (indexBuffer == nullptr && !indices.empty())
        {
            GraphicsCore::BufferDesc desc{};
            desc.size          = indices.size() * sizeof(uint32_t);
            desc.usage         = GraphicsCore::BufferUsage::Index;
            desc.cpuAccessible = true;

            indexBuffer = renderer.CreateBuffer(desc);

            void* mapped = renderer.MapBuffer(indexBuffer);
            memcpy(mapped, indices.data(), desc.size);
            renderer.UnmapBuffer(indexBuffer);
        }
    }

    void DeleteGPUResources(GraphicsCore::IRenderer& renderer)
    {
        if (vertexBuffer != nullptr)
        {
            renderer.DestroyBuffer(vertexBuffer);
            vertexBuffer = nullptr;
        }
        if (indexBuffer != nullptr)
        {
            renderer.DestroyBuffer(indexBuffer);
            indexBuffer = nullptr;
        }
    }
};