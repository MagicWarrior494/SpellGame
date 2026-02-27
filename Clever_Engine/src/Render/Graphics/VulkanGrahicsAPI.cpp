#include "VulkanGrahicsAPI.h"

#include "CoreTypes/VulkanContructors/Pipeline.h"
#include "CoreTypes/VulkanContructors/PipelineModules.h"

#include "World/Assets/Mesh.h"
#include "World/Assets/Shader.h"
#include "World/Assets/Material.h"
#include "World/ECS/Components.h"

#include <stdexcept>

namespace
{
    VkVertexInputBindingDescription GetVertexBindingDescription()
    {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attrs;
        attrs.resize(3);

        attrs[0].binding = 0;
        attrs[0].location = 0;
        attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[0].offset = offsetof(Vertex, position);

        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[1].offset = offsetof(Vertex, normal);

        attrs[2].binding = 0;
        attrs[2].location = 2;
        attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
        attrs[2].offset = offsetof(Vertex, uv);

        return attrs;
    }

    void EnsureVulkanCoreInitialized(std::shared_ptr<Vulkan::VulkanCore>& vulkanCore)
    {
        if (vulkanCore)
        {
            return;
        }

        vulkanCore = std::make_shared<Vulkan::VulkanCore>();
    }

    VkBufferUsageFlags ToVkBufferUsage(BufferUsage usage)
    {
        switch (usage)
        {
        case BufferUsage::Vertex:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferUsage::Index:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BufferUsage::Uniform:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BufferUsage::Storage:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        default:
            return 0;
        }
    }
}

VulkanGraphicsAPI::VulkanGraphicsAPI() = default;

VulkanGraphicsAPI::~VulkanGraphicsAPI() = default;

BufferHandle VulkanGraphicsAPI::CreateBuffer(size_t size, BufferUsage usage)
{
    EnsureVulkanCoreInitialized(m_vulkanCore);

    if (size == 0)
    {
        return BufferHandle{ 0 };
    }

    Vulkan::VulkanBuffer buffer{};
    buffer.size = size;
    buffer.capacity = size;
    buffer.usageFlags = ToVkBufferUsage(usage) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer.memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	buffer.Create(m_vulkanCore.get(), buffer.capacity, buffer.usageFlags, buffer.memoryFlags);

    const uint32_t handle = m_nextHandle++;
    m_buffers.emplace(handle, std::move(buffer));
    return BufferHandle{ handle };
}

void VulkanGraphicsAPI::UpdateBuffer(BufferHandle bufferHandle, const void* data, size_t sizeInBytes)
{
    EnsureVulkanCoreInitialized(m_vulkanCore);

    if (bufferHandle.value == 0)
    {
        throw std::runtime_error("UpdateBuffer: bufferHandle is 0.");
    }

    auto it = m_buffers.find(bufferHandle.value);
    if (it == m_buffers.end())
    {
        throw std::runtime_error("UpdateBuffer: buffer not found.");
    }

    Vulkan::VulkanBuffer& buffer = it->second;

    if (sizeInBytes > buffer.capacity)
    {
        throw std::runtime_error("UpdateBuffer: sizeInBytes exceeds buffer capacity.");
    }
	buffer.Update(m_vulkanCore.get(), data, sizeInBytes);
}

ShaderHandle VulkanGraphicsAPI::CreateShader(ShaderStage /*stage*/, const std::vector<uint32_t>& code)
{
    EnsureVulkanCoreInitialized(m_vulkanCore);

    if (code.empty())
    {
        return ShaderHandle{ 0 };
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule module = VK_NULL_HANDLE;
    const VkResult result = vkCreateShaderModule(m_vulkanCore->vkDevice, &createInfo, nullptr, &module);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("CreateShader: vkCreateShaderModule failed.");
    }

    const uint32_t handle = m_nextHandle++;
    m_shaders.emplace(handle, module);

    return ShaderHandle{ handle };
}

uint32_t VulkanGraphicsAPI::CreateWindow(GLFWwindow* glfwWindow)
{
    EnsureVulkanCoreInitialized(m_vulkanCore);

    if (glfwWindow == nullptr)
    {
        return 0;
    }

    const uint32_t windowId = m_nextHandle++;
    auto window = std::make_unique<Vulkan::VulkanWindow>(m_vulkanCore, glfwWindow);
    m_windows.emplace(windowId, std::move(window));
    return windowId;
}

void VulkanGraphicsAPI::CloseWindow(uint32_t windowId)
{
    auto it = m_windows.find(windowId);
    if (it == m_windows.end())
    {
        return;
    }

    m_windows.erase(it);
}

