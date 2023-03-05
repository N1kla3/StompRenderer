#pragma once

#include "vulkan/vulkan.h"
#include <memory>


namespace omp
{
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(VkDevice inLogicalDevice);
        ~GraphicsPipeline();

        void startCreation();
        void startDefaultCreation();
        void createVertexInfo();
        void createInputAssembly();
        void createViewport(VkExtent2D scissorExtent);
        void createRasterizer();
        void createMultisamplingInfo(VkSampleCountFlagBits rasterizationSamples);
        void createColorBlending();
        void createPipelineLayout(VkDescriptorSetLayout& descriptorSetLayout);
        void createShaders(const std::shared_ptr<class Shader>& shader);
        void confirmCreation(VkRenderPass renderPass);

        VkPipeline getGraphicsPipeline() { return m_GraphicsPipeline; }

        VkPipelineLayout getPipelineLayout() { return m_PipelineLayout; }

    private:
        bool m_IsCreated = false;
        VkDevice m_LogicalDevice;

        // PIPELINE //
        // ======== //
        VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
        VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
        VkViewport m_Viewport{};
        VkRect2D m_Scissor{};
        VkPipelineViewportStateCreateInfo m_ViewportState{};
        VkPipelineRasterizationStateCreateInfo m_Rasterizer{};
        VkPipelineMultisampleStateCreateInfo m_Multisampling{};
        VkPipelineColorBlendAttachmentState m_ColorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo m_ColorBlending{};
        VkPushConstantRange m_ConstantRange{};
        VkPipelineDepthStencilStateCreateInfo m_DepthStencil{};
        std::shared_ptr<omp::Shader> m_Shader;

        VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{};

        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;


        void tryDestroyVulkanObjects();
    };
}
