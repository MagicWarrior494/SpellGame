#pragma once
#include "Texture.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace GraphicsCore {
    class VulkanTexture : public ITexture {
    public:
        VulkanTexture(VkDevice device, VmaAllocator allocator, const TextureDesc& desc);

        // Special constructor for swapchain images (doesn't own the VkImage)
        VulkanTexture(VkDevice device, VkImage image, VkFormat format, uint32_t width, uint32_t height);

        ~VulkanTexture();

        const TextureDesc& GetDesc() const override { return m_desc; }
        void* GetNativeHandle() const override { return (void*)m_image; }

        VkImage GetImage() const { return m_image; }
        VkImageView GetImageView() const { return m_imageView; }
        VmaAllocation GetAllocation() const { return m_allocation; }
        VkImageLayout GetCurrentLayout() const { return m_currentLayout; }
        void SetCurrentLayout(VkImageLayout layout) { m_currentLayout = layout; }
        bool OwnsImage() const { return m_ownsImage; }

    private:
        void CreateImageView();

        TextureDesc m_desc;
        VkDevice m_device;
        VmaAllocator m_allocator;
        VkImage m_image;
        VkImageView m_imageView;
        VmaAllocation m_allocation;
        VkImageLayout m_currentLayout;
        bool m_ownsImage;  // False for swapchain images
    };
}