void VulkanGraphicsAPI::RenderWindow(uint32_t windowId)
{
    auto it = m_windows.find(windowId);
    if (it == m_windows.end())
    {
        return;
    }

    it->second->Render();
}

uint32_t VulkanGraphicsAPI::CreateScene(uint32_t windowId)
{
    EnsureVulkanCoreInitialized(m_vulkanCore);

    auto winIt = m_windows.find(windowId);
    if (winIt == m_windows.end())
    {
        return 0;
    }

    Vulkan::VulkanWindow* parentWindow = winIt->second.get();

    // For now, use the parent window size as scene size.
    const uint32_t width = parentWindow->GetWindowSize().x;
    const uint32_t height = parentWindow->GetWindowSize().y;

    const int xpos = 0;
    const int ypos = 0;

    const uint32_t sceneId = m_nextHandle++;
    Vulkan::VulkanScene scene(m_vulkanCore, static_cast<int>(width), static_cast<int>(height), xpos, ypos, parentWindow);

    m_scenes.emplace(sceneId, std::move(scene));
    return sceneId;
}
 

VulkanGraphicsAPI::CachedPipeline& VulkanGraphicsAPI::GetOrCreatePipeline(
    Vulkan::VulkanScene& scene,
    ShaderHandle vert,
    ShaderHandle frag)
{
    PipelineKey key{};
    key.renderPass = scene.GetRenderPass();
    key.vertShaderHandle = vert.value;
    key.fragShaderHandle = frag.value;

    auto it = m_pipelineCache.find(key);
    if (it != m_pipelineCache.end())
    {
        return it->second;
    }

    auto vertIt = m_shaders.find(vert.value);
    auto fragIt = m_shaders.find(frag.value);
    if (vertIt == m_shaders.end() || fragIt == m_shaders.end())
    {
        throw std::runtime_error("GetOrCreatePipeline: shader module not found for provided ShaderHandle(s).");
    }

    Vulkan::PipelineLayoutInfo layoutInfo{};
    VkPipelineLayout layout = Vulkan::CreatePipelineLayout(m_vulkanCore.get(), layoutInfo);

    Vulkan::PipelineModulesInfo pso{};
    pso.vertShaderModule = vertIt->second;
    pso.fragShaderModule = fragIt->second;
    pso.pipelineLayout = layout;
    pso.renderPass = scene.GetRenderPass();
    pso.cullMode = VK_CULL_MODE_BACK_BIT;
    pso.enableDepthTest = true;
    pso.bindingDescription = GetVertexBindingDescription();
    pso.attributeDescriptions = GetVertexAttributeDescriptions();

    VkPipeline pipeline = Vulkan::CreateGraphicsPipeline(m_vulkanCore.get(), pso);

    CachedPipeline cached{};
    cached.layout = layout;
    cached.pipeline = pipeline;

    auto [insertIt, inserted] = m_pipelineCache.emplace(key, cached);
    return insertIt->second;
}

void VulkanGraphicsAPI::RenderScene(uint32_t sceneId, Registry& registry, AssetManager& assetManager)
{
    auto it = m_scenes.find(sceneId);
    if (it == m_scenes.end())
    {
        return;
    }

    Vulkan::VulkanScene& scene = it->second;

    auto& meshes = registry.GetAllComponents<MeshComponent>();
    auto& shaders = registry.GetAllComponents<Shader>();
    auto& materials = registry.GetAllComponents<Material>();

    for (const auto& [entity, meshComp] : meshes)
    {
        auto matIt = materials.find(entity);
        if (matIt == materials.end())
        {
            continue;
        }

        auto shader = matInst.material->GetShader();
        if (!shader)
        {
            continue;
        }

        CachedPipeline& pso = GetOrCreatePipeline(scene, shader->vertexHandle, shader->fragmentHandle);

        const uint32_t frameIndex = 0;
        VkCommandBuffer cmd = scene.BeginFrame(frameIndex);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pso.pipeline);

        auto mesh = assetManager.Get<Mesh>(meshComp.assetName);
        if (!mesh)
        {
            scene.EndFrame(cmd);
            return;
        }

        mesh->UploadToGPU(*this);

        if (mesh->vertexBuffer.value == 0 || mesh->indexBuffer.value == 0)
        {
            scene.EndFrame(cmd);
            return;
        }

        auto vbIt = m_buffers.find(mesh->vertexBuffer.value);
        auto ibIt = m_buffers.find(mesh->indexBuffer.value);
        if (vbIt == m_buffers.end() || ibIt == m_buffers.end())
        {
            scene.EndFrame(cmd);
            return;
        }

        VkBuffer vb = vbIt->second.buffer;
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, offsets);

        vkCmdBindIndexBuffer(cmd, ibIt->second.buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);

        scene.EndFrame(cmd);
        return;
    }
}
