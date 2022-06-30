#pragma once

#include <vector>
#include "vulkan/vulkan.h"

namespace omp{
/**
 * Class to help with vulkan routine functions
 */
class VulkanContext
{
    VkDevice m_LogicalDevice;
    VkPhysicalDevice m_PhysDevice;
    VkCommandPool m_CommandPools;
    VkQueue graphics_queue;
    // TODO replace occurences in Renderer.cpp
public:
    VulkanContext(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool pool, VkQueue graphicsQueue);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createImage(
            uint32_t width, uint32_t height, uint32_t mip_levels,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
            VkImage &image, VkDeviceMemory &imageMemory,
            VkSampleCountFlagBits numSamples
    );
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mip_levels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    bool hasStencilComponent(VkFormat format);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    friend class MaterialManager;
};
} // omp
