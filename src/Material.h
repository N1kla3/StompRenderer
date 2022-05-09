#pragma once
#include <vector>
#include <memory>
#include "Texture.h"

struct TextureData
{
    uint32_t BindingIndex;
    std::shared_ptr<omp::Texture> Texture;
};

namespace omp{
class Material
{
    std::vector<TextureData> m_Textures;

    bool m_IsDirty = true;

    std::vector<VkWriteDescriptorSet> m_DescriptorSets;

public:
    void AddTexture(const TextureData& Data);
    void RemoveTexture(const TextureData& Data);

    std::vector<VkWriteDescriptorSet> GetDescriptorSets();
};
} // omp
