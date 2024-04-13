#include "AssetSystem/AssetManager.h"
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include <filesystem>
#include <future>
#include "Rendering/TextureSrc.h"
#include "Scene.h"
#include "Rendering/Model.h"

using namespace std::filesystem;

omp::AssetManager::AssetManager(omp::ThreadPool* threadPool, omp::ObjectFactory* factory)
    : m_ThreadPool(threadPool)
    , m_Factory(factory)
{
    // TODO: strange
    // loadAssetsFromDrive();
    // TODO: add all classes that should be read
    
    m_Factory->registerClass<omp::TextureSrc>("TextureSrc");
    m_Factory->registerClass<omp::Model>("Model");
    m_Factory->registerClass<omp::Scene>("Scene");
}

std::future<bool> omp::AssetManager::loadProject(const std::string& inPath)
{
    return m_ThreadPool->submit([this, inPath]() -> bool
    {
        loadAssetsFromDrive(inPath);
        return true;
    });
}

void omp::AssetManager::saveAsset(AssetHandle assetHandle)
{
    std::shared_ptr<omp::Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    if (found_asset)
    {
        // TODO: saving multithreading handling, conflicts
        m_ThreadPool->submit([found_asset]()
        {
            found_asset->saveAsset();
        });
    }
    else
    {
        WARN(LogAssetManager, "Cant save asset with id specified {}", assetHandle.id);
    }
}

void omp::AssetManager::deleteAsset(AssetHandle assetHandle)
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    if (found_asset)
    {
        m_ThreadPool->submit([this, found_asset, assetHandle]()
        {
            found_asset->unloadAsset();
            m_AssetRegistry.remove_mapping(assetHandle);
            // TODO: delete file
        });
    }
    else
    {
        WARN(LogAssetManager, "Cant delete asset with id specified {}", assetHandle.id);
    }
}

omp::AssetHandle omp::AssetManager::createAsset(const std::string& inName, const std::string& inPath, const std::string& inClass)
{
    omp::MetaData init_metadata;
    // TODO: unique id
    AssetHandle handle(1);
    init_metadata.asset_name = inName;
    init_metadata.path_on_disk = inPath;
    init_metadata.class_id = inClass;
    std::shared_ptr<omp::Asset> new_asset = std::make_shared<omp::Asset>();
    new_asset->specifyMetaData(std::move(init_metadata));
    // TODO: check collision
    m_AssetRegistry.add_or_update_mapping(init_metadata.asset_id, new_asset);

    return handle;
}


void omp::AssetManager::loadAssetsFromDrive(const std::string& path)
{
    directory_iterator directory{std::filesystem::path(path)};
    for (auto iter: directory)
    {
        if (iter.is_directory())
        {
            std::string temp_path = iter.path().generic_string();
            loadAssetsFromDrive(temp_path);
        }
        if (iter.path().extension().string() == ASSET_FORMAT)
        {
            loadAsset_internal(iter.path().string());
        }
    }
}

void omp::AssetManager::loadAsset_internal(const std::string& inPath)
{
    JsonParser<> file_data{};
    if (file_data.populateFromFile(inPath))
    {
        std::shared_ptr<omp::Asset> asset = std::make_shared<omp::Asset>((std::move(file_data)));
        if (asset->loadMetadata())
        {
            m_AssetRegistry.add_or_update_mapping(asset->getMetaData().asset_id, asset);
            INFO(LogAssetManager, "Asset loaded successfully: {0}", inPath);
        }
        else
        {
            ERROR(LogAssetManager, "Cannot load asset metadata: {}", inPath);
            // should not be possible
            // TODO: assert
        }
    }
}

std::future<std::weak_ptr<omp::Asset>> omp::AssetManager::loadAsset(AssetHandle assetHandle)
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    std::future<std::weak_ptr<Asset>> result;
    if (found_asset)
    {
        result = m_ThreadPool->submit([found_asset, this, assetHandle]()
        {
            found_asset->loadAsset(m_Factory);
            return std::weak_ptr<omp::Asset>(found_asset);
        });
    }
    ERROR(LogAssetManager, "Cant find asset with id {0}", assetHandle.id);
    return result;
}

std::weak_ptr<omp::Asset> omp::AssetManager::getAsset(AssetHandle assetHandle)
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    if (!found_asset)
    {
        ERROR(LogAssetManager, "Cant find asset {0}", assetHandle.id);
    }
    return std::weak_ptr<omp::Asset>(found_asset);
}

