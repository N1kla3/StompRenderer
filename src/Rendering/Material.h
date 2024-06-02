#pragma once

#include <vector>
#include <memory>
#include "Texture.h"
#include "TextureSrc.h"
#include "IO/SerializableObject.h"

namespace omp
{
    class MaterialManager;

    struct TextureData
    {
        uint32_t binding_index;
        std::shared_ptr<Texture> texture = nullptr;
        std::string name;
    };

    enum class ETextureType
    {
        Texture = 0,
        DiffusiveMap,
        SpecularMap,
        Max
    };

    struct MaterialRenderInfo
    {
        static constexpr int MAX_TEXTURES = 3;

        std::vector<TextureData> textures;

        std::string shader_name;

        MaterialRenderInfo()
            : shader_name("")
        {
            textures.resize(static_cast<size_t>(ETextureType::Max));
        }
    };

    class Material : public SerializableObject
    {
    private:
        std::unique_ptr<omp::MaterialRenderInfo> m_RenderInfo;

        std::vector<VkDescriptorSet> m_DescriptorSets;

        void addTextureInternal(TextureData&& data);

        bool m_EnableBlending = false;

    public:
        Material();
        explicit Material(const std::string& name);

        virtual void serialize(JsonParser<> &parser) override;
        virtual void deserialize(JsonParser<> &parser) override;

        void loadToGpu(const std::shared_ptr<omp::VulkanContext>& context);

        void addTexture(ETextureType type, const std::shared_ptr<Texture>& texture);
        void removeTexture(const TextureData& data);

        void addTexture(const std::shared_ptr<omp::TextureSrc>& texture);
        void addDiffusiveTexture(const std::shared_ptr<omp::TextureSrc>& texture);
        void addSpecularTexture(const std::shared_ptr<omp::TextureSrc>& texture);

        std::vector<TextureData> getTextureData() const;

        void setShaderName(const std::string& newName) { m_RenderInfo->shader_name = newName; };

        std::string getShaderName() const { return m_RenderInfo->shader_name; }

        const omp::MaterialRenderInfo* getRenderInfo() const { return m_RenderInfo.get(); }

        void setDescriptorSet(const std::vector<VkDescriptorSet>& ds);
        std::vector<VkDescriptorSet>& getDescriptorSet();
        void clearDescriptorSets() { m_DescriptorSets.clear(); }

        bool isPotentiallyReadyForRendering() { return !m_DescriptorSets.empty(); }

        void enableBlending(bool enable);
        bool isBlendingEnabled() const { return m_EnableBlending; }

        friend MaterialManager;
    };
} // omp
