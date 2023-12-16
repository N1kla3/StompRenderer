#include <cmath>
#include <stdexcept>
#include <memory>
#include "Texture.h"
#include "Logs.h"
#include "imgui_impl_vulkan.h"
#include "stb_image.h"

omp::Texture::Texture(const std::string& inPath)
{
    m_ContentPaths.reserve(1);
    m_ContentPaths.push_back(inPath);
    // assume 2d texture, because 1 path only
    m_Config.type = TextureConfig::TEXTURE2D;
    m_Config.layer_amount = 1;
    lazyLoad();
}

omp::Texture::Texture(const std::string& inPath, const std::shared_ptr<VulkanContext>& helper)
        : Texture(inPath)
{
    m_VulkanContext = helper;
}

omp::Texture::Texture(const std::shared_ptr<VulkanContext>& inContext, const std::vector<std::string>& inPaths, TextureConfig inConfig)
    : m_VulkanContext(inContext)
{
    m_Config = inConfig;
    m_ContentPaths = inPaths;
    lazyLoad();
}

void omp::Texture::destroyVkObjects()
{
    if (hasVulkanContext() && hasFlags(LOADED_TO_GPU))
    {
        vkDestroySampler(m_VulkanContext.lock()->logical_device, m_TextureSampler, nullptr);
        vkDestroyImageView(m_VulkanContext.lock()->logical_device, m_TextureImageView, nullptr);
        vkDestroyImage(m_VulkanContext.lock()->logical_device, m_TextureImage, nullptr);
        vkFreeMemory(m_VulkanContext.lock()->logical_device, m_TextureImageMemory, nullptr);
    }
}

VkDescriptorSet omp::Texture::getTextureId()
{
    if (!hasFlags(LOADED_TO_UI))
    {
        loadToUi();
    }
    return m_Id;
}

void omp::Texture::loadTextureToCpu()
{
    removeFlags(LOADED_TO_GPU | LOADED_TO_CPU | LOADED_TO_UI);

    // leak, should not be called twice
    m_Pixels.clear();

    if (m_ContentPaths.empty())
    {
        VWARN(Rendering, "Incorrect path to load texture");
        return;
    }

    int tex_channels;
    if (m_Config.type == TextureConfig::TEXTURE2D)
    {
        m_Pixels.push_back(stbi_load(m_ContentPaths[0].c_str(), &m_Width, &m_Height, &tex_channels, STBI_rgb_alpha));
    }
    else if (m_Config.type == TextureConfig::CUBEMAP)
    {
        for (const std::string& path : m_ContentPaths)
        {
            m_Pixels.reserve(m_Config.layer_amount);
            m_Pixels.push_back(stbi_load(path.c_str(), &m_Width, &m_Height, &tex_channels, STBI_rgb_alpha));
        }
    }

    // all sizes for cubemap should be identical
    m_Size = m_Width * m_Height * 4;
    m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Width, m_Height)))) + 1;

    if (!m_Pixels[0])
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    addFlags(LOADED_TO_CPU);
}

void omp::Texture::loadToGpu()
{
    if (hasFlags(LOADED_TO_GPU) || !hasVulkanContext())
    {
        VERROR(Rendering, "Texture cant be loaded to GPU");
        return;
    }

    if (!hasFlags(LOADED_TO_CPU))
    {
        VERROR(Rendering, "Texture not loaded to CPU {1} ", m_ContentPaths[0]);
        return;
    }

    createSampler();
    createImage();
    createImageView();

    addFlags(LOADED_TO_GPU);
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

    if (vkCreateSampler(m_VulkanContext.lock()->logical_device, &sampler_info, nullptr, &m_TextureSampler) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create sampler");
    }
}

