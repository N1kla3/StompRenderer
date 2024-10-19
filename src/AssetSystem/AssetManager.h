#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include "Async/threadsafe_map.h"
#include "AssetSystem/Asset.h"
#include "AssetSystem/ObjectFactory.h"
#include "AssetSystem/ProjectSettings.h"
#include "Async/ThreadPool.h"
#include "Scene.h"

namespace omp
{
    class AssetManager
    {
    private:
        omp::threadsafe_map<AssetHandle, std::shared_ptr<Asset>> m_AssetRegistry;
        std::unordered_map<std::string, omp::AssetHandle::handle_type> m_PathRegistry;
        omp::ProjectSettings m_ProjectSettings;

        omp::ThreadPool* m_ThreadPool = nullptr;
    public:

        explicit AssetManager(omp::ThreadPool* threadPool);
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
        std::weak_ptr<Asset> loadAsset(AssetHandle assetHandle);
        std::weak_ptr<Asset> loadAsset(const std::string& inPath);
        void saveAsset(AssetHandle assetHandle);
        bool deleteAsset(AssetHandle assetHandle);
        AssetHandle createAsset(const std::string& inName, const std::string& inPath, const std::string& inClass);

        [[nodiscard]] std::weak_ptr<Asset> getAsset(AssetHandle assetHandle) const;
        [[nodiscard]] std::weak_ptr<Asset> getAsset(const std::string& inPath) const;

        std::shared_ptr<omp::Scene> tryLoadProjectDefaultMap();
        static void unloadMap(const std::shared_ptr<omp::Scene>& scene, bool save);

    private:
        void saveAssetsToDrive();
        void loadAssetsFromDrive(const std::string& pathDirectory = ASSET_FOLDER);
        void loadAssetFromFileSystem_internal(const std::string& inPath);
        std::weak_ptr<Asset> loadAssetInternal(const std::shared_ptr<omp::Asset>& asset);

        bool tryLoadProjectFile(const std::string& dirPath);

        // This is the only places to store data
        inline static const std::string ASSET_FOLDER = "../assets/";

        inline static const std::string ASSET_FORMAT = ".json";
        inline static const std::string PROJECT_FORMAT = ".stomp";

        inline static const std::string NAME_MEMBER = "Name";
        inline static const std::string CLASS_MEMBER = "Class";
    }; // Asset Manager
}

