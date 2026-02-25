#pragma once
#include <cstdint>
#include <glm.hpp>

#include "Asset.h"
#include "Vertex.h"
#include "Render/Graphics/GraphicsAPI.h"

class Mesh : public Asset
{
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    BufferHandle vertexBuffer{ 0 };
    BufferHandle indexBuffer{ 0 };

    void UploadToGPU(GraphicsAPI& api)
    {
        if (vertexBuffer.value == 0 && !vertices.empty())
        {
            const size_t sizeInBytes = vertices.size() * sizeof(Vertex);
            vertexBuffer = api.CreateBuffer(sizeInBytes, BufferUsage::Vertex);
            api.UpdateBuffer(vertexBuffer, vertices.data(), sizeInBytes);
        }

        if (indexBuffer.value == 0 && !indices.empty())
        {
            const size_t sizeInBytes = indices.size() * sizeof(uint32_t);
            indexBuffer = api.CreateBuffer(sizeInBytes, BufferUsage::Index);
            api.UpdateBuffer(indexBuffer, indices.data(), sizeInBytes);
        }
    }

    void DeleteGPUResources(GraphicsAPI& api)
    {
        if (vertexBuffer.value != 0)
        {
            api.DeleteBuffer(vertexBuffer);
            vertexBuffer = BufferHandle{ 0 };
        }
        if (indexBuffer.value != 0)
        {
            api.DeleteBuffer(indexBuffer);
            indexBuffer = BufferHandle{ 0 };
        }
    }
};