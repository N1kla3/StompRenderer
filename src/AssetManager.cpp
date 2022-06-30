#include "AssetManager.h"
#include "Logs.h"
#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"
#include "Material.h"
#include "Model.h"

using namespace std::filesystem;

omp::AssetManager::AssetManager(const std::shared_ptr<VulkanContext> &inHelper)
    : m_VkHelper(inHelper)
{

}

void omp::AssetManager::loadAssetFromString(const std::string &className, const std::string &inName)
{
    //if (className == "Material") loadAsset<Material>(inName);
    //else if (className == "Model") loadAsset<Model>(inName);
}

void omp::AssetManager::saveAsset(const std::string &inName)
{
    if (m_Assets.find(inName) == m_Assets.end())
    {
        ERROR(Rendering, "Cant save asset " + inName);
    }

    auto& asset = m_Assets.at(inName);
    asset->saveAssetToFile(Asset::ASSET_FOLDER + inName + Asset::ASSET_FORMAT);
}

void omp::AssetManager::deleteAsset(const std::string &inName)
{
    if (m_Assets.find(inName) == m_Assets.end())
    {
        ERROR(Rendering, "Cant delete asset " + inName);
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
            std::ifstream stream(iter.path().string());
            if (stream.is_open())
            {
                nlohmann::json data;
                stream >> data;
                if (data["Class"].is_string() && data["Name"].is_string())
                {
                    loadAssetFromString(data["Class"].get<std::string>(), data["Name"].get<std::string>());
                }
            }
        }
    }
}

