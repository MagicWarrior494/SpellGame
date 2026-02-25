#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>
#include <type_traits>
#include <stdexcept>

#include "Assets/Asset.h"

class GraphicsAPI; // Forward declaration

class AssetManager
{
public:
    AssetManager() = default;

    void SetAssetRoot(const std::filesystem::path& absoluteRoot)
    {
        if (!std::filesystem::exists(absoluteRoot))
        {
            throw std::runtime_error(
                "Asset root does not exist: " + absoluteRoot.string());
        }
        m_assetRoot = absoluteRoot;
    }

    void SetGraphicsAPI(std::shared_ptr<GraphicsAPI> api)
    {
        m_graphicsAPI = api;
    }

    template<typename T>
    std::shared_ptr<T> Get(const std::string& relativePath)
    {
        static_assert(std::is_base_of_v<Asset, T>,
            "T must derive from Asset");

        std::filesystem::path fullPath = m_assetRoot / relativePath;
        fullPath = std::filesystem::weakly_canonical(fullPath);

        std::string key = fullPath.string();

        // Already loaded?
        auto it = m_assets.find(key);
        if (it != m_assets.end())
        {
            auto casted = std::dynamic_pointer_cast<T>(it->second);
            if (!casted)
            {
                throw std::runtime_error(
                    "Asset type mismatch for: " + key);
            }

            return casted;
        }

        // Load it
        auto asset = LoadInternal<T>(fullPath);

        if (!asset)
        {
            throw std::runtime_error(
                "Failed to load asset: " + key);
        }

        m_assets[key] = std::static_pointer_cast<Asset>(asset);
        return asset;
    }

private:
    template<typename T>
    std::shared_ptr<T> LoadInternal(const std::filesystem::path& fullPath);

private:
    std::unordered_map<std::string, std::shared_ptr<Asset>> m_assets;
    std::filesystem::path m_assetRoot; // absolute path
    std::shared_ptr<GraphicsAPI> m_graphicsAPI;
};