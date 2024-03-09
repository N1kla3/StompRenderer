#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "AssetSystem/Asset.h"
#include "AssetSystem/ObjectFactory.h"
#include "Async/ThreadPool.h"

namespace omp
{
    class AssetManager
    {
    private:
        std::unordered_map<uint64_t, std::unique_ptr<Asset>> m_AssetRegistry;
        omp::ThreadPool m_ThreadPool;
        omp::ObjectFactory m_Factory;
public:

        AssetManager();
        ~AssetManager() = default;
        AssetManager(const AssetManager&) = delete;
        AssetManager& operator=(const AssetManager&) = delete;
        AssetManager(AssetManager&&) = delete;
        AssetManager& operator=(AssetManager&&) = delete;

        Asset* loadAsset(uint64_t assetId);
        void saveAsset(uint64_t assetId);
        void deleteAsset(uint64_t assetId);
        void createAsset(const std::string& inName, const std::string& inPath, const std::string& inClass);

        Asset* getAsset(uint64_t assetId);

    private:
        void loadAssetsFromDrive();
        void loadAssetsFromDrive(const std::string& pathDirectory);
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
