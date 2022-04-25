#include <cmath>
#include <stdexcept>
#include <memory>
#include "Texture.h"
#include "stb_image.h"

omp::Texture::Texture(VkDevice device, VkPhysicalDevice physDevice, const std::shared_ptr<VulkanHelper> &helper)
    : m_LogicalDevice(device)
    , m_PhysDevice(physDevice)
    , m_VkHelper(helper)
{

}

void omp::Texture::Destroy()
{
    vkDestroySampler(m_LogicalDevice, m_TextureSampler, nullptr);
    vkDestroyImageView(m_LogicalDevice, m_TextureImageView, nullptr);
    vkDestroyImage(m_LogicalDevice, m_TextureImage, nullptr);
    vkFreeMemory(m_LogicalDevice, m_TextureImageMemory, nullptr);
}

uint64_t omp::Texture::GetTextureId() const
{
    return 0;
}

void omp::Texture::LoadTextureToCPU(const std::string &path)
{
    m_LoadedToGPU = false;
    m_LoadedToCPU = false;

    m_ContentPath = path;
    int tex_channels;
    m_Pixels = stbi_load(path.c_str(), &m_Width, &m_Height, &tex_channels, STBI_rgb_alpha);
    m_Size = m_Width * m_Height * 4;

    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

    if (!m_Pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    m_LoadedToCPU = true;
}

void omp::Texture::LoadToGPU()
{
    createSampler();
    createImage();
    createImageView();

    m_LoadedToGPU = true;
}

void omp::Texture::createSampler()
{
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;

    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 16;

    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    sampler_info.unnormalizedCoordinates = VK_FALSE;

    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.f;
    sampler_info.minLod = 0;
    sampler_info.maxLod = static_cast<float>(m_MipLevels);

    if (vkCreateSampler(m_LogicalDevice, &sampler_info, nullptr, &m_TextureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create sampler");
    }
}

void omp::Texture::createImage()
{
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    m_VkHelper.lock()->createBuffer(m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    staging_buffer, staging_buffer_memory);

    void* data;
    vkMapMemory(m_LogicalDevice, staging_buffer_memory, 0, m_Size, 0, &data);
    memcpy(data, m_Pixels, static_cast<size_t>(m_Size));
    vkUnmapMemory(m_LogicalDevice, staging_buffer_memory);

    stbi_image_free(m_Pixels);

    m_VkHelper.lock()->createImage(m_Width, m_Height, m_MipLevels, VK_FORMAT_R8G8B8A8_SRGB,
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory, VK_SAMPLE_COUNT_1_BIT);

    m_VkHelper.lock()->transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
    m_VkHelper.lock()->copyBufferToImage(staging_buffer, m_TextureImage, static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height));
    //transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
    //                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
    m_VkHelper.lock()->generateMipmaps(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, m_Width, m_Height, m_MipLevels);

    vkDestroyBuffer(m_LogicalDevice, staging_buffer, nullptr);
    vkFreeMemory(m_LogicalDevice, staging_buffer_memory, nullptr);
}

void omp::Texture::createImageView()
{
    m_TextureImageView = m_VkHelper.lock()->createImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);
}

