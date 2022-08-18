#pragma once
#include <string>
#include "vulkan/vulkan.h"
#include "stb_image.h"
#include "VulkanContext.h"

namespace omp{
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
    VkDevice m_LogicalDevice;
    VkPhysicalDevice m_PhysDevice;

    VkImage m_TextureImage;
    VkDeviceMemory m_TextureImageMemory;
    VkImageView m_TextureImageView;
    VkSampler m_TextureSampler;

    VkDescriptorSet m_Id;

    std::weak_ptr<VulkanContext> m_VulkanContext;

public:
    Texture(VkDevice device, VkPhysicalDevice physDevice, const std::shared_ptr<VulkanContext> &helper);

    void FullLoad(const std::string& path);
    void LazyLoad(const std::string& path);

    void DestroyVkObjects();
    uint64_t GetTextureId();
    VkImageView GetImageView();
    VkImage GetImage();
    VkSampler GetSampler();

    std::string GetPath() const { return m_ContentPath; }

protected:
    // Subroutines //
    // =========== //
    void LoadTextureToCPU(const std::string& path);
    void LoadToGPU();
    void LoadToUI();

    void createSampler();
    void createImage();
    void createImageView();

private:
    void removeFlags(uint16_t flags);
    void addFlags(uint16_t flags);
    bool hasFlags(uint16_t flags) const;
};
} // omp