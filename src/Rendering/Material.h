#pragma once

#include <vector>
#include <memory>
#include "Texture.h"
#include "Asset.h"
#include "UI/MaterialPanel.h"
#include "glm/vec4.hpp"

namespace omp
{
    class MaterialManager;

    struct TextureData
    {
        uint32_t binding_index;
        std::shared_ptr<Texture> texture;
        std::string name;
    };

    struct MaterialRenderInfo
    {
        static constexpr int MAX_TEXTURES = 3;

        std::vector<TextureData> textures;

        std::string shader_name;
    };

    enum class ETextureType
    {
        Texture = 0,
        DiffusiveMap,
        SpecularMap,
        Max
    };

    class Material
    {
    private:
        std::unique_ptr<omp::MaterialRenderInfo> m_RenderInfo;

        omp::MaterialManager* m_Manager = nullptr;

        std::vector<VkDescriptorSet> m_DescriptorSets;

        void addTextureInternal(TextureData&& data);

    public:
        Material();
        explicit Material(const std::string& name);

        void addTexture(ETextureType type, const std::shared_ptr<Texture>& texture);
        void removeTexture(const TextureData& data);

        std::vector<TextureData> getTextureData() const;

        void setShaderName(const std::string& newName) { m_RenderInfo->shader_name = newName; };

        std::string getShaderName() const { return m_RenderInfo->shader_name; }

        const omp::MaterialRenderInfo* const getRenderInfo() const { return m_RenderInfo.get(); }

        void setDescriptorSet(const std::vector<VkDescriptorSet>& ds);
        std::vector<VkDescriptorSet>& getDescriptorSet();
        void clearDescriptorSets() { m_DescriptorSets.clear(); }

        bool isPotentiallyReadyForRendering() { return !m_DescriptorSets.empty(); }

        friend MaterialManager;
    };
} // omp
