#include "AssetLoader.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include "Logs.h"

omp::Asset* omp::AssetLoader::loadAssetFromStorage(const std::string& path)
{
    auto file = std::filesystem::directory_entry(path);
    if (file.path().extension().string() == Asset::ASSET_FORMAT)
    {
        std::ifstream stream(file.path().string());
        if (stream.is_open())
        {
            nlohmann::json data;
            stream >> data;
            if (data["Class"].is_string() && data["Name"].is_string())
            {
                std::string class_name = data["Class"].get<std::string>();

                stream.close();
                auto* obj = createClassFromString(class_name);
                obj->setName(data["Name"]);
                obj->setPath(path);
                return obj;
            }
        }
        stream.close();
        VERROR(AssetManager, "Something wrong with path: {1}", path);
        return nullptr;
    }
    else
    {
        VERROR(AssetManager, "File name extension is incorrect: {1}", path);
        return nullptr;
    }
}

omp::Asset* omp::AssetLoader::createClassFromString(const std::string& name)
{
    if (CLASS_NAMES.find(name) != CLASS_NAMES.end())
    {
        return std::invoke(CLASS_NAMES.at(name));
    }
    ERROR(AssetManager, "Cant create class with name: {1}", name);
    return nullptr;
}
