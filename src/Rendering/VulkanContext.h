#pragma once

#include <vector>
#include "vulkan/vulkan.h"

namespace omp
{
/**
 * Class to help with vulkan routine functions
 */
    class VulkanContext
    {
    public:
        VkDevice logical_device;
        VkPhysicalDevice phys_device;
        VkCommandPool command_pools;
        VkQueue graphics_queue;
        // TODO replace occurences in Renderer.cpp
        VulkanContext(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool pool, VkQueue graphicsQueue);

        void createBuffer(
                VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                VkDeviceMemory& bufferMemory);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void createImage(
                uint32_t width, uint32_t height, uint32_t mipLevels,
                VkFormat format, VkImageTiling tiling,
                VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                VkImage& image, VkDeviceMemory& imageMemory,
                VkSampleCountFlagBits numSamples,
                VkImageCreateFlags flags = 0,
                uint32_t arrayLayers = 1
        );
        void createImage(const VkImageCreateInfo& imageInfo, VkImage& image, VkDeviceMemory& imageMemory,VkMemoryPropertyFlags properties);
        void transitionImageLayout(
                VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void copyBufferToImage(VkBuffer buffer, VkImage image, const std::vector<VkBufferImageCopy>& regions);
        void generateMipmaps(
                VkImage image,
                VkFormat imageFormat,
                int32_t texWidth,
                int32_t texHeight,
                uint32_t mipLevels);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        bool hasStencilComponent(VkFormat format);
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        VkImageView createImageView(const VkImageViewCreateInfo& info);

        VkShaderModule createShaderModule(const std::vector<char>& code);
        void destroyShaderModule(VkShaderModule inModule);

        void setCommandPool(VkCommandPool pool) { command_pools = pool; }

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        friend class MaterialManager;
    };
} // omp
