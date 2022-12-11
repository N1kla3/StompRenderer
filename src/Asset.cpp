#include <filesystem>
#include <fstream>
#include "Asset.h"
#include "Logs.h"
#include "AssetLoader.h"
#include "nlohmann/json.hpp"

void omp::Asset::saveAssetToFile(const std::string& inPath)
{
    m_Path = inPath;
    auto file = std::filesystem::directory_entry(inPath);
    std::ofstream stream(file.path().string());
    if (stream.is_open())
    {
        nlohmann::json data;
        data[NAME_MEMBER] = m_Name;
        data[CLASS_MEMBER] = omp::AssetLoader::getClassString(this);
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
        saveAssetToFile(m_Path);
        return true;
    }
    else
    {
        VWARN(AssetManager, "Not valid file path");
        return false;
    }
}
