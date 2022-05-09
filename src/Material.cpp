#include "Material.h"

void omp::Material::AddTexture(const TextureData &Data)
{
    m_IsDirty = true;
}

void omp::Material::RemoveTexture(const TextureData &Data)
{
    m_IsDirty = true;
}

std::vector<VkWriteDescriptorSet> omp::Material::GetDescriptorSets()
{
    if (!m_IsDirty)
    {
        return m_DescriptorSets;
    }

    for (size_t i = 0; i < m_Textures.size(); i++)
    {
        m_DescriptorSets.resize(m_Textures.size());

        auto& cached_texture_data = m_Textures[i];
        auto& cached_texture = cached_texture_data.Texture;

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = cached_texture->GetImageView();
        image_info.sampler = cached_texture->GetSampler();

        m_DescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //m_DescriptorSets[i].dstSet = m_DescriptorSets[i];
        m_DescriptorSets[i].dstBinding = cached_texture_data.BindingIndex + 1;
        m_DescriptorSets[i].dstArrayElement = 0;
        m_DescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        m_DescriptorSets[i].descriptorCount = 1;
        m_DescriptorSets[i].pImageInfo = &image_info;
    }
    m_IsDirty = false;
    return m_DescriptorSets;
}
