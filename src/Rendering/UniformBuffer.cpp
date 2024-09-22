#include "UniformBuffer.h"

omp::UniformBuffer::UniformBuffer(const std::shared_ptr<omp::VulkanContext>& inVulkanContext, uint32_t khrImageCount, VkDeviceSize bufferSize, VkBufferUsageFlagBits flags)
    : m_VulkanContext(inVulkanContext)
    , m_KHRNum(khrImageCount)
{
    m_Buffer.resize(khrImageCount);
    m_Memory.resize(khrImageCount);
    for (size_t i = 0; i < khrImageCount; i++)
    {
        m_VulkanContext->createBuffer(bufferSize, flags,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_Buffer[i], m_Memory[i]);
    }
}
omp::UniformBuffer::UniformBuffer(UniformBuffer&& rhs)
{
    m_VulkanContext = std::move(rhs.m_VulkanContext);
    m_KHRNum = rhs.m_KHRNum;
    m_Buffer = std::move(rhs.m_Buffer);
    m_Memory = std::move(rhs.m_Memory);
}
omp::UniformBuffer& omp::UniformBuffer::operator=(UniformBuffer&& rhs)
{
    m_VulkanContext = std::move(rhs.m_VulkanContext);
    m_KHRNum = rhs.m_KHRNum;
    m_Buffer = std::move(rhs.m_Buffer);
    m_Memory = std::move(rhs.m_Memory);
    return *this;
}

omp::UniformBuffer::~UniformBuffer()
{
    for (size_t i = 0; i < m_Buffer.size(); i++)
    {
        vkDestroyBuffer(m_VulkanContext->logical_device, m_Buffer[i], nullptr);
    }
    for (size_t i = 0; i < m_Memory.size(); i++)
    {
        vkFreeMemory(m_VulkanContext->logical_device, m_Memory[i], nullptr);
    }
}