void omp::Texture::createImage()
{
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    size_t size_to_alloc = m_Size * m_Config.layer_amount;
    m_VulkanContext.lock()->createBuffer(size_to_alloc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                         staging_buffer, staging_buffer_memory);

    char* data;
    vkMapMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory, 0, size_to_alloc, 0,
                reinterpret_cast<void**>(&data));
    size_t offset = 0;
    for (auto* pixels : m_Pixels)
    {
        memcpy(data + offset, pixels, static_cast<size_t>(m_Size));
        offset += m_Size;
    }
    vkUnmapMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory);

    for(auto* pix : m_Pixels) stbi_image_free(pix);

    VkImageCreateFlags flags = 0;
    uint32_t array_layers = 1;
    if (m_Config.type == TextureConfig::CUBEMAP)
    {
        flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        // faces of cubemap
        array_layers = m_Config.layer_amount;
    }
    m_VulkanContext.lock()->createImage(m_Width, m_Height, m_MipLevels, VK_FORMAT_R8G8B8A8_SRGB,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                        VK_IMAGE_USAGE_SAMPLED_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory,
                                        VK_SAMPLE_COUNT_1_BIT,
                                        flags, array_layers);

    m_VulkanContext.lock()->transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);

    if (m_Config.type == TextureConfig::CUBEMAP)
    {
        offset = 0;
        std::vector<VkBufferImageCopy> buffer_copy_regions{};
        buffer_copy_regions.reserve(m_Config.layer_amount);
        for (size_t layer = 0; layer < m_Config.layer_amount; layer++)
        {
            VkBufferImageCopy buffer_copy_region = {};
            buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            buffer_copy_region.imageSubresource.mipLevel = m_MipLevels;
            buffer_copy_region.imageSubresource.baseArrayLayer = layer;
            buffer_copy_region.imageSubresource.layerCount = 1;
            buffer_copy_region.imageExtent.width = m_Width;
            buffer_copy_region.imageExtent.height = m_Height;
            buffer_copy_region.imageExtent.depth = 1;
            buffer_copy_region.bufferOffset = offset;
            buffer_copy_regions.push_back(buffer_copy_region);
            offset += m_Size;
        }
        m_VulkanContext.lock()->copyBufferToImage(staging_buffer, m_TextureImage, buffer_copy_regions);
    }
    else
    {
        m_VulkanContext.lock()->copyBufferToImage(staging_buffer, m_TextureImage, static_cast<uint32_t>(m_Width),
                                                  static_cast<uint32_t>(m_Height));
    }
    //transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
    //                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
    m_VulkanContext.lock()->generateMipmaps(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, m_Width, m_Height, m_MipLevels);

    vkDestroyBuffer(m_VulkanContext.lock()->logical_device, staging_buffer, nullptr);
    vkFreeMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory, nullptr);
}

void omp::Texture::createImageView()
{
    m_TextureImageView = m_VulkanContext.lock()->createImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB,
                                                                 VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);
}

void omp::Texture::fullLoad()
{
    if (hasFlags(LOADED_TO_GPU))
    {
        destroyVkObjects();
    }
    loadTextureToCpu();
    loadToGpu();
}

void omp::Texture::lazyLoad()
{
    loadTextureToCpu();
}

void omp::Texture::removeFlags(uint16_t flags)
{
    m_Flags = (m_Flags & flags) ^ m_Flags;
}

void omp::Texture::addFlags(uint16_t flags)
{
    m_Flags = m_Flags | flags;
}

bool omp::Texture::hasFlags(uint16_t flags) const
{
    return m_Flags & flags;
}

void omp::Texture::loadToUi()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    m_Id = ImGui_ImplVulkan_AddTexture(m_TextureSampler, m_TextureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    addFlags(LOADED_TO_UI);
}

VkImageView omp::Texture::getImageView()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    return m_TextureImageView;
}

VkImage omp::Texture::getImage()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    return m_TextureImage;
}

VkSampler omp::Texture::getSampler()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    return m_TextureSampler;
}

void omp::Texture::specifyVulkanContext(const std::shared_ptr<VulkanContext>& inHelper)
{
    m_VulkanContext = inHelper;
}