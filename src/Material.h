#pragma once
#include <vector>
#include <memory>
#include "Texture.h"
#include "Asset.h"

struct TextureData
{
    uint32_t BindingIndex;
    std::shared_ptr<omp::Texture> Texture;
};

namespace omp{
class Material : public Asset
{
    std::vector<TextureData> m_Textures;

    bool m_IsDirty = true;
    bool m_IsInitialized = false;

    std::vector<VkWriteDescriptorSet> m_DescriptorWriteSets;

    std::vector<VkDescriptorSet> m_DescriptorSets;

    VkDescriptorImageInfo image_info{};
public:
    void AddTexture(const TextureData& Data);
    void RemoveTexture(const TextureData& Data);

    std::vector<TextureData> GetTextureData() const;

    void SetDescriptorSet(const std::vector<VkDescriptorSet>& DS);
    std::vector<VkDescriptorSet>& GetDescriptorSet();

    std::vector<VkWriteDescriptorSet> GetDescriptorWriteSets();

    bool IsInitialized() const noexcept { return m_IsInitialized; }

    virtual void initialize() override;

    static constexpr int MAX_TEXTURES = 1;
};
} // omp
