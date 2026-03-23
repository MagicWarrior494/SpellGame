#include "AssetManager.h"

#include "AssetLoaders/Meshloader.h"
#include "AssetLoaders/ShaderLoader.h"
#include "Assets/Mesh.h"
#include "Assets/Shader.h"

template<>
std::shared_ptr<Mesh> AssetManager::LoadInternal<Mesh>(const std::filesystem::path& path)
{
    std::filesystem::path fullPath = m_assetRoot / path;
    if (!std::filesystem::exists(fullPath))
    {
        throw std::runtime_error("Asset not found: " + fullPath.string());
    }
    std::shared_ptr<Mesh> mesh = MeshLoader::LoadFromFile(fullPath.string());
	mesh->SetName(path.string()); // Store the relative path as the asset name for reference

	m_assets.insert({ path.string(), mesh});

    if(m_graphicsAPI)
		mesh->UploadToGPU(*m_graphicsAPI);

    return mesh;
}

template<>
std::shared_ptr<Shader> AssetManager::LoadInternal<Shader>(const std::filesystem::path& path)
{
    std::filesystem::path fullPath = m_assetRoot / path;
    if (!std::filesystem::exists(fullPath))
    {
        throw std::runtime_error("Asset not found: " + fullPath.string());
    }

    std::shared_ptr<Shader> shader = ShaderLoader::LoadFromFile(m_graphicsAPI, fullPath);
    shader->SetName(path.string());

    //Create Descriptor Layouts using Spir-v reflection data
//Gets stored in GraphicsAPI and uses the shaderHandle as the key



//Create pipeline LAYOUT using descriptor layouts and push constant info from reflection data
//Gets stored in GraphicsAPI and uses the shaderHandle as the key

    m_assets.insert({ path.string(), shader });

    shader->UploadToGPU(*m_graphicsAPI);

    return shader;
}