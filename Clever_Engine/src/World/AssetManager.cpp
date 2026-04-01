#include "AssetManager.h"

#include "AssetLoaders/Meshloader.h"
#include "AssetLoaders/ShaderLoader.h"
#include "AssetLoaders/TextureLoader.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"
#include "Assets/Texture.h"

void AssetManager::ReleaseAllGPUResources()
{
    if (!m_renderer)
        return;

    for (auto& [key, asset] : m_assets)
    {
        if (auto mesh = std::dynamic_pointer_cast<Mesh>(asset))
            mesh->DeleteGPUResources(*m_renderer);
        else if (auto shader = std::dynamic_pointer_cast<Shader>(asset))
            shader->DeleteGPUResources(*m_renderer);
        else if (auto texture = std::dynamic_pointer_cast<Texture>(asset))
            texture->DeleteGPUResources(*m_renderer);
    }
    m_assets.clear();
}
template<>
std::shared_ptr<Mesh> AssetManager::LoadInternal<Mesh>(const std::filesystem::path& fullPath)
{
    if (!std::filesystem::exists(fullPath))
        throw std::runtime_error("Asset not found: " + fullPath.string());

    std::shared_ptr<Mesh> mesh = MeshLoader::LoadFromFile(fullPath.string());
    mesh->SetName(fullPath.string());

    if (m_renderer)
        mesh->UploadToGPU(*m_renderer);

    return mesh;
}

template<>
std::shared_ptr<Shader> AssetManager::LoadInternal<Shader>(const std::filesystem::path& fullPath)
{
    std::filesystem::path vertPath = std::filesystem::path(fullPath).replace_extension(".vert.spv");
    if (!std::filesystem::exists(vertPath))
        throw std::runtime_error("Shader not found: " + vertPath.string());

    std::shared_ptr<Shader> shader = ShaderLoader::LoadFromFile(m_renderer, fullPath);
    shader->SetName(fullPath.string());
    return shader;
}

template<>
std::shared_ptr<Texture> AssetManager::LoadInternal<Texture>(const std::filesystem::path& fullPath)
{
    if (!std::filesystem::exists(fullPath))
        throw std::runtime_error("Asset not found: " + fullPath.string());

    std::shared_ptr<Texture> texture = TextureLoader::LoadFromFile(m_renderer, fullPath);
    texture->SetName(fullPath.string());
    return texture;
}

// Explicit instantiation definitions — these are exported from the DLL
template CLEVER_ENGINE_API std::shared_ptr<Mesh>    AssetManager::LoadInternal<Mesh>   (const std::filesystem::path&);
template CLEVER_ENGINE_API std::shared_ptr<Shader>  AssetManager::LoadInternal<Shader> (const std::filesystem::path&);
template CLEVER_ENGINE_API std::shared_ptr<Texture> AssetManager::LoadInternal<Texture>(const std::filesystem::path&);