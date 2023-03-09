#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>

namespace omp
{
    struct RenderPassAttachment
    {
        VkAttachmentDescription description;
        VkAttachmentReference reference;
    };

    class RenderPass
    {
    public:
        RenderPass(VkDevice inLogicalDevice);

    private:
        // Cache //
        // ===== //
        int m_AttachmentCounter = 0;


        // State //
        // ===== //
        VkDevice m_LogicalDevice;
        std::vector<RenderPassAttachment> m_Attachments;


        // Methods //
        // ======= //
    public:
        bool startConfiguration();
        void addAttachment(VkAttachmentDescription description, VkAttachmentReference reference);
        void addAttachment(RenderPassAttachment&& attachment);
        bool endConfiguration();

        bool startRenderPass();
        bool endRenderPass();
    };
}
