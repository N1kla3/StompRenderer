#include <stdexcept>
#include "Logs.h"
#include "FrameBuffer.h"

omp::FrameBuffer::FrameBuffer(VkDevice device)
    : m_LogicalDevice(device)
{

}

omp::FrameBuffer::FrameBuffer(
        VkDevice device,
        const std::vector<VkImageView>& imagesViews,
        const std::shared_ptr<omp::RenderPass>& renderPass,
        uint32_t width,
        uint32_t height)
        : m_LogicalDevice(device)
{
    m_CreateInfo.renderPass = renderPass->getRenderPass();
    m_CreateInfo.attachmentCount = static_cast<uint32_t>(imagesViews.size());
    m_CreateInfo.pAttachments = imagesViews.data();
    m_CreateInfo.width = width;
    m_CreateInfo.height = height;
    m_CreateInfo.layers = layers;

    finishCreation();
}

omp::FrameBuffer::~FrameBuffer()
{
    //destroyInnerState();
}

void omp::FrameBuffer::destroyInnerState()
{
    if (!m_ShouldDestroy)
    {
        return;
    }
    vkDestroyFramebuffer(m_LogicalDevice, m_VulkanBuffer, nullptr);
    m_ShouldDestroy = false;
}

void omp::FrameBuffer::addImages(const std::vector<VkImageView>& imagesViews)
{
    m_ImageAttachments = imagesViews;
    m_CreateInfo.attachmentCount = static_cast<uint32_t>(imagesViews.size());
    m_CreateInfo.pAttachments = imagesViews.data();
}

void omp::FrameBuffer::setSizes(uint32_t width, uint32_t height)
{
    m_CreateInfo.width = width;
    m_CreateInfo.height = height;
}

void omp::FrameBuffer::finishCreation()
{
    //INFO(LogRendering, "DEBUG: framebuffer size is: {}{}", m_CreateInfo.height, m_CreateInfo.width);
    m_CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    if (vkCreateFramebuffer(m_LogicalDevice, &m_CreateInfo, nullptr, &m_VulkanBuffer)
        != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
    m_ShouldDestroy = true;
}

void omp::FrameBuffer::bindRenderPass(const std::shared_ptr<omp::RenderPass>& renderPass)
{
    m_CreateInfo.renderPass = renderPass->getRenderPass();
}
