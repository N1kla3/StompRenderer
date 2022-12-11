#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "VulkanContext.h"
#include "Asset.h"
#include "AssetLoader.h"
#include "Logs.h"

namespace omp
{
    class AssetManager
    {
    private:
        std::unordered_map<std::string, std::shared_ptr<Asset>> m_Assets;

        AssetManager();
        ~AssetManager() = default;
    public:

        AssetManager(const AssetManager&) = delete;
        AssetManager operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager operator=(AssetManager&&) = delete;
        static AssetManager& getAssetManager();

        template<class T>
        requires std::is_base_of_v<Asset, T>
        void createAsset(const std::string& inName, const std::string& inPath);

        std::shared_ptr<Asset> loadAsset(const std::string& inPath);
        void saveAsset(const std::string& inPath);
        void deleteAsset(const std::string& inPath);
        std::shared_ptr<Asset> getAsset(const std::string& inPath);

    private:
        void loadAssetsFromDrive();
        void loadAssetsFromDrive(const std::string& pathDirectory);
        void loadAsset_internal(const std::string& inPath);

    }; // Asset Manager

    template<class T>
    requires std::is_base_of_v<Asset, T>
    void AssetManager::createAsset(const std::string& inName, const std::string& inPath)
    {
        std::shared_ptr<Asset> asset_ptr = std::make_shared<T>();
        asset_ptr->setName(inName);
        asset_ptr->saveAssetToFile(inPath);
        m_Assets.insert({inName, asset_ptr});
    }
}