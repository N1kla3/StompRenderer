#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>
#include <memory>
#include "VulkanContext.h"

namespace omp
{
    class UniformBuffer
    {
    private:
        std::shared_ptr<omp::VulkanContext> m_VulkanContext;
        std::vector<VkBuffer> m_Buffer;
        std::vector<VkDeviceMemory> m_Memory;
        uint32_t m_KHRNum;

    public:
        UniformBuffer(const std::shared_ptr<omp::VulkanContext>& inVulkanContext, uint32_t khrImageCount, VkDeviceSize bufferSize, VkBufferUsageFlagBits flags);

        VkBuffer getBuffer(uint32_t khr) const { return m_Buffer.at(khr); }

        template<class T>
        void mapMemory(T& memory, uint32_t imageIndex, uint32_t offset = 0)
        {
            void* data;
            vkMapMemory(m_VulkanContext->logical_device, m_Memory[imageIndex], offset, sizeof(T), 0, &data);
            memcpy(data, &memory, sizeof(T));
            vkUnmapMemory(m_VulkanContext->logical_device, m_Memory[imageIndex]);
        }

        ~UniformBuffer();
    };
}
