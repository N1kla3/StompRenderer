#include <filesystem>
#include <fstream>
#include "Asset.h"
#include "Logs.h"
#include "nlohmann/json.hpp"

void omp::Asset::saveAssetToFile(const std::string& inPath)
{
    auto file = std::filesystem::directory_entry(inPath);
    std::ofstream stream(file.path().string());
    if (stream.is_open())
    {
        nlohmann::json data;
        data["Name"] = m_Name;
        serializeData(data);
        stream << std::setw(4) << data << std::endl;
        INFO(AssetManager, "Asset saved successfully: {0}", inPath);
    }

    stream.close();
}

void omp::Asset::saveToLastValidPath()
{
    if (!m_Path.empty())
    {
        saveAssetToFile(m_Path);
    }
    else
    {
        VWARN(AssetManager, "Not valid file path");
    }
}
