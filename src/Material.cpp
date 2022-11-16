#include "Material.h"
#include "Logs.h"
#include "MaterialManager.h"
#include "UI/MaterialRepresentation.h"

void omp::Material::addTextureInternal(TextureData&& data)
{
    if (m_Textures.size() > MAX_TEXTURES)
    {
        WARN(Rendering, "Reached limit of textures for material");
    }

    m_IsDirty = true;
    m_IsInitialized = false;

    m_Textures[data.binding_index - 2] = std::move(data);
}

void omp::Material::addTexture(ETextureType type, const std::shared_ptr<Texture>& texture)
{
    const static std::array<std::string, static_cast<size_t>(ETextureType::Max)> names
            {"",
             "",
             "Texture",
             "Diffusive map",
             "Specular map"};

    addTextureInternal({static_cast<uint32_t>(type), texture, names[static_cast<uint32_t>(type)]});
}

void omp::Material::removeTexture(const TextureData& data)
{
    m_IsDirty = true;
    m_IsInitialized = false;
}

std::vector<VkWriteDescriptorSet> omp::Material::getDescriptorWriteSets()
{
    if (!m_IsDirty)
    {
        // return m_DescriptorWriteSets;
    }

    m_DescriptorWriteSets.resize(m_Textures.size());
    m_ImageInfosCache.resize(m_Textures.size());
    for (size_t i = 0; i < m_Textures.size(); i++)
    {
        auto& cached_texture_data = m_Textures[i];
        auto& cached_texture = cached_texture_data.texture;
        if (!cached_texture || cached_texture_data.binding_index <= 1)
        {
            addTexture(static_cast<ETextureType>(i + static_cast<uint32_t>(ETextureType::Texture)),
                       m_Manager->getEmptyTexture().lock());
        }
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = cached_texture->getImageView();
        image_info.sampler = cached_texture->getSampler();
        m_DescriptorWriteSets[i].dstBinding = cached_texture_data.binding_index;

        m_DescriptorWriteSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //m_DescriptorSets[i].dstSet = m_DescriptorSets[i];
        m_DescriptorWriteSets[i].dstArrayElement = 0;
        m_DescriptorWriteSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        m_DescriptorWriteSets[i].descriptorCount = 1;
        m_ImageInfosCache[i] = image_info;
        m_DescriptorWriteSets[i].pImageInfo = &m_ImageInfosCache[i];
    }
    m_IsDirty = false;
    return m_DescriptorWriteSets;
}

void omp::Material::setDescriptorSet(const std::vector<VkDescriptorSet>& ds)
{
    m_DescriptorSets = ds;

    m_IsInitialized = true;// TODO safety
}

std::vector<VkDescriptorSet>& omp::Material::getDescriptorSet()
{
    return m_DescriptorSets;
}

std::array<omp::TextureData, omp::Material::MAX_TEXTURES> omp::Material::getTextureData() const
{
    return m_Textures;
}

omp::Material::Material(const std::string& name)
{
}
