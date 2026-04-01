#define STB_IMAGE_IMPLEMENTATION
#include "TextureLoader.h"

#include "stb_image.h"

#include <stdexcept>

std::shared_ptr<Texture> TextureLoader::LoadFromFile(
    GraphicsCore::IRenderer* renderer,
    const std::filesystem::path& filePath)
{
    if (!renderer)
        throw std::runtime_error("TextureLoader: renderer is null");

    if (!std::filesystem::exists(filePath))
        throw std::runtime_error("Texture not found: " + filePath.string());

    int width = 0, height = 0, channels = 0;
    stbi_uc* pixels = stbi_load(filePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels)
        throw std::runtime_error("Failed to decode image: " + filePath.string());

    const uint32_t pixelDataSize = static_cast<uint32_t>(width) * static_cast<uint32_t>(height) * 4u;

    // Create CPU-accessible staging buffer and copy pixels into it
    GraphicsCore::BufferDesc stagingDesc{};
    stagingDesc.size          = pixelDataSize;
    stagingDesc.usage         = GraphicsCore::BufferUsage::Staging;
    stagingDesc.cpuAccessible = true;

    GraphicsCore::IBuffer* stagingBuffer = renderer->CreateBuffer(stagingDesc);
    void* mapped = renderer->MapBuffer(stagingBuffer);
    memcpy(mapped, pixels, pixelDataSize);
    renderer->UnmapBuffer(stagingBuffer);

    stbi_image_free(pixels);

    // Create the GPU texture
    GraphicsCore::TextureDesc texDesc{};
    texDesc.width       = static_cast<uint32_t>(width);
    texDesc.height      = static_cast<uint32_t>(height);
    texDesc.depth       = 1;
    texDesc.mipLevels   = 1;
    texDesc.arrayLayers = 1;
    texDesc.type        = GraphicsCore::TextureType::Texture2D;
    texDesc.format      = GraphicsCore::TextureFormat::RGBA8;
    texDesc.usage       = GraphicsCore::TextureUsage_ShaderResource | GraphicsCore::TextureUsage_TransferDst;

    GraphicsCore::ITexture* gpuTexture = renderer->CreateTexture(texDesc);

    // Upload: barrier to TransferDst, copy, barrier to ShaderResource
    GraphicsCore::ICommandList* cmd = renderer->CreateCommandList();
    cmd->Begin();

    cmd->TextureBarrier(gpuTexture,
                        GraphicsCore::TextureUsage_TransferSrc,
                        GraphicsCore::TextureUsage_TransferDst);

    cmd->CopyBufferToTexture(stagingBuffer, gpuTexture,
                             texDesc.width, texDesc.height);

    cmd->TextureBarrier(gpuTexture,
                        GraphicsCore::TextureUsage_TransferDst,
                        GraphicsCore::TextureUsage_ShaderResource);

    cmd->End();
    renderer->SubmitImmediate(cmd);

    renderer->DestroyCommandList(cmd);
    renderer->DestroyBuffer(stagingBuffer);

    auto texture      = std::make_shared<Texture>();
    texture->texture  = gpuTexture;
    return texture;
}
