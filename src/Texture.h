#pragma once
#include <string>
#include "vulkan/vulkan.h"
#include "stb_image.h"
#include "VulkanHelper.h"

namespace omp{
/**
 * @brief Used to load images from PC, and upload directly to GPU
 */
class Texture
{
    std::string m_ContentPath;
    stbi_uc* m_Pixels;
    int m_Size;
    int m_Width, m_Height;
    uint32_t m_MipLevels;

    bool m_LoadedToCPU;
    bool m_LoadedToGPU;

    // Vulkan //
    // ====== //
    VkDevice m_LogicalDevice;
    VkPhysicalDevice m_PhysDevice;

    VkImage m_TextureImage;
    VkDeviceMemory m_TextureImageMemory;
    VkImageView m_TextureImageView;
    VkSampler m_TextureSampler;

    VkDescriptorSet m_Id;

    std::weak_ptr<VulkanHelper> m_VkHelper;

public:
    Texture(VkDevice device, VkPhysicalDevice physDevice, const std::shared_ptr<VulkanHelper> &helper);

    void FullLoad(const std::string& path);
    void LazyLoad(const std::string& path);

    void Destroy();
    uint64_t GetTextureId() const;
    VkImageView GetImageView() const { return m_TextureImageView; }
    VkImage GetImage() const { return m_TextureImage; }
    VkSampler GetSampler() const { return m_TextureSampler; }

protected:
    // Subroutines //
    // =========== //
    void LoadTextureToCPU(const std::string& path);
    void LoadToGPU();

    void createSampler();
    void createImage();
    void createImageView();
};
} // omp