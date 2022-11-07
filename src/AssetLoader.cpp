#include "AssetLoader.h"
#include "nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include "Logs.h"

omp::Asset* omp::AssetLoader::LoadAssetFromStorage(const std::string &path)
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
                auto* Obj = CreateClassFromString(class_name);
                Obj->setName(data["Name"]);
                Obj->setPath(path);
                return Obj;
            }
        }
        stream.close();
        VERROR(AssetManager, "Something wrong with path: ", path);
        return nullptr;
    }
    else
    {
        VERROR(AssetManager, "File name extension is incorrect: ", path);
        return nullptr;
    }
}

omp::Asset* omp::AssetLoader::CreateClassFromString(const std::string &name)
{
    if (ClassNames.find(name) != ClassNames.end())
    {
        return std::invoke(ClassNames.at(name));
    }
    ERROR(AssetManager, "Cant create class with name: ", name);
    return nullptr;
}
