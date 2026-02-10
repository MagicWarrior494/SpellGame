#include "VulkanGrahicsAPI.h"

VulkanGraphicsAPI::VulkanGraphicsAPI()
{
    m_vulkanCore = std::make_shared<Vulkan::VulkanCore>();
}

VulkanGraphicsAPI::~VulkanGraphicsAPI()
{
}

BufferHandle VulkanGraphicsAPI::CreateBuffer(size_t size, BufferUsage usage) {
    uint32_t handle = m_nextHandle++;
    Vulkan::VulkanBuffer vkBuf{};
    vkBuf.capacity = size;

    VkBufferUsageFlags vkUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT; // Most buffers need this
    VkMemoryPropertyFlags vkProperties = 0;

    // Route logic based on your existing wrapper patterns
    switch (usage) {
    case BufferUsage::Vertex:
        vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vkProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case BufferUsage::Index:
        vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        vkProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case BufferUsage::Uniform:
        vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        // Uniforms are usually small/frequent, so we keep them mapped
        vkProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    case BufferUsage::Storage:
        vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        // Your wrapper used HOST_VISIBLE for SSBOs
        vkProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    }

    // Call your existing internal wrapper function
    CreateBufferInternal(m_vulkanCore.get(), size, vkUsage, vkProperties, vkBuf);

    // If it's Host Visible, map it now (like your CreateUniformBuffer did)
    if (vkProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        vkMapMemory(m_vulkanCore->vkDevice, vkBuf.memory, 0, size, 0, &vkBuf.mappedPtr);
    }

    vkBuf.usageFlags = vkUsage;
    vkBuf.memoryFlags = vkProperties;

    m_buffers[handle] = vkBuf;
    return handle;
}

void VulkanGraphicsAPI::UpdateBuffer(BufferHandle bufferHandle, const void* data, size_t sizeInBytes) {
    auto it = m_buffers.find(bufferHandle);
    if (it == m_buffers.end()) return;

    Vulkan::VulkanBuffer& vkBuf = it->second;



    EnsureCapacity(m_vulkanCore.get(), vkBuf, sizeInBytes, vkBuf.usageFlags, vkBuf.memoryFlags);

    // 2. Choose Update Path
    if (vkBuf.mappedPtr != nullptr) {
        // Direct Path: For Uniforms/Mappable Storage
        memcpy(vkBuf.mappedPtr, data, sizeInBytes);
        vkBuf.size = sizeInBytes;
    }
    else {
        // Staging Path: For Vertex/Index (Device Local)
        UploadViaStaging(m_vulkanCore.get(), vkBuf, data, sizeInBytes);
    }
}

uint32_t VulkanGraphicsAPI::CreateWindow(GLFWwindow* glfwWindow)
{
	Vulkan::VulkanWindow newWindow{m_vulkanCore, glfwWindow};
	m_windows[m_nextHandle] = std::move(newWindow);
	return m_nextHandle++;
}

void VulkanGraphicsAPI::CloseWindow(uint32_t windowId)
{
    auto it = m_windows.find(windowId);
    if (it != m_windows.end()) {
        // Vulkan resources will be cleaned up in VulkanWindow's destructor
        m_windows.erase(it);
	}
}

void VulkanGraphicsAPI::RenderWindow(uint32_t windowId)
{
    auto it = m_windows.find(windowId);
    if (it == m_windows.end()) return;
    Vulkan::VulkanWindow& window = it->second;
    if (!window.Render())
    {
        // Handle swapchain recreation if needed
        window.RecreateSwapchain();
	}
}
