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
        // TODO: Get from Application, should not have own thread pool 
        omp::ThreadPool* m_ThreadPool;
        omp::ObjectFactory* m_Factory;
public:

        AssetManager(omp::ThreadPool* threadPool, omp::ObjectFactory* factory);
        ~AssetManager() = default;
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager& operator=(AssetManager&&) = delete;

        std::future<bool> loadProject(const std::string& inPath = ASSET_FOLDER);
        std::future<bool> saveProject();
        std::future<std::weak_ptr<Asset>> loadAsset(AssetHandle assetId);
        void saveAsset(AssetHandle assetId);
        void deleteAsset(AssetHandle assetId);
        AssetHandle createAsset(const std::string& inName, const std::string& inPath, const std::string& inClass);

        std::weak_ptr<Asset> getAsset(AssetHandle assetId);

    private:
        void saveAssetsToDrive();
        void loadAssetsFromDrive(const std::string& pathDirectory = ASSET_FOLDER);
        void loadAsset_internal(const std::string& inPath);

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

