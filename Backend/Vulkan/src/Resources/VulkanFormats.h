#pragma once
#include <vulkan/vulkan.h>
#include "Texture.h"

namespace GraphicsCore
{
    inline VkFormat GetVulkanFormat(TextureFormat format)
    {
        switch (format) {
        case TextureFormat::R8:              return VK_FORMAT_R8_UNORM;
        case TextureFormat::RG8:             return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RGB8:            return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormat::RGBA8:           return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA8:           return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::R16F:            return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::RG16F:           return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RGB16F:          return VK_FORMAT_R16G16B16_SFLOAT;
        case TextureFormat::RGBA16F:         return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::R32F:            return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::RG32F:           return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RGB32F:          return VK_FORMAT_R32G32B32_SFLOAT;
        case TextureFormat::RGBA32F:         return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::Depth24Stencil8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::Depth32F:        return VK_FORMAT_D32_SFLOAT;
        default:                             return VK_FORMAT_UNDEFINED;
        }
    }
}
