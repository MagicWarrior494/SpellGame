#pragma once
#include "Common.h"
#include <cstdint>

namespace GraphicsCore
{

    struct TextureDesc {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t mipLevels;
        uint32_t arrayLayers;
        TextureType type;
        TextureFormat format;
        uint32_t usage; // Bitmask of TextureUsageFlags
    };

    class ITexture
    {
    public:
        virtual ~ITexture() = default;
        virtual const TextureDesc& GetDesc() const = 0;
        virtual void* GetNativeHandle() const = 0;
    };
}
