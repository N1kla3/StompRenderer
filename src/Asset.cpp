#include <filesystem>
#include <fstream>
#include "Asset.h"
#include "Logs.h"
#include "AssetLoader.h"
#include "nlohmann/json.hpp"

void omp::Asset::saveAssetToFile(const std::string &path)
{
    WARN(UI, "Asset " + m_Name + " is not saveable");
}

void omp::Asset::loadAssetFromFile(const std::string &path)
{
    Asset* LoadingAsset = AssetLoader::LoadAssetFromStorage(path);
    if (LoadingAsset)
    {
        auto file = std::filesystem::directory_entry(path);
        std::ifstream stream(file.path().string());
        if (stream.is_open())
        {
            nlohmann::json data;
            stream >> data;
            LoadingAsset->deserializeData(data);// memory leak watch
        }

        stream.close();
    }
    WARN(UI, "Asset " + m_Name + " is not loadable");
}
