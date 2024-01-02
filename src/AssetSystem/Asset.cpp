#include <filesystem>
#include <fstream>
#include "AssetSystem/Asset.h"
#include "Logs.h"
#include "AssetSystem/AssetLoader.h"
#include "nlohmann/json.hpp"

void omp::Asset::saveAssetToFile(const std::string& inPath, const std::string& inClassName)
{
    m_Path = inPath;
    m_ClassName = inClassName;
    auto file = std::filesystem::directory_entry(inPath);
    std::ofstream stream(file.path().string());
    if (stream.is_open())
    {
        nlohmann::json data;
        data[NAME_MEMBER] = m_Name;
        data[CLASS_MEMBER] = inClassName;
        serializeData(data);
        stream << std::setw(4) << data << std::endl;
        INFO(AssetManager, "Asset saved successfully: {0}", inPath);
    }

    stream.close();
}

bool omp::Asset::saveToLastValidPath()
{
    if (!m_Path.empty())
    {
        saveAssetToFile(m_Path, m_ClassName);
        // TODO add check for existent class name
        return true;
    }
    else
    {
        VWARN(AssetManager, "Not valid file path");
        return false;
    }
}
