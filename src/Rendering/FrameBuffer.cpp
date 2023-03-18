#include <stdexcept>
#include "FrameBuffer.h"

omp::FrameBuffer::FrameBuffer(VkDevice device)
    : m_LogicalDevice(device)
{

}

omp::FrameBuffer::FrameBuffer(
        VkDevice device,
        const std::vector<VkImageView>& imagesViews,
        const std::shared_ptr<omp::RenderPass>& renderPass,
        float width,
        float height)
        : m_LogicalDevice(device)
{
    VkFramebufferCreateInfo frame_buffer_info{};
    frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_info.renderPass = renderPass->getRenderPass();
    frame_buffer_info.attachmentCount = static_cast<uint32_t>(imagesViews.size());
    frame_buffer_info.pAttachments = imagesViews.data();
    frame_buffer_info.width = width;
    frame_buffer_info.height = height;
    frame_buffer_info.layers = layers;

    finishCreation();
}

omp::FrameBuffer::~FrameBuffer()
{
    destroyInnerState();
}

void omp::FrameBuffer::destroyInnerState()
{
    vkDestroyFramebuffer(m_LogicalDevice, m_VulkanBuffer, nullptr);

    for (auto image_view : m_ImageAttachments)
    {
        vkDestroyImageView(m_LogicalDevice, image_view, nullptr);
    }
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
    if (vkCreateFramebuffer(m_LogicalDevice, &m_CreateInfo, nullptr, &m_VulkanBuffer)
        != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}
