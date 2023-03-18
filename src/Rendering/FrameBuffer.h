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
        FrameBuffer(VkDevice device, const std::vector<VkImageView>& imagesViews, const std::shared_ptr<omp::RenderPass>& renderPass, float width, float height);
        ~FrameBuffer();

        void destroyInnerState();

        void addImages(const std::vector<VkImageView>& imagesViews);
        void setSizes(uint32_t width, uint32_t height);

        void finishCreation();


    private:
        // State //
        // ===== //
        VkDevice m_LogicalDevice;
        VkFramebuffer m_VulkanBuffer;
        VkFramebufferCreateInfo m_CreateInfo;
        std::vector<VkImageView> m_ImageAttachments;


    public:
        // Configuration //
        // ============= //
        int layers = 1;
    };
}
