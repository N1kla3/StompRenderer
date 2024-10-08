#include "AssetSystem/AssetManager.h"
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include <filesystem>
#include <future>
#include "Rendering/TextureSrc.h"
#include "Core/CoreLib.h"
#include "Rendering/Shader.h"
#include "Scene.h"
#include "Core/Profiling.h"
#include "SceneEntityFactory.h"
#include "LightSystem.h"
#include "Rendering/Model.h"

using namespace std::filesystem;

omp::AssetManager::AssetManager(omp::ThreadPool* threadPool)
    : m_AssetRegistry()
    , m_ThreadPool(threadPool)
{
    omp::ObjectFactory::registerClass<omp::TextureSrc>("TextureSrc");
    omp::ObjectFactory::registerClass<omp::Model>("Model");
    omp::ObjectFactory::registerClass<omp::Scene>("Scene");
    omp::ObjectFactory::registerClass<omp::Material>("Material");
    omp::ObjectFactory::registerClass<omp::Shader>("Shader");

    omp::SceneEntityFactory::registerClass<omp::SceneEntity>("SceneEntity");
    omp::SceneEntityFactory::registerClass<omp::Camera>("Camera");
    omp::SceneEntityFactory::registerClass<omp::LightObject<omp::GlobalLight>>("GlobalLight");
    omp::SceneEntityFactory::registerClass<omp::LightObject<omp::PointLight>>("PointLight");
    omp::SceneEntityFactory::registerClass<omp::LightObject<omp::SpotLight>>("SpotLight");
}

omp::AssetManager::~AssetManager()
{
    m_AssetRegistry.foreach([](std::pair<AssetHandle, std::shared_ptr<omp::Asset>>& pair)
    {
        pair.second->resetHierarchy();
    });
}

void omp::AssetManager::loadProject(const std::string& inPath)
{
    loadAssetsFromDrive(inPath);
}

std::future<bool> omp::AssetManager::loadProjectAsync(const std::string& inPath)
{
    OMP_STAT_SCOPE("LoadProject");

    if (m_ThreadPool)
    {
        return m_ThreadPool->submit([this, inPath]() -> bool
        {
            loadProject(inPath);
            return true;
        });
    }
    else 
    {
        std::promise<bool> prom;
        std::future<bool> res = prom.get_future();
        loadProject(inPath);
        prom.set_value(true);
        return res;
    }
}

std::future<bool> omp::AssetManager::saveProject()
{
    OMP_STAT_SCOPE("SaveProject");

    if (m_ThreadPool)
    {
        return m_ThreadPool->submit([this]() -> bool
        {
            saveAssetsToDrive();
            return true;
        });
    }
    else
    {
        std::promise<bool> prom;
        std::future<bool> res = prom.get_future();
        saveAssetsToDrive();
        prom.set_value(true);
        return res;
    }
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

        if (m_ThreadPool)
        {
            m_ThreadPool->submit([found_asset]()
            {
                found_asset->saveAsset();
            });
        }
        else
        {
            found_asset->saveAsset();
        }
    }
    else
    {
        WARN(LogAssetManager, "Cant save asset with id specified {}", assetHandle.id);
    }
}

bool omp::AssetManager::deleteAsset(AssetHandle assetHandle)
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    if (found_asset)
    {
        omp::MetaData meta = found_asset->getMetaData();
        found_asset->unloadAsset();
        m_AssetRegistry.remove_mapping(meta.asset_id);
        m_PathRegistry.erase(meta.path_on_disk);
        // TODO: delete file
        return true;
    }
    else
    {
        WARN(LogAssetManager, "Cant delete asset with id specified {}", assetHandle.id);
        return false;
    }
}

omp::AssetHandle omp::AssetManager::createAsset(const std::string& inName, const std::string& inPath, const std::string& inClass)
{
    OMP_STAT_SCOPE("CreateAsset");

    omp::MetaData init_metadata;
    uint64_t id = omp::CoreLib::generateId64();

    if (m_PathRegistry.find(inPath) != m_PathRegistry.end())
    {
        ERROR(LogAssetManager, "Cant create asset while asset with same path exists. Path: {}", inPath);
        return omp::AssetHandle::INVALID_HANDLE;
    }

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
        m_PathRegistry.emplace(inPath, id);
        new_asset->specifyMetaData(std::move(init_metadata));
        new_asset->createObject();
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
    OMP_STAT_SCOPE("SaveAssetsToDrive");

    m_AssetRegistry.foreach([](std::pair<AssetHandle, std::shared_ptr<omp::Asset>>& asset)
    {
        INFO(LogAssetManager, "Starting to Save asset: id-{}, path: {}", asset.second->m_Metadata.asset_id, asset.second->m_Metadata.path_on_disk);
        bool suc = asset.second->saveAsset();
        if (suc)
        {
            INFO(LogAssetManager, "Asset Saved: id-{}, path: {}", asset.second->m_Metadata.asset_id, asset.second->m_Metadata.path_on_disk);
        }
        else
        {
            WARN(LogAssetManager, "Asset cant be Saved: id-{}, path: {}", asset.second->m_Metadata.asset_id, asset.second->m_Metadata.path_on_disk);
        }
    });
}

