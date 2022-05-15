#include "Asset.h"
#include "Logs.h"

void omp::Asset::saveAssetToFile(const std::string &path)
{
    WARN(UI, "Asset " + m_Name + " is not saveable");
}

void omp::Asset::loadAssetFromFile(const std::string &path)
{
    WARN(UI, "Asset " + m_Name + " is not loadable");
}
