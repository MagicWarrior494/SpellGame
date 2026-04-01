#pragma once
#include "Asset.h"
#include "IRenderer.h"

class Texture : public Asset
{
public:
    ~Texture()
    {
        // GPU resource destroyed via IRenderer by the AssetManager before shutdown
    }

    GraphicsCore::ITexture* texture = nullptr;

    void DeleteGPUResources(GraphicsCore::IRenderer& renderer)
    {
        if (texture) { renderer.DestroyTexture(texture); texture = nullptr; }
    }
};