void omp::AssetManager::loadAssetsFromDrive(const std::string& path)
{
    OMP_STAT_SCOPE("LoadAssetsFromDrive");

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
            loadAssetFromFileSystem_internal(iter.path().string());
        }
    }
}

void omp::AssetManager::loadAssetFromFileSystem_internal(const std::string& inPath)
{
    JsonParser<> file_data{};
    if (file_data.populateFromFile(inPath))
    {
        std::shared_ptr<omp::Asset> asset = std::make_shared<omp::Asset>((std::move(file_data)));
        if (asset->loadMetadata())
        {
            omp::MetaData meta = asset->getMetaData();
            m_AssetRegistry.add_or_update_mapping(meta.asset_id, asset);
            m_PathRegistry.emplace(meta.path_on_disk, meta.asset_id);
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
        if (m_ThreadPool)
        {
            result = m_ThreadPool->submit([found_asset, this]()
            {
                return loadAssetInternal(found_asset);
            });
        }
        else
        {
            std::promise<std::weak_ptr<Asset>> prom;
            result = prom.get_future();
            prom.set_value(loadAssetInternal(found_asset));
            return result;
        }
    }
    ERROR(LogAssetManager, "Cant find asset with id {0}", assetHandle.id);
    return result;
}

std::future<std::weak_ptr<omp::Asset>> omp::AssetManager::loadAssetAsync(const std::string& inPath)
{
    std::future<std::weak_ptr<Asset>> result;
    if (m_PathRegistry.find(inPath) != m_PathRegistry.end())
    {
        return loadAssetAsync(m_PathRegistry[inPath]);
    }
    return result;
}

std::future<bool> omp::AssetManager::loadAllAssets()
{
    OMP_STAT_SCOPE("LoadAllAssets");

    if (m_ThreadPool)
    {
        return m_ThreadPool->submit([this]() -> bool
        {
            m_AssetRegistry.foreach([this](std::pair<AssetHandle, std::shared_ptr<omp::Asset>>& asset)
            {
                asset.second->tryLoadObject();
            });
            return true;
        });
    }
    else
    {
        std::promise<bool> prom;
        std::future<bool> res = prom.get_future();
        m_AssetRegistry.foreach([this](std::pair<AssetHandle, std::shared_ptr<omp::Asset>>& asset)
        {
            asset.second->tryLoadObject();
        });
        prom.set_value(true);
        return res;
    }
}

std::weak_ptr<omp::Asset> omp::AssetManager::loadAsset(AssetHandle assetHandle)
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    std::weak_ptr<Asset> result;
    if (found_asset)
    {
        result = loadAssetInternal(found_asset);
    }
    else
    {
        ERROR(LogAssetManager, "Cant find asset with id {0}", assetHandle.id);
    }
    return result;
}

std::weak_ptr<omp::Asset> omp::AssetManager::loadAsset(const std::string& inPath)
{
    std::weak_ptr<Asset> result;
    if (m_PathRegistry.find(inPath) != m_PathRegistry.end())
    {
        result = loadAsset(m_PathRegistry[inPath]);
    }
    return result;
}

std::weak_ptr<omp::Asset> omp::AssetManager::loadAssetInternal(const std::shared_ptr<omp::Asset>& asset)
{
    auto metadata = asset->getMetaData();
    for (auto dependency_id : metadata.dependencies)
    {
        INFO(LogAssetManager, "Start loading dependency for asset {}, dependency: {}", metadata.asset_id, dependency_id);

        std::weak_ptr<omp::Asset> child = loadAsset(dependency_id);
        asset->addChild(child.lock());
    }

    asset->tryLoadObject();
    return std::weak_ptr<omp::Asset>(asset);
}

std::weak_ptr<omp::Asset> omp::AssetManager::getAsset(AssetHandle assetHandle) const
{
    std::shared_ptr<Asset> found_asset = m_AssetRegistry.value_for(assetHandle, nullptr);
    if (!found_asset)
    {
        ERROR(LogAssetManager, "Cant find asset {0}", assetHandle.id);
    }
    return std::weak_ptr<omp::Asset>(found_asset);
}

std::weak_ptr<omp::Asset> omp::AssetManager::getAsset(const std::string& inPath) const
{
    if (m_PathRegistry.find(inPath) != m_PathRegistry.end())
    {
        return getAsset(m_PathRegistry.at(inPath));
    }
    ERROR(LogAssetManager, "Cant find registered path in Asset manager {0}", inPath);
    return std::weak_ptr<omp::Asset>();
}

