#include "AssetManager.h"
#include "Logs.h"
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"
#include "Rendering/Material.h"

using namespace std::filesystem;

omp::AssetManager::AssetManager()
{
    loadAssetsFromDrive();
}

void omp::AssetManager::saveAsset(const std::string& inPath)
{
    if (m_Assets.find(inPath) == m_Assets.end())
    {
        ERROR(AssetManager, "Cant save asset " + inPath);
    }

    auto& asset = m_Assets.at(inPath);
    asset->saveToLastValidPath();
}

void omp::AssetManager::deleteAsset(const std::string& inPath)
{
    if (m_Assets.find(inPath) == m_Assets.end())
    {
        ERROR(AssetManager, "Cant delete asset " + inPath);
    }
}

void omp::AssetManager::loadAssetsFromDrive()
{
    loadAssetsFromDrive(Asset::ASSET_FOLDER);
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
        if (iter.path().extension().string() == Asset::ASSET_FORMAT)
        {
            if (!getAsset(iter.path().string()))
            {
                loadAsset_internal(iter.path().string());
            }
        }
    }
}

void omp::AssetManager::loadAsset_internal(const std::string& inPath)
{
    auto&& loading_asset = AssetLoader::loadAssetFromStorage(inPath);
    if (loading_asset)
    {
        auto file = std::filesystem::directory_entry(inPath);
        std::ifstream stream(file.path().string());
        if (stream.is_open())
        {
            nlohmann::json data;
            stream >> data;
            loading_asset->deserializeData(data);// memory leak watch
            m_Assets.erase(loading_asset->getPath());
            m_Assets.insert({loading_asset->getPath(), loading_asset});
            loading_asset->initialize();
            INFO(AssetManager, "Asset loaded successfully: {0}", loading_asset->getPath());
        }

        stream.close();
    }
}

omp::AssetManager& omp::AssetManager::getAssetManager()
{
    static AssetManager singleton{};
    return singleton;
}

std::shared_ptr<omp::Asset> omp::AssetManager::loadAsset(const std::string& inPath)
{
    loadAsset_internal(inPath);
    return getAsset(inPath);
}

std::shared_ptr<omp::Asset> omp::AssetManager::getAsset(const std::string& inPath)
{
    if (m_Assets.find(inPath) == m_Assets.end())
    {
        ERROR(AssetManager, "Cant find asset " + inPath);
        return nullptr;
    }
    return m_Assets.at(inPath);
}

std::shared_ptr<omp::Asset> omp::AssetManager::tryGetAndLoadIfNot(const std::string& inPath)
{
    if (m_Assets.find(inPath) == m_Assets.end())
    {
        loadAsset_internal(inPath);
        return getAsset(inPath);
    }
    return m_Assets.at(inPath);
}

