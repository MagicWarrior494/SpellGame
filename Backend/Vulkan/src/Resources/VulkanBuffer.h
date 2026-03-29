#pragma once
#include "Buffer.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace GraphicsCore {
    class VulkanBuffer : public IBuffer {
    public:
        VulkanBuffer(VkDevice device, VmaAllocator allocator, const BufferDesc& desc);
        ~VulkanBuffer();

        const BufferDesc& GetDesc() const override { return m_desc; }
        void* GetNativeHandle() const override { return (void*)m_buffer; }

        VkBuffer GetBuffer() const { return m_buffer; }
        VmaAllocation GetAllocation() const { return m_allocation; }

    private:
        BufferDesc m_desc;
        VkDevice m_device;
        VmaAllocator m_allocator;
        VkBuffer m_buffer;
        VmaAllocation m_allocation;
    };
}
