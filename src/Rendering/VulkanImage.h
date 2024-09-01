#pragma once
#include "vulkan/vulkan.h"
#include "VulkanContext.h"
#include <memory>

namespace omp
{
    class VulkanImage
    {
    public:
        VulkanImage() = delete;
        VulkanImage(const std::shared_ptr<omp::VulkanContext>& context);
        VulkanImage(const VulkanImage& image) = delete;
        VulkanImage(VulkanImage&& image) = delete;
        ~VulkanImage();

        void createImage(uint32_t width, uint32_t height, uint32_t, VkFormat format, VkImageTiling tiling,
                         VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSampleCountFlagBits numSamples,
                        VkImageCreateFlags flags, uint32_t arrayLayers);
        void createImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

        void createSampler(const VkSamplerCreateInfo& info);
        void createImguiImage();
        void recreateImage(uint32_t width, uint32_t height);
        void destroyAll();

    private:
        std::shared_ptr<omp::VulkanContext> m_Context;

        VkImageCreateInfo m_ImageInfo{};
        VkImageViewCreateInfo m_ViewInfo{};
        VkMemoryPropertyFlags m_MemoryFlags{};

        VkImage m_Image = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sample = VK_NULL_HANDLE;
        VkDeviceMemory m_Memory = VK_NULL_HANDLE;
        VkDescriptorSet m_ImguiImage = VK_NULL_HANDLE;
    public:
        const VkImage getImage() const { return m_Image; }
        const VkImageView getImageView() const { return m_ImageView; }
        const VkSampler getSampler() const { return m_Sample; }
        const VkDescriptorSet getImguiImage() const { return m_ImguiImage; }
    };
}

