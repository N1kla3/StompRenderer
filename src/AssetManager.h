#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "VulkanContext.h"
#include "Asset.h"

namespace omp{
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
    static AssetManager& GetAssetManager();

    template<class T>
    requires std::is_base_of_v<Asset, T>
    void createAsset(const std::string& inName);

    void saveAsset(const std::string& inName);
    void deleteAsset(const std::string& inName);

private:
    void loadAssetsFromDrive();
    void loadAssetsFromDrive(const std::string& path);

    template<class T>
    requires std::is_base_of_v<Asset, T>
    void loadAsset(const std::string& inName);

    void loadAssetFromString(const std::string& className, const std::string& inName);

}; // Asset Manager

    template<class T>
    requires std::is_base_of_v<Asset, T>
    void AssetManager::createAsset(const std::string& inName)
    {
        std::shared_ptr<Asset> asset_ptr = std::make_shared<T>();
        asset_ptr->SetName(inName);
        m_Assets.insert({inName, asset_ptr});
    }

    template<class T>
    requires std::is_base_of_v<Asset, T>
    void AssetManager::loadAsset(const std::string &inName)
    {
        std::shared_ptr<Asset> asset_ptr = std::make_shared<T>();
        asset_ptr->SetName(inName);


        m_Assets.insert({inName, asset_ptr});
    }
}