#pragma once
#include <vector>
#include <memory>
#include "Texture.h"
#include "Asset.h"
#include "UI/MaterialPanel.h"
#include "glm/vec4.hpp"

struct TextureData
{
    uint32_t BindingIndex;
    std::shared_ptr<omp::Texture> Texture;
};

namespace omp{
class Material : public Asset
{
    std::vector<TextureData> m_Textures;

    glm::vec4 m_Ambient = {1.0f, 1.0f, 1.0f, 0};
    glm::vec4 m_Diffusive = {1.0f, 1.0f, 1.0f, 0};
    glm::vec4 m_Specular = {1.0f, 1.0f, 1.0f, 0};

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

    glm::vec4 GetAmbient() const { return m_Ambient; }
    glm::vec4 GetDiffusive() const { return m_Diffusive; }
    glm::vec4 GetSpecular() const { return m_Specular; }

    friend class MaterialPanel;
};
} // omp
