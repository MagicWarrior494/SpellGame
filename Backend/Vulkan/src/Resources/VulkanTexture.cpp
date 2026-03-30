#include "VulkanTexture.h"
#include "VulkanFormats.h"
#include <stdexcept>

namespace GraphicsCore {

    static VkImageType GetVulkanImageType(TextureType type) {
        switch (type) {
        case TextureType::Texture1D: return VK_IMAGE_TYPE_1D;
        case TextureType::Texture2D: return VK_IMAGE_TYPE_2D;
        case TextureType::Texture3D: return VK_IMAGE_TYPE_3D;
        case TextureType::TextureCube: return VK_IMAGE_TYPE_2D;
        default: return VK_IMAGE_TYPE_2D;
        }
    }

    static VkImageUsageFlags GetVulkanImageUsage(uint32_t usage) {
        VkImageUsageFlags flags = 0;
        if (usage & TextureUsage_ShaderResource) flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (usage & TextureUsage_RenderTarget) flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (usage & TextureUsage_DepthStencil) flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (usage & TextureUsage_Storage) flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (usage & TextureUsage_TransferSrc) flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (usage & TextureUsage_TransferDst) flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        return flags;
    }

    VulkanTexture::VulkanTexture(VkDevice device, VmaAllocator allocator, const TextureDesc& desc)
        : m_desc(desc), m_device(device), m_allocator(allocator), 
          m_image(VK_NULL_HANDLE), m_imageView(VK_NULL_HANDLE), 
          m_allocation(VK_NULL_HANDLE), m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED),
          m_ownsImage(true)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = GetVulkanImageType(desc.type);
        imageInfo.format = GetVulkanFormat(desc.format);
        imageInfo.extent.width = desc.width;
        imageInfo.extent.height = desc.height;
        imageInfo.extent.depth = desc.depth;
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = desc.arrayLayers;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = GetVulkanImageUsage(desc.usage);
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (desc.type == TextureType::TextureCube) {
            imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VkResult result = vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &m_image, &m_allocation, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan image");
        }

        CreateImageView();
    }

    // Special constructor for swapchain images (doesn't own the VkImage)
    VulkanTexture::VulkanTexture(VkDevice device, VkImage image, VkFormat format, uint32_t width, uint32_t height)
        : m_device(device), m_allocator(VK_NULL_HANDLE),
          m_image(image), m_imageView(VK_NULL_HANDLE),
          m_allocation(VK_NULL_HANDLE), m_currentLayout(VK_IMAGE_LAYOUT_UNDEFINED),
          m_ownsImage(false)
    {
        // Setup minimal texture desc for swapchain image
        m_desc.width = width;
        m_desc.height = height;
        m_desc.depth = 1;
        m_desc.mipLevels = 1;
        m_desc.arrayLayers = 1;
        m_desc.type = TextureType::Texture2D;
        m_desc.usage = TextureUsage_RenderTarget;

        // Convert VkFormat back to TextureFormat
        if (format == VK_FORMAT_B8G8R8A8_UNORM) {
            m_desc.format = TextureFormat::BGRA8;
        } else if (format == VK_FORMAT_R8G8B8A8_UNORM) {
            m_desc.format = TextureFormat::RGBA8;
        } else {
            m_desc.format = TextureFormat::BGRA8; // Default fallback
        }

        CreateImageView();
    }

    VulkanTexture::~VulkanTexture() {
        if (m_imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(m_device, m_imageView, nullptr);
        }
        // Only destroy the image if we own it (not a swapchain image)
        if (m_ownsImage && m_image != VK_NULL_HANDLE) {
            vmaDestroyImage(m_allocator, m_image, m_allocation);
        }
    }

    void VulkanTexture::CreateImageView() {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_image;
        viewInfo.format = GetVulkanFormat(m_desc.format);

        switch (m_desc.type) {
        case TextureType::Texture1D:
            viewInfo.viewType = m_desc.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        case TextureType::Texture2D:
            viewInfo.viewType = m_desc.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            break;
        case TextureType::Texture3D:
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case TextureType::TextureCube:
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        }

        if (m_desc.usage & TextureUsage_DepthStencil) {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (m_desc.format == TextureFormat::Depth24Stencil8) {
                viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = m_desc.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = m_desc.arrayLayers;

        VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &m_imageView);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan image view");
        }
    }
}
