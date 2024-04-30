#pragma once
#include "IO/SerializableObject.h"
#include "Rendering/TextureSrc.h"
#include "IO/stb_image.h"
#include <memory>
#include "VulkanContext.h"

namespace omp
{
    /**
     * @brief Used to load images from PC, and upload directly to GPU
     */
    class Cubemap : public SerializableObject
    {
        enum
        {
            LOADED_TO_GPU = 1 << 1,
            LOADED_TO_UI = 1 << 3
        };

        std::vector<std::shared_ptr<omp::TextureSrc>> m_Textures;
        std::weak_ptr<VulkanContext> m_VulkanContext;
        // Vulkan //
        // ====== //
        VkImage m_TextureImage;
        VkDeviceMemory m_TextureImageMemory;
        VkImageView m_TextureImageView;
        VkSampler m_TextureSampler;

        VkDescriptorSet m_Id;

        uint16_t m_Flags = 0;
        size_t m_LayerAmount = 1;

    public:
        Cubemap() = default;
        explicit Cubemap(const std::vector<std::shared_ptr<omp::TextureSrc>>& inTextures);
        Cubemap(const std::vector<std::shared_ptr<omp::TextureSrc>>& inTextures, const std::shared_ptr<VulkanContext>& helper);

        // Serializable //
        // ============ //
        virtual void serialize(JsonParser<>& parser) override;
        virtual void deserialize(JsonParser<>& parser) override;

        void fullLoad();

        void specifyVulkanContext(const std::shared_ptr<VulkanContext>& inHelper);
        bool hasVulkanContext() const { return !m_VulkanContext.expired(); }
        void setTextures(const std::vector<std::shared_ptr<omp::TextureSrc>>& inTextures);

        void destroyVkObjects();
        VkDescriptorSet getTextureId();
        VkImageView getImageView();
        VkImage getImage();
        VkSampler getSampler();

    protected:
        // Subroutines //
        // =========== //
        void loadToGpu();
        void loadToUi();

        void createSampler();
        void createImage();
        void createImageView();

    private:
        bool isLoadedToMemory() const;
        size_t getFirstTextureSize() const;
        size_t getFirstTextureMipMap() const;
        size_t getFirstTextureWidth() const;
        size_t getFirstTextureHeight() const;
        void removeFlags(uint16_t flags);
        void addFlags(uint16_t flags);
        bool hasFlags(uint16_t flags) const;
    };
} // omp
