#include "AssetSystem/AssetManager.h"
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include <filesystem>
#include <future>
#include "Rendering/TextureSrc.h"
#include "Core/CoreLib.h"
#include "Scene.h"
#include "Rendering/Model.h"

using namespace std::filesystem;

omp::AssetManager::AssetManager(omp::ThreadPool* threadPool, omp::ObjectFactory* factory)
    : m_ThreadPool(threadPool)
    , m_Factory(factory)
    , m_AssetRegistry()
{
    // TODO: strange
    // loadAssetsFromDrive();
    // TODO: add all classes that should be read
    
    m_Factory->registerClass<omp::TextureSrc>("TextureSrc");
    m_Factory->registerClass<omp::Model>("Model");
    m_Factory->registerClass<omp::Scene>("Scene");
    m_Factory->registerClass<omp::Material>("Material");
}

omp::AssetManager::~AssetManager()
{
    //m_ThreadPool.reset();
}

std::future<bool> omp::AssetManager::loadProject(const std::string& inPath)
{
    return m_ThreadPool->submit([this, inPath]() -> bool
    {
        loadAssetsFromDrive(inPath);
        return true;
    });
}

std::future<bool> omp::AssetManager::saveProject()
{
    return m_ThreadPool->submit([this]() -> bool
    {
        saveAssetsToDrive();
        return true;
    });
}

void omp::AssetManager::saveAsset(AssetHandle assetHandle)
{
    std::shared_ptr<omp::Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    if (found_asset)
    {
        if (!found_asset->isLoaded())
        {
            WARN(LogAssetManager, "Cant save unloaded asset with id: {}", assetHandle.id);
            return;
        }
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
    uint64_t id = omp::CoreLib::generateId64();

    while (m_AssetRegistry.value_for(id, nullptr))
    {
        ERROR(LogAssetManager, "Trying to create asset with existing id!! ID: {}", id);
        id = omp::CoreLib::generateId64();
    } 

    AssetHandle handle(id);
    init_metadata.asset_id = id;
    init_metadata.asset_name = inName;
    init_metadata.path_on_disk = inPath;
    init_metadata.class_id = inClass;
    std::shared_ptr<omp::Asset> new_asset = std::make_shared<omp::Asset>();
    if (new_asset)
    {
        new_asset->specifyMetaData(std::move(init_metadata));
        new_asset->createObject(m_Factory);
        new_asset->addMetadataToObject(new_asset.get(), id);
    }
    else
    {
        ERROR(LogAssetManager, "Asset failed to create, class: {}", inClass);
    }
    m_AssetRegistry.add_or_update_mapping(init_metadata.asset_id, new_asset);

    return handle;
}

void omp::AssetManager::saveAssetsToDrive()
{
    m_AssetRegistry.foreach([](std::pair<AssetHandle, std::shared_ptr<omp::Asset>>& asset)
    {
        INFO(LogAssetManager, "Starting to Save asset: id-{}, path: {}", asset.second->m_Metadata.asset_id, asset.second->m_Metadata.path_on_disk);
        asset.second->saveAsset();
        INFO(LogAssetManager, "AssetSaved: id-{}, path: {}", asset.second->m_Metadata.asset_id, asset.second->m_Metadata.path_on_disk);
    });
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

std::future<std::weak_ptr<omp::Asset>> omp::AssetManager::loadAssetAsync(AssetHandle assetHandle)
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    std::future<std::weak_ptr<Asset>> result;
    if (found_asset)
    {
        result = m_ThreadPool->submit([found_asset, this, assetHandle]()
        {
            auto metadata = found_asset->getMetaData();
            for (auto dependency_id : metadata.dependencies)
            {
                INFO(LogAssetManager, "ASYNC: Start loading dependency for asset {}, dependency: {}", metadata.asset_id, dependency_id);

                std::weak_ptr<omp::Asset> child = loadAsset(dependency_id);
                found_asset->addChild(child.lock());
            }
            
            found_asset->loadAsset(m_Factory);
            return std::weak_ptr<omp::Asset>(found_asset);
        });
    }
    ERROR(LogAssetManager, "Cant find asset with id {0}", assetHandle.id);
    return result;
}

std::future<bool> omp::AssetManager::loadAllAssets()
{
    return m_ThreadPool->submit([this]() -> bool
    {
        m_AssetRegistry.foreach([this](std::pair<AssetHandle, std::shared_ptr<omp::Asset>>& asset)
        {
            asset.second->loadAsset(m_Factory);
        });
        return true;
    });
}

std::weak_ptr<omp::Asset> omp::AssetManager::loadAsset(AssetHandle assetHandle)
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    std::weak_ptr<Asset> result;
    if (found_asset)
    {
        auto metadata = found_asset->getMetaData();
        for (auto dependency_id : metadata.dependencies)
        {
            INFO(LogAssetManager, "Start loading dependency for asset {}, dependency: {}", metadata.asset_id, dependency_id);

            std::weak_ptr<omp::Asset> child = loadAsset(dependency_id);
            found_asset->addChild(child.lock());
        }
        
        found_asset->loadAsset(m_Factory);
        return std::weak_ptr<omp::Asset>(found_asset);
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

