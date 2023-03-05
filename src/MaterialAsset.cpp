#include "MaterialAsset.h"
#include "MaterialManager.h"
#include "Rendering/Material.h"

omp::MaterialAsset::MaterialAsset()
        : Asset()
{

}

void omp::MaterialAsset::initialize()
{
    std::shared_ptr<omp::Material>&& material = omp::MaterialManager::getMaterialManager().createOrGetMaterial(m_Name);
    int index = 0;
    for (auto& texture_path : TexturePaths)
    {
        auto&& texture = omp::MaterialManager::getMaterialManager().loadTextureLazily(texture_path);
        material->addTexture(static_cast<ETextureType>(index + static_cast<uint32_t>(ETextureType::Texture)), std::move(texture));
        index++;
    }
    material->setShaderName(ShaderName);

    m_Material = std::move(material);
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
