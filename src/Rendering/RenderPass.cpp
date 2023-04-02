#include <stdexcept>
#include "RenderPass.h"

bool omp::RenderPass::startConfiguration()
{
    m_Attachments.clear();
    m_Dependencies.clear();
    m_Subpasses.clear();
    return true;
}

bool omp::RenderPass::endConfiguration()
{
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(m_Attachments.size());
    render_pass_info.pAttachments = m_Attachments.data();
    render_pass_info.subpassCount = static_cast<uint32_t>(m_Subpasses.size());
    render_pass_info.pSubpasses = m_Subpasses.data();
    render_pass_info.dependencyCount = static_cast<uint32_t>(m_Dependencies.size());
    render_pass_info.pDependencies = m_Dependencies.data();

    if (vkCreateRenderPass(m_LogicalDevice, &render_pass_info, nullptr, &m_VulkanRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
    m_IsCreated = true;
    return true;
}

omp::RenderPass::RenderPass(VkDevice inLogicalDevice)
    : m_LogicalDevice(inLogicalDevice)
{

}

void omp::RenderPass::addAttachment(VkAttachmentDescription&& attachment)
{
    m_Attachments.push_back(std::move(attachment));
}

void omp::RenderPass::addSubpass(VkSubpassDescription&& subpassInfo)
{
    m_Subpasses.push_back(std::move(subpassInfo));
}

void omp::RenderPass::addDependency(VkSubpassDependency&& dependency)
{
    m_Dependencies.push_back(std::move(dependency));
}

void omp::RenderPass::destroyInnerState()
{
    if (m_IsCreated)
    {
        vkDestroyRenderPass(m_LogicalDevice, m_VulkanRenderPass, nullptr);
        m_IsCreated = false;
    }
}

omp::RenderPass::~RenderPass()
{
    destroyInnerState();
}
