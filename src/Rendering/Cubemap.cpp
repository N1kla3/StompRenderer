#include "Cubemap.h"
#include <memory>
#include "Logs.h"
#include "imgui_impl_vulkan.h"
#include "IO/stb_image.h"

omp::Cubemap::Cubemap(const std::vector<std::shared_ptr<omp::TextureSrc>>& inTextures)
    : m_Textures(inTextures)
{
    m_LayerAmount = m_Textures.size();
}

omp::Cubemap::Cubemap(const std::vector<std::shared_ptr<omp::TextureSrc>>& inTextures, const std::shared_ptr<VulkanContext>& helper)
    : m_Textures(inTextures)
    , m_VulkanContext(helper)
{
    m_LayerAmount = m_Textures.size();
}

void omp::Cubemap::destroyVkObjects()
{
    if (hasVulkanContext() && hasFlags(LOADED_TO_GPU))
    {
        vkDestroySampler(m_VulkanContext.lock()->logical_device, m_TextureSampler, nullptr);
        vkDestroyImageView(m_VulkanContext.lock()->logical_device, m_TextureImageView, nullptr);
        vkDestroyImage(m_VulkanContext.lock()->logical_device, m_TextureImage, nullptr);
        vkFreeMemory(m_VulkanContext.lock()->logical_device, m_TextureImageMemory, nullptr);
    }
}

VkDescriptorSet omp::Cubemap::getTextureId()
{
    if (!hasFlags(LOADED_TO_UI))
    {
        loadToUi();
    }
    return m_Id;
}

void omp::Cubemap::loadToGpu()
{
    if (hasFlags(LOADED_TO_GPU) || !hasVulkanContext())
    {
        VERROR(LogRendering, "Cubemap cant be loaded to GPU");
        return;
    }

    if (isLoadedToMemory())
    {
        VERROR(LogRendering, "Cubemap not loaded to Memory ");
        return;
    }

    createSampler();
    createImage();
    createImageView();

    addFlags(LOADED_TO_GPU);
}

void omp::Cubemap::createSampler()
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
    sampler_info.maxLod = static_cast<float>(getFirstTextureMipMap());

    if (vkCreateSampler(m_VulkanContext.lock()->logical_device, &sampler_info, nullptr, &m_TextureSampler) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create sampler");
    }
}

void omp::Cubemap::createImage()
{
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    size_t size = getFirstTextureSize();
    size_t mip_level = getFirstTextureMipMap();
    size_t width = getFirstTextureWidth();
    size_t height = getFirstTextureHeight();

    size_t size_to_alloc = getFirstTextureSize() * m_LayerAmount;
    m_VulkanContext.lock()->createBuffer(size_to_alloc, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                         staging_buffer, staging_buffer_memory);

    char* data;
    vkMapMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory, 0, size_to_alloc, 0,
                reinterpret_cast<void**>(&data));
    size_t offset = 0;
    for (auto& texture : m_Textures)
    {
        memcpy(data + offset, texture->getPixels(), size);
        offset += size;
    }
    vkUnmapMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory);

    VkImageCreateFlags flags = 0;
    uint32_t array_layers = 1;

    flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    // faces of cubemap
    array_layers = m_LayerAmount;

    m_VulkanContext.lock()->createImage(width, height, mip_level, VK_FORMAT_R8G8B8A8_SRGB,
                                        VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                        VK_IMAGE_USAGE_SAMPLED_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory,
                                        VK_SAMPLE_COUNT_1_BIT,
                                        flags, array_layers);

    m_VulkanContext.lock()->transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_level);

    offset = 0;
    std::vector<VkBufferImageCopy> buffer_copy_regions{};
    buffer_copy_regions.reserve(m_LayerAmount);
    for (size_t layer = 0; layer < m_LayerAmount; layer++)
    {
        VkBufferImageCopy buffer_copy_region = {};
        buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_copy_region.imageSubresource.mipLevel = mip_level;
        buffer_copy_region.imageSubresource.baseArrayLayer = layer;
        buffer_copy_region.imageSubresource.layerCount = 1;
        buffer_copy_region.imageExtent.width = width;
        buffer_copy_region.imageExtent.height = height;
        buffer_copy_region.imageExtent.depth = 1;
        buffer_copy_region.bufferOffset = offset;
        buffer_copy_regions.push_back(buffer_copy_region);
        offset += size;
    }
    m_VulkanContext.lock()->copyBufferToImage(staging_buffer, m_TextureImage, buffer_copy_regions);

    //transitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
    //                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
    m_VulkanContext.lock()->generateMipmaps(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, mip_level);

    vkDestroyBuffer(m_VulkanContext.lock()->logical_device, staging_buffer, nullptr);
    vkFreeMemory(m_VulkanContext.lock()->logical_device, staging_buffer_memory, nullptr);
}

void omp::Cubemap::createImageView()
{
    m_TextureImageView = m_VulkanContext.lock()->createImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB,
                                                                 VK_IMAGE_ASPECT_COLOR_BIT, getFirstTextureMipMap());
}

void omp::Cubemap::serialize(JsonParser<>& parser)
{
    using id_type = omp::SerializableObject::SerializationId;

    std::vector<id_type> textures_ids;
    textures_ids.reserve(m_Textures.size());

    for (auto& texture : m_Textures)
    {
        textures_ids.push_back(serializeDependency(texture.get()));
    }
    parser.writeValue("textures", textures_ids);
}

void omp::Cubemap::deserialize(JsonParser<>& parser)
{
    using id_type = omp::SerializableObject::SerializationId;

    std::vector<omp::SerializableObject::SerializationId> textures_ids;
    textures_ids = parser.readValue<std::vector<id_type>>("textures").value();

    for (id_type id : textures_ids)
    {
        m_Textures.push_back(std::dynamic_pointer_cast<omp::TextureSrc>(getDependency(id)));
    }
}

void omp::Cubemap::fullLoad()
{
    if (hasFlags(LOADED_TO_GPU))
    {
        destroyVkObjects();
    }
    loadToGpu();
}

bool omp::Cubemap::isLoadedToMemory() const
{
    if (!m_Textures.empty())
    {
        for (auto& texture : m_Textures)
        {
            if (!texture->isLoaded())
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

size_t omp::Cubemap::getFirstTextureSize() const
{
    if (!m_Textures.empty())
    {
        return m_Textures[0]->getSize();
    }
    return 0;
}

size_t omp::Cubemap::getFirstTextureMipMap() const
{
    if (!m_Textures.empty())
    {
        return m_Textures[0]->getMipLevels();
    }
    return 0;
}

size_t omp::Cubemap::getFirstTextureWidth() const
{
    if (!m_Textures.empty())
    {
        return m_Textures[0]->getWidth();
    }
    return 0;
}

size_t omp::Cubemap::getFirstTextureHeight() const
{
    if (!m_Textures.empty())
    {
        return m_Textures[0]->getHeight();
    }
    return 0;
}

void omp::Cubemap::removeFlags(uint16_t flags)
{
    m_Flags = (m_Flags & flags) ^ m_Flags;
}

void omp::Cubemap::addFlags(uint16_t flags)
{
    m_Flags = m_Flags | flags;
}

bool omp::Cubemap::hasFlags(uint16_t flags) const
{
    return m_Flags & flags;
}

void omp::Cubemap::loadToUi()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    m_Id = ImGui_ImplVulkan_AddTexture(m_TextureSampler, m_TextureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    addFlags(LOADED_TO_UI);
}

VkImageView omp::Cubemap::getImageView()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    return m_TextureImageView;
}

VkImage omp::Cubemap::getImage()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    return m_TextureImage;
}

VkSampler omp::Cubemap::getSampler()
{
    if (!hasFlags(LOADED_TO_GPU))
    {
        loadToGpu();
    }
    return m_TextureSampler;
}

void omp::Cubemap::specifyVulkanContext(const std::shared_ptr<VulkanContext>& inHelper)
{
    m_VulkanContext = inHelper;
}
void omp::Cubemap::setTextures(const std::vector<std::shared_ptr<omp::TextureSrc>>& inTextures)
{
    m_Textures = inTextures;
    m_LayerAmount = m_Textures.size();
}

