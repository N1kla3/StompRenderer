#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "Async/threadsafe_map.h"
#include "AssetSystem/Asset.h"
#include "AssetSystem/ObjectFactory.h"
#include "Async/ThreadPool.h"

namespace omp
{
    class AssetManager
    {
    private:
        omp::threadsafe_map<AssetHandle, std::shared_ptr<Asset>> m_AssetRegistry;
        std::unordered_map<std::string, omp::AssetHandle::handle_type> m_PathRegistry;

        omp::ThreadPool* m_ThreadPool = nullptr;
    public:

        AssetManager(omp::ThreadPool* threadPool);
        ~AssetManager();
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager& operator=(AssetManager&&) = delete;

        void loadProject(const std::string& inPath = ASSET_FOLDER);
        std::future<bool> loadProjectAsync(const std::string& inPath = ASSET_FOLDER);
        std::future<bool> saveProject();
        std::future<std::weak_ptr<Asset>> loadAssetAsync(AssetHandle assetId);
        std::future<std::weak_ptr<Asset>> loadAssetAsync(const std::string& inPath);
        std::future<bool> loadAllAssets();
        /* 
         * Try to load asset, if already loaded, will return asset
         * */
        std::weak_ptr<Asset> loadAsset(AssetHandle assetId);
        std::weak_ptr<Asset> loadAsset(const std::string& inPath);
        void saveAsset(AssetHandle assetId);
        bool deleteAsset(AssetHandle assetId);
        AssetHandle createAsset(const std::string& inName, const std::string& inPath, const std::string& inClass);

        [[nodiscard]] std::weak_ptr<Asset> getAsset(AssetHandle assetId) const;
        [[nodiscard]] std::weak_ptr<Asset> getAsset(const std::string& inPath) const;

    private:
        void saveAssetsToDrive();
        void loadAssetsFromDrive(const std::string& pathDirectory = ASSET_FOLDER);
        void loadAssetFromFileSystem_internal(const std::string& inPath);
        std::weak_ptr<Asset> loadAssetInternal(const std::shared_ptr<omp::Asset>& asset);

        // This is the only places to store data
        inline static const std::string ASSET_FOLDER = "../assets/";

        inline static const std::string ASSET_FORMAT = ".json";

        inline static const std::string NAME_MEMBER = "Name";
        inline static const std::string CLASS_MEMBER = "Class";
    }; // Asset Manager

//    {
//        std::shared_ptr<Asset> asset_ptr = std::make_shared<T>();
//        asset_ptr->setName(inName);
//        asset_ptr->saveAssetToFile(inPath, omp::ObjectFactory::getClassString<T>());
//        m_Assets.insert({inPath, asset_ptr});
//    }
}

