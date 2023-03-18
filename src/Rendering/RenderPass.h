#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>

namespace omp
{

    class RenderPass
    {
    public:
        RenderPass(VkDevice inLogicalDevice);
        ~RenderPass();
        void destroyInnerState();

    private:
        // State //
        // ===== //
        bool m_IsCreated;

        VkDevice m_LogicalDevice;
        VkRenderPass m_VulkanRenderPass;
        std::vector<VkAttachmentDescription> m_Attachments;
        std::vector<VkSubpassDescription> m_Subpasses;
        std::vector<VkSubpassDependency> m_Dependencies;


        // Methods //
        // ======= //
    public:
        bool startConfiguration();
        void addAttachment(VkAttachmentDescription&& attachment);
        void addSubpass(VkSubpassDescription&& subpassInfo);
        void addDependency(VkSubpassDependency&& dependency);
        bool endConfiguration();

        VkRenderPass getRenderPass() const { return m_VulkanRenderPass; }
    };
}
