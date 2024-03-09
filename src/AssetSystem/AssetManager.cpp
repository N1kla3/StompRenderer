#include "AssetSystem/AssetManager.h"
#include "Logs.h"
#include <filesystem>

using namespace std::filesystem;

omp::AssetManager::AssetManager()
{
    // TODO: strange
    // loadAssetsFromDrive();
}

void omp::AssetManager::saveAsset(uint64_t assetId)
{
    if (m_AssetRegistry.find(assetId) != m_AssetRegistry.end())
    {
        // TODO: saving multithreading handling, conflicts
        m_ThreadPool.submit([this, assetId]()
        {
            m_AssetRegistry[assetId]->saveAsset();
        });
    }
    else
    {
        WARN(LogAssetManager, "Cant save asset with id specified {}", assetId);
    }
}

void omp::AssetManager::deleteAsset(uint64_t assetID)
{
    if (m_AssetRegistry.find(assetID) != m_AssetRegistry.end())
    {
        m_ThreadPool.submit([this, assetID]()
        {
            m_AssetRegistry[assetID]->unloadAsset();
            // TODO: delete file
        });
    }
    else
    {
        WARN(LogAssetManager, "Cant delete asset with id specified {}", assetID);
    }
}

void omp::AssetManager::loadAssetsFromDrive()
{
    loadAssetsFromDrive(ASSET_FOLDER);
}

void omp::AssetManager::loadAssetsFromDrive(const std::string& path)
{
    directory_iterator directory{std::filesystem::path(path)};
    for (auto iter: directory)
    {
        if (iter.is_directory())
        {
            loadAssetsFromDrive(iter.path().generic_string());
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
        std::unique_ptr<omp::Asset> asset = std::make_unique<omp::Asset>((std::move(file_data)));
        if (asset->loadMetadata())
        {
            m_AssetRegistry.insert({asset->getMetaData().asset_id, std::move(asset)});
            INFO(LogAssetManager, "Asset loaded successfully: {0}", inPath);
        }
        else
        {
            // should not be possible
            // TODO: assert
        }
    }
}

omp::Asset* omp::AssetManager::loadAsset(uint64_t assetId)
{
    if (m_AssetRegistry.at(assetId)->loadAsset(m_Factory))
    {
        return getAsset(assetId);
    }
    ERROR(LogAssetManager, "Cant find asset with id {0}", assetId);
    return nullptr;
}

omp::Asset* omp::AssetManager::getAsset(uint64_t assetId)
{
    if (m_AssetRegistry.find(assetId) == m_AssetRegistry.end())
    {
        ERROR(LogAssetManager, "Cant find asset {0}", assetId);
        return nullptr;
    }
    return m_AssetRegistry.at(assetId).get();
}

