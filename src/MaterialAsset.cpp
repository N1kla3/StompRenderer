#include "MaterialAsset.h"
#include "Material.h"

omp::MaterialAsset::MaterialAsset()
        : Asset()
{

}

void omp::MaterialAsset::initialize()
{
    std::shared_ptr<omp::Material> material = std::make_shared<omp::Material>(m_Name);
}

void omp::MaterialAsset::serializeData(nlohmann::json& data)
{
    write_ShaderName(data);
    write_TexturePaths(data);
}

void omp::MaterialAsset::deserializeData(const nlohmann::json& data)
{
    ShaderName = read_ShaderName(data);
    TexturePaths = read_TexturePaths(data);
}
