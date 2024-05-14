#include <cmath>
#include <stdexcept>
#include <memory>
#include "Texture.h"
#include "Logs.h"
#include "imgui_impl_vulkan.h"
#include "IO/stb_image.h"

omp::Texture::Texture(const std::shared_ptr<omp::TextureSrc>& inTexture)
    : m_TextureSource(inTexture)
{
}

omp::Texture::Texture(const std::shared_ptr<omp::TextureSrc>& inTexture, const std::shared_ptr<VulkanContext>& helper)
        : Texture(inTexture)
{
    m_VulkanContext = helper;
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

void omp::Texture::loadToGpu()
{
    if (hasFlags(LOADED_TO_GPU) || !hasVulkanContext())
    {
        VERROR(LogRendering, "Texture cant be loaded to GPU");
        return;
    }

    if (!m_TextureSource || !m_TextureSource->isLoaded())
    {
        VERROR(LogRendering, "Texture not loaded to CPU ");
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
    sampler_info.maxLod = static_cast<float>(m_TextureSource->getMipLevels());

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
    // TODO: Layer amount 
    size_t size_to_alloc = m_TextureSource->getSize();
    m_VulkanContext.lock()->createBuffer(size_to_alloc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                         staging_buffer, staging_buffer_memory);

    char* data;
    vkMapMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory, 0, size_to_alloc, 0, reinterpret_cast<void**>(&data));
    memcpy(data, m_TextureSource->getPixels(), size_to_alloc);
    vkUnmapMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory);

    VkImageCreateFlags flags = 0;
    uint32_t array_layers = 1;

    uint32_t size = static_cast<uint32_t>(m_TextureSource->getSize());
    uint32_t height = static_cast<uint32_t>(m_TextureSource->getHeight());
    uint32_t width = static_cast<uint32_t>(m_TextureSource->getWidth());
    uint32_t mip_levels = m_TextureSource->getMipLevels();
    m_VulkanContext.lock()->createImage(size,
                                        height,
                                        width, VK_FORMAT_R8G8B8A8_SRGB,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                        VK_IMAGE_USAGE_SAMPLED_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory,
                                        VK_SAMPLE_COUNT_1_BIT,
                                        flags, array_layers);

    m_VulkanContext.lock()->transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);

    m_VulkanContext.lock()->copyBufferToImage(staging_buffer,
                                              m_TextureImage,
                                              width,
                                              height);

    m_VulkanContext.lock()->generateMipmaps(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, static_cast<int32_t>(width), static_cast<int32_t>(height), mip_levels);

    vkDestroyBuffer(m_VulkanContext.lock()->logical_device, staging_buffer, nullptr);
    vkFreeMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory, nullptr);
}

void omp::Texture::createImageView()
{
    m_TextureImageView = m_VulkanContext.lock()->createImageView(
                m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_ASPECT_COLOR_BIT,
                m_TextureSource->getMipLevels());
}

void omp::Texture::fullLoad()
{
    if (hasFlags(LOADED_TO_GPU))
    {
        destroyVkObjects();
    }
    loadToGpu();
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
