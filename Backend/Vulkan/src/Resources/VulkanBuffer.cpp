#include "VulkanBuffer.h"
#include <stdexcept>

namespace GraphicsCore {

    static VkBufferUsageFlags GetVulkanBufferUsage(BufferUsage usage) {
        switch (usage) {
        case BufferUsage::Vertex:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferUsage::Index:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferUsage::Uniform:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferUsage::Storage:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferUsage::Staging:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        default:
            return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
    }

    VulkanBuffer::VulkanBuffer(VkDevice device, VmaAllocator allocator, const BufferDesc& desc)
        : m_desc(desc), m_device(device), m_allocator(allocator), m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = desc.size;
        bufferInfo.usage = GetVulkanBufferUsage(desc.usage);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        if (desc.cpuAccessible) {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
                              VMA_ALLOCATION_CREATE_MAPPED_BIT;
        } else {
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        }

        VkResult result = vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan buffer");
        }
    }

    VulkanBuffer::~VulkanBuffer() {
        if (m_buffer != VK_NULL_HANDLE) {
            vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
        }
    }
}
