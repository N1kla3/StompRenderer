#include "AssetManager.h"
#include "Logs.h"
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"
#include "Material.h"
#include "Model.h"

using namespace std::filesystem;

omp::AssetManager::AssetManager()
{
    loadAssetsFromDrive();
}

void omp::AssetManager::saveAsset(const std::string &inName)
{
    if (m_Assets.find(inName) == m_Assets.end())
    {
        ERROR(AssetManager, "Cant save asset " + inName);
    }

    auto& asset = m_Assets.at(inName);
    asset->saveToLastValidPath();
}

void omp::AssetManager::deleteAsset(const std::string &inName)
{
    if (m_Assets.find(inName) == m_Assets.end())
    {
        ERROR(AssetManager, "Cant delete asset " + inName);
    }
}

void omp::AssetManager::loadAssetsFromDrive()
{
    loadAssetsFromDrive(Asset::ASSET_FOLDER);
}

void omp::AssetManager::loadAssetsFromDrive(const std::string &path)
{
    directory_iterator directory{std::filesystem::path(path)};
    for (auto iter : directory)
    {
        if (iter.is_directory())
        {
            loadAssetsFromDrive(iter.path().generic_string());
        }
        if (iter.path().extension().string() == Asset::ASSET_FORMAT)
        {
            loadAsset(iter.path().string());
        }
    }
}

void omp::AssetManager::loadAsset(const std::string& inPath)
{
    Asset* LoadingAsset = AssetLoader::LoadAssetFromStorage(inPath);
    if (LoadingAsset)
    {
        auto file = std::filesystem::directory_entry(inPath);
        std::ifstream stream(file.path().string());
        if (stream.is_open())
        {
            nlohmann::json data;
            stream >> data;
            LoadingAsset->deserializeData(data);// memory leak watch
            std::shared_ptr<Asset> asset_ptr(LoadingAsset);
            m_Assets.insert({asset_ptr->getPath(), asset_ptr});
            asset_ptr->initialize();
            INFO(AssetManager, "Asset loaded successfully:", asset_ptr->getPath());
        }

        stream.close();
    }
}

omp::AssetManager& omp::AssetManager::GetAssetManager()
{
    static AssetManager Singleton{};
    return Singleton;
}

std::shared_ptr<omp::Asset> omp::AssetManager::getAsset(const std::string &inName)
{
    if (m_Assets.find(inName) == m_Assets.end())
    {
        ERROR(AssetManager, "Cant find asset " + inName);
        return nullptr;
    }
    return m_Assets.at(inName);
}

