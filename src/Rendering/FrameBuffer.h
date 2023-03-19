#pragma once
#include "vulkan/vulkan_core.h"
#include "RenderPass.h"
#include <vector>
#include <memory>

namespace omp
{
    class FrameBuffer
    {
    public:
        FrameBuffer() = default;
        FrameBuffer(VkDevice device);
        FrameBuffer(VkDevice device, const std::vector<VkImageView>& imagesViews, const std::shared_ptr<omp::RenderPass>& renderPass, uint32_t width, uint32_t height);
        ~FrameBuffer();

        void destroyInnerState();

        void bindRenderPass(const std::shared_ptr<omp::RenderPass>& renderPass);
        void addImages(const std::vector<VkImageView>& imagesViews);
        void setSizes(uint32_t width, uint32_t height);

        void finishCreation();


        // Accessors //
        // ========= //
        VkFramebuffer getVulkanFrameBuffer() const { return m_VulkanBuffer; }


    private:
        // State //
        // ===== //
        bool m_ShouldDestroy = false;
        VkDevice m_LogicalDevice;
        VkFramebuffer m_VulkanBuffer;
        VkFramebufferCreateInfo m_CreateInfo{};
        std::vector<VkImageView> m_ImageAttachments;


    public:
        // Configuration //
        // ============= //
        int layers = 1;
    };
}
