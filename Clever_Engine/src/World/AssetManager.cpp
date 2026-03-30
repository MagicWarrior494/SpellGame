#include "AssetManager.h"

#include "AssetLoaders/Meshloader.h"
#include "AssetLoaders/ShaderLoader.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"
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

// Explicit instantiation definitions — these are exported from the DLL
template CLEVER_ENGINE_API std::shared_ptr<Mesh>   AssetManager::LoadInternal<Mesh>  (const std::filesystem::path&);
template CLEVER_ENGINE_API std::shared_ptr<Shader> AssetManager::LoadInternal<Shader>(const std::filesystem::path&);