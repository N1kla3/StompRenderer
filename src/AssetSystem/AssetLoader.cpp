#include "AssetSystem/AssetLoader.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include "Logs.h"

std::shared_ptr<omp::Asset> omp::AssetLoader::loadAssetFromStorage(const std::string& path)
{
    auto file = std::filesystem::directory_entry(path);
    if (file.path().extension().string() == Asset::ASSET_FORMAT)
    {
        std::ifstream stream(file.path().string());
        if (stream.is_open())
        {
            nlohmann::json data;
            stream >> data;
            if (data[Asset::CLASS_MEMBER].is_string() && data[Asset::NAME_MEMBER].is_string())
            {
                std::string class_name = data[Asset::CLASS_MEMBER].get<std::string>();

                stream.close();
                auto&& obj = createClassFromString(class_name);
                obj->setName(data[Asset::NAME_MEMBER]);
                obj->setPath(path);
                return obj;
            }
            VERROR(AssetManager, "Class or Name member not specified: {}", path);
        }
        else
        {
            VERROR(AssetManager, "Something wrong with path: {}", path);
        }
        stream.close();
        return nullptr;
    }
    else
    {
        VERROR(AssetManager, "File name extension is incorrect: {}", path);
        return nullptr;
    }
}

std::shared_ptr<omp::Asset> omp::AssetLoader::createClassFromString(const std::string& name)
{
    if (s_AssetClasses.find(name) != s_AssetClasses.end())
    {
        return  s_AssetClasses.at(name)->CreateAsset();
    }
    ERROR(AssetManager, "Cant create class with name: {}", name);
    // TODO safety
    return nullptr;
}
