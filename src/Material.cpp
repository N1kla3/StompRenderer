#include "Material.h"
#include "Logs.h"
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
    AddTextureInternal({static_cast<uint32_t>(type), texture});
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
    for (size_t i = 0; i < m_Textures.size(); i++)
    {
        auto& cached_texture_data = m_Textures[0];// TODO replace with i
        auto& cached_texture = cached_texture_data.Texture;

        // TODO: this will load for imgui too, not good
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = VK_NULL_HANDLE;
        image_info.sampler = VK_NULL_HANDLE;
        if (cached_texture)
        {
            cached_texture->GetTextureId();
            image_info.imageView = cached_texture->GetImageView();
            image_info.sampler = cached_texture->GetSampler();
        }

        m_DescriptorWriteSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //m_DescriptorSets[i].dstSet = m_DescriptorSets[i];
        m_DescriptorWriteSets[i].dstBinding = cached_texture_data.BindingIndex;
        m_DescriptorWriteSets[i].dstArrayElement = 0;
        m_DescriptorWriteSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        m_DescriptorWriteSets[i].descriptorCount = 1;
        m_DescriptorWriteSets[i].pImageInfo = &image_info;
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

void omp::Material::initialize()
{
    // TODO handle default texture if null
    m_AssetRepresentation = std::make_shared<MaterialRepresentation>(m_Textures[0].Texture);
}

omp::Material::Material(const std::string& name)
    : Asset()
{
    m_Name = name;
    m_Textures.resize(MAX_TEXTURES);
}
