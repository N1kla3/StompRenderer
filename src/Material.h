#pragma once
#include <vector>
#include <memory>
#include "Texture.h"
#include "Asset.h"
#include "glm/vec3.hpp"

struct TextureData
{
    uint32_t BindingIndex;
    std::shared_ptr<omp::Texture> Texture;
};

namespace omp{
class Material : public Asset
{
    std::vector<TextureData> m_Textures;

    glm::vec3 m_Ambient;
    glm::vec3 m_Diffusive;
    glm::vec3 m_Specular;

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

    glm::vec3 GetAmbient() const { return m_Ambient; }
    glm::vec3 GetDiffusive() const { return m_Diffusive; }
    glm::vec3 GetSpecular() const { return m_Specular; }
};
} // omp
