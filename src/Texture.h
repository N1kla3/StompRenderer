#pragma once

#include <string>
#include "vulkan/vulkan.h"
#include "stb_image.h"
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
            LoadedToGPU = 1 << 1,
            LoadedToCPU = 1 << 2,
            LoadedToUI = 1 << 3
        };

        std::string m_ContentPath;
        stbi_uc* m_Pixels;
        int m_Size;
        int m_Width, m_Height;
        uint32_t m_MipLevels;

        uint16_t m_Flags = 0;

        // Vulkan //
        // ====== //
        VkImage m_TextureImage;
        VkDeviceMemory m_TextureImageMemory;
        VkImageView m_TextureImageView;
        VkSampler m_TextureSampler;

        VkDescriptorSet m_Id;

        std::weak_ptr<VulkanContext> m_VulkanContext;

    public:
        explicit Texture(const std::string& inPath);
        Texture(const std::shared_ptr<VulkanContext>& helper);

        void fullLoad(const std::string& path);
        void lazyLoad(const std::string& path);

        void destroyVkObjects();
        uint64_t getTextureId();
        VkImageView getImageView();
        VkImage getImage();
        VkSampler getSampler();

        std::string getPath() const { return m_ContentPath; }

    protected:
        // Subroutines //
        // =========== //
        void loadTextureToCpu(const std::string& path);
        void loadToGpu();
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