#pragma once

#include <string>
#include "vulkan/vulkan.h"
#include "IO/stb_image.h"
#include "TextureSrc.h"
#include <memory>
#include "VulkanContext.h"

namespace omp
{
    /**
     * @brief Used to load images from PC, and upload directly to GPU
     */
    class Texture
    {
        enum
        {
            LOADED_TO_GPU = 1 << 1,
            LOADED_TO_UI = 1 << 2
        };
        // Vulkan //
        // ====== //
        VkImage m_TextureImage;
        VkDeviceMemory m_TextureImageMemory;
        VkImageView m_TextureImageView;
        VkSampler m_TextureSampler;

        VkDescriptorSet m_Id;

        // Data //
        // ==== //
        std::shared_ptr<omp::TextureSrc> m_TextureSource;
        std::weak_ptr<VulkanContext> m_VulkanContext;
        uint16_t m_Flags = 0;

    public:
        Texture() = default;
        explicit Texture(const std::shared_ptr<omp::TextureSrc>& inTexture);
        Texture(const std::shared_ptr<omp::TextureSrc>& inTexture, const std::shared_ptr<VulkanContext>& helper);
        ~Texture();

        void fullLoad();

        void specifyVulkanContext(const std::shared_ptr<VulkanContext>& inHelper);
        bool hasVulkanContext() const { return !m_VulkanContext.expired(); }

        void destroyVkObjects();
        VkDescriptorSet getTextureId();
        VkImageView getImageView();
        VkImage getImage();
        VkSampler getSampler();
        TextureSrc* getTextureSrc() const { return m_TextureSource.get(); }

    protected:
        // Subroutines //
        // =========== //
        bool tryLoadToGpu();
        void loadToUi();

        void createSampler();
        void createImage();
        void createImageView();

    private:
        void removeFlags(uint16_t flags);
        void addFlags(uint16_t flags);
        bool hasFlags(uint16_t flags) const;
    };
} // omp
