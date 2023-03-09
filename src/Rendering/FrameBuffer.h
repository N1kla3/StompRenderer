#pragma once
#include "vulkan/vulkan_core.h"
#include <vector>

namespace omp
{
    class FrameBuffer
    {
    public:
        FrameBuffer() = default;

    private:
        VkFramebuffer m_VulkanBuffer;
        std::vector<VkImageView> m_ImageAttachments;



    };
}
