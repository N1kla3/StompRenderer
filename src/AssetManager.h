#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "VulkanHelper.h"
#include "Asset.h"

namespace omp{
class AssetManager
{
private:
    std::unordered_map<std::string, std::shared_ptr<Asset>> m_Assets;

    std::shared_ptr<VulkanHelper> m_VkHelper;
public:
    AssetManager(const std::shared_ptr<VulkanHelper>& inHelper);

    template<class T>
    void createAsset(const std::string& inName);

    void saveAsset(const std::string& inName);
    void deleteAsset(const std::string& inName);

private:
    void loadAssetsFromDrive();
    void loadAssetsFromDrive(const std::string& path);

    template<class T>
    void loadAsset(const std::string& inName);

    void loadAssetFromString(const std::string& className, const std::string& inName);

}; // Asset Manager

    template<class T>
    void AssetManager::createAsset(const std::string& inName)
    {
        static_assert(std::is_base_of_v<Asset, T>);

        std::shared_ptr<Asset> asset_ptr = std::make_shared<T>();
        asset_ptr->SetName(inName);
        m_Assets.insert({inName, asset_ptr});
        asset_ptr->initialize();
    }

    template<class T>
    void AssetManager::loadAsset(const std::string &inName)
    {
        static_assert(std::is_base_of_v<Asset, T>);

        std::shared_ptr<Asset> asset_ptr = std::make_shared<T>();
        asset_ptr->SetName(inName);

        asset_ptr->loadAssetFromFile(Asset::ASSET_FOLDER + inName + Asset::ASSET_FORMAT);

        m_Assets.insert({inName, asset_ptr});
        asset_ptr->initialize();
    }
}