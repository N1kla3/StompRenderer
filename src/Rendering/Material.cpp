#include "Material.h"
#include "Logs.h"
#include "MaterialManager.h"
#include "UI/MaterialRepresentation.h"

void omp::Material::addTextureInternal(TextureData&& data)
{
    if (m_RenderInfo->textures.size() > MaterialRenderInfo::MAX_TEXTURES)
    {
        WARN(Rendering, "Reached limit of textures for material");
    }

    clearDescriptorSets();
    m_RenderInfo->textures.push_back(std::move(data));
}

void omp::Material::addTexture(ETextureType type, const std::shared_ptr<Texture>& texture)
{
    const static std::array<std::string, static_cast<size_t>(ETextureType::Max)> names
            {
             "Texture",
             "Diffusive map",
             "Specular map"
            };

    clearDescriptorSets();

    addTextureInternal({static_cast<uint32_t>(type), texture, names[static_cast<uint32_t>(type)]});
}

void omp::Material::removeTexture(const TextureData& data)
{
    for (size_t index = 0; index < m_RenderInfo->textures.size(); index++)
    {
        if (m_RenderInfo->textures[index].binding_index == data.binding_index)
        {
            m_RenderInfo->textures.erase(m_RenderInfo->textures.cbegin() + index);
            break;
        }
    }
    clearDescriptorSets();
}

void omp::Material::setDescriptorSet(const std::vector<VkDescriptorSet>& ds)
{
    m_DescriptorSets = ds;
}

std::vector<VkDescriptorSet>& omp::Material::getDescriptorSet()
{
    return m_DescriptorSets;
}

std::vector<omp::TextureData> omp::Material::getTextureData() const
{
    return m_RenderInfo->textures;
}

omp::Material::Material()
{
    m_RenderInfo = std::make_unique<omp::MaterialRenderInfo>();
}

omp::Material::Material(const std::string& /*name*/)
    : Material()
{

}

void omp::Material::enableBlending(bool enable)
{
    m_EnableBlending = enable;
}
