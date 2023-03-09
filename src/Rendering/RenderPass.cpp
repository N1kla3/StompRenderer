#include "RenderPass.h"

bool omp::RenderPass::startConfiguration()
{
    return false;
}

bool omp::RenderPass::endConfiguration()
{
    return false;
}

bool omp::RenderPass::startRenderPass()
{
    return false;
}

bool omp::RenderPass::endRenderPass()
{
    return false;
}

omp::RenderPass::RenderPass(VkDevice inLogicalDevice)
    : m_LogicalDevice(inLogicalDevice)
{

}

void omp::RenderPass::addAttachment(VkAttachmentDescription description, VkAttachmentReference reference)
{
    reference.attachment = m_AttachmentCounter;
    m_Attachments.push_back({description, reference});
    m_AttachmentCounter++;
}

void omp::RenderPass::addAttachment(omp::RenderPassAttachment&& attachment)
{
    attachment.reference.attachment = m_AttachmentCounter;
    m_Attachments.push_back(std::move(attachment));
    m_AttachmentCounter++;
}
