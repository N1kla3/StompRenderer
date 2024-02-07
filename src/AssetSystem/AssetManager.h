#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include "Rendering/VulkanContext.h"
#include "AssetSystem/Asset.h"
#include "AssetSystem/ObjectFactory.h"
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

        template<typename T>
        std::shared_ptr<T> getAssetCasted(const std::string& inPath);
        std::shared_ptr<Asset> getAsset(const std::string& inPath);

        template<typename T>
        std::shared_ptr<T> tryGetAndLoadIfNot_casted(const std::string& inPath);
        std::shared_ptr<Asset> tryGetAndLoadIfNot(const std::string& inPath);

    private:
        void loadAssetsFromDrive();
        void loadAssetsFromDrive(const std::string& pathDirectory);
        void loadAsset_internal(const std::string& inPath);

        // This is the only places to store data
        inline static const std::string_view ASSET_FOLDER = "../assets/";

        inline static const std::string_view ASSET_FORMAT = ".json";

        inline static const std::string_view NAME_MEMBER = "Name";
        inline static const std::string_view CLASS_MEMBER = "Class";
    }; // Asset Manager

    template<class T>
    requires std::is_base_of_v<Asset, T>
    void AssetManager::createAsset(const std::string& inName, const std::string& inPath)
    {
        std::shared_ptr<Asset> asset_ptr = std::make_shared<T>();
        asset_ptr->setName(inName);
        asset_ptr->saveAssetToFile(inPath, omp::ObjectFactory::getClassString<T>());
        m_Assets.insert({inPath, asset_ptr});
    }
}

template<typename T>
std::shared_ptr<T> omp::AssetManager::getAssetCasted(const std::string& inPath)
{
    auto&& ptr = getAsset(inPath);
    auto&& res = dynamic_pointer_cast<T>(ptr);
    return res;
}

template<typename T>
std::shared_ptr<T> omp::AssetManager::tryGetAndLoadIfNot_casted(const std::string& inPath)
{
    auto&& ptr = tryGetAndLoadIfNot(inPath);
    auto&& res = dynamic_pointer_cast<T>(ptr);
    return res;
}
