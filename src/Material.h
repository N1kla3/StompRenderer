#pragma once
#include <vector>
#include <memory>
#include "Texture.h"
#include "Asset.h"
#include "UI/MaterialPanel.h"
#include "glm/vec4.hpp"

namespace omp{
struct TextureData
{
    uint32_t BindingIndex;
    std::shared_ptr<Texture> Texture;
    std::string Name;
};

enum class TextureType
{
    Texture = 2,
    DiffusiveMap,
    SpecularMap,
    MAX
};

class Material
{
public:
    static constexpr int MAX_TEXTURES = 3;

private:
    omp::MaterialManager* m_Manager = nullptr;
    std::array<TextureData, MAX_TEXTURES> m_Textures;

    bool m_IsDirty = true;
    bool m_IsInitialized = false;

    std::string m_ShaderName;

    std::vector<VkWriteDescriptorSet> m_DescriptorWriteSets;

    std::vector<VkDescriptorSet> m_DescriptorSets;

    std::vector<VkDescriptorImageInfo> m_ImageInfosCache{};

    void addTextureInternal(TextureData&& Data);

public:
    explicit Material(const std::string& name);

    void addTexture(TextureType type, const std::shared_ptr<Texture>& texture);
    void removeTexture(const TextureData& Data);

    std::array<TextureData, MAX_TEXTURES> getTextureData() const;
    void setShaderName(const std::string& newName) { m_ShaderName = newName; };
    std::string getShaderName() const { return m_ShaderName; }

    void setDescriptorSet(const std::vector<VkDescriptorSet>& DS);
    std::vector<VkDescriptorSet>& getDescriptorSet();

    std::vector<VkWriteDescriptorSet> getDescriptorWriteSets();

    bool isInitialized() const noexcept { return m_IsInitialized; }

    friend MaterialManager;
};
} // omp
