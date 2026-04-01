#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>
#include <type_traits>
#include <stdexcept>

#include "Assets/Asset.h"
#include "IRenderer.h"

#ifndef CLEVER_ENGINE_API
    #ifdef _WIN32
        #define CLEVER_ENGINE_API __declspec(dllexport)
    #else
        #define CLEVER_ENGINE_API
    #endif
#endif

// Forward declare asset types so we can extern-template LoadInternal
class Mesh;
class Shader;
class Texture;

class CLEVER_ENGINE_API AssetManager
{
public:
    AssetManager() = default;

    void SetAssetRoot(const std::filesystem::path& absoluteRoot)
    {
        if (!std::filesystem::exists(absoluteRoot))
            throw std::runtime_error("Asset root does not exist: " + absoluteRoot.string());

        m_assetRoot = absoluteRoot;
    }

    void SetRenderer(GraphicsCore::IRenderer* renderer)
    {
        m_renderer = renderer;
    }

    GraphicsCore::IRenderer* GetRenderer() const { return m_renderer; }

    void ReleaseAllGPUResources();

    template<typename T>
    std::shared_ptr<T> Get(const std::string& relativePath)
    {
        static_assert(std::is_base_of<Asset, T>::value, "T must derive from Asset");

        std::filesystem::path fullPath = m_assetRoot / relativePath;
        fullPath = std::filesystem::weakly_canonical(fullPath);
        const std::string key = fullPath.string();

        auto it = m_assets.find(key);
        if (it != m_assets.end())
        {
            auto casted = std::dynamic_pointer_cast<T>(it->second);
            if (!casted)
                throw std::runtime_error("Asset type mismatch for: " + key);
            return casted;
        }

        auto asset = LoadInternal<T>(fullPath);
        if (!asset)
            throw std::runtime_error("Failed to load asset: " + key);

        m_assets[key] = std::static_pointer_cast<Asset>(asset);
        return asset;
    }

private:
    template<typename T>
    std::shared_ptr<T> LoadInternal(const std::filesystem::path& fullPath);

    std::unordered_map<std::string, std::shared_ptr<Asset>> m_assets;
    std::filesystem::path m_assetRoot;
    GraphicsCore::IRenderer* m_renderer = nullptr;
};

// Tell consumers that these instantiations live in the DLL — do not instantiate locally
extern template CLEVER_ENGINE_API std::shared_ptr<Mesh>    AssetManager::LoadInternal<Mesh>   (const std::filesystem::path&);
extern template CLEVER_ENGINE_API std::shared_ptr<Shader>  AssetManager::LoadInternal<Shader> (const std::filesystem::path&);
extern template CLEVER_ENGINE_API std::shared_ptr<Texture> AssetManager::LoadInternal<Texture>(const std::filesystem::path&);