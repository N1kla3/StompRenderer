#include "VulkanImage.h"
#include "Logs.h"
#include "imgui_impl_vulkan.h"
#include <vulkan/vulkan_core.h>

omp::VulkanImage::VulkanImage(const std::shared_ptr<omp::VulkanContext>& context)
    : m_Context(context)
{

}

omp::VulkanImage::~VulkanImage()
{
    destroyAll();
    if (m_Sample != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_Context->logical_device, m_Sample, nullptr);
        m_Sample = VK_NULL_HANDLE;
    }
}

void omp::VulkanImage::destroyAll()
{
    if (m_ImguiImage != VK_NULL_HANDLE)
    {
        ImGui_ImplVulkan_RemoveTexture(m_ImguiImage);
        m_ImguiImage = VK_NULL_HANDLE;
    }
    if (m_ImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_Context->logical_device, m_ImageView, nullptr);
        m_ImageView = VK_NULL_HANDLE;
    }
    if (m_Image != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_Context->logical_device, m_Image, nullptr);
        m_Image = VK_NULL_HANDLE;
    }
    if (m_Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_Context->logical_device, m_Memory, nullptr);
        m_Memory = VK_NULL_HANDLE;
    }
}

void omp::VulkanImage::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                 VkFormat format, VkImageTiling tiling,
                 VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                 VkSampleCountFlagBits numSamples,
                 VkImageCreateFlags flags, uint32_t arrayLayers)
{
    m_ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    m_ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    m_ImageInfo.extent.width = width;
    m_ImageInfo.extent.height = height;
    m_ImageInfo.extent.depth = 1;
    m_ImageInfo.mipLevels = mipLevels;
    m_ImageInfo.arrayLayers = arrayLayers;
    m_ImageInfo.format = format;
    m_ImageInfo.tiling = tiling;
    m_ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_ImageInfo.usage = usage;
    m_ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    m_ImageInfo.samples = numSamples;
    m_ImageInfo.flags = flags;

    m_MemoryFlags = properties;

    m_Context->createImage(m_ImageInfo, m_Image, m_Memory, m_MemoryFlags);
}

void omp::VulkanImage::createImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    m_ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    m_ViewInfo.image = m_Image;
    m_ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    m_ViewInfo.format = format;
    m_ViewInfo.subresourceRange.aspectMask = aspectFlags;
    m_ViewInfo.subresourceRange.baseMipLevel = 0;
    m_ViewInfo.subresourceRange.levelCount = mipLevels;
    m_ViewInfo.subresourceRange.baseArrayLayer = 0;
    m_ViewInfo.subresourceRange.layerCount = 1;

    m_ImageView = m_Context->createImageView(m_ViewInfo);
}

void omp::VulkanImage::createSampler(const VkSamplerCreateInfo& info)
{
    if (vkCreateSampler(m_Context->logical_device, &info, nullptr, &m_Sample) != VK_SUCCESS)
    {
        ERROR(LogRendering, "Cant create image sampler");
    }

}

void omp::VulkanImage::createImguiImage()
{
    m_ImguiImage = ImGui_ImplVulkan_AddTexture(m_Sample, m_ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void omp::VulkanImage::recreateImage(uint32_t width, uint32_t height)
{
    destroyAll();
    m_ImageInfo.extent.width = width;
    m_ImageInfo.extent.height = height;
    m_Context->createImage(m_ImageInfo, m_Image, m_Memory, m_MemoryFlags);
    m_ViewInfo.image = m_Image;
    m_ImageView = m_Context->createImageView(m_ViewInfo);
}

