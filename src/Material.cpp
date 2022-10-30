#include "Material.h"
#include "Logs.h"
#include "MaterialManager.h"
#include "UI/MaterialRepresentation.h"

void omp::Material::AddTextureInternal(TextureData&& Data)
{
    if (m_Textures.size() > MAX_TEXTURES)
    {
        WARN(Rendering, "Reached limit of textures for material");
    }

    m_IsDirty = true;
    m_IsInitialized = false;

    m_Textures[Data.BindingIndex-2] = std::move(Data);
}

void omp::Material::AddTexture(TextureType type, const std::shared_ptr<Texture>& texture)
{
    const static std::array<std::string, static_cast<size_t>(TextureType::MAX)> Names
            {"",
             "",
             "Texture",
             "Diffusive map",
             "Specular map"};

    AddTextureInternal({static_cast<uint32_t>(type), texture, Names[static_cast<uint32_t>(type)]});
}

void omp::Material::RemoveTexture(const TextureData &Data)
{
    m_IsDirty = true;
    m_IsInitialized = false;
}

std::vector<VkWriteDescriptorSet> omp::Material::GetDescriptorWriteSets()
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
        auto& cached_texture = cached_texture_data.Texture;
        if (!cached_texture || cached_texture_data.BindingIndex <= 1)
        {
            AddTexture(static_cast<TextureType>(i + static_cast<uint32_t>(TextureType::Texture)), m_Manager->GetEmptyTexture().lock());
        }
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = cached_texture->GetImageView();
        image_info.sampler = cached_texture->GetSampler();
        m_DescriptorWriteSets[i].dstBinding = cached_texture_data.BindingIndex;

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

void omp::Material::SetDescriptorSet(const std::vector<VkDescriptorSet>& DS)
{
    m_DescriptorSets = DS;

    m_IsInitialized = true;// TODO safety
}

std::vector<VkDescriptorSet>& omp::Material::GetDescriptorSet()
{
    return m_DescriptorSets;
}

std::vector<omp::TextureData> omp::Material::GetTextureData() const
{
    return m_Textures;
}

omp::Material::Material(const std::string& name)
{
    m_Textures.resize(MAX_TEXTURES);
}
