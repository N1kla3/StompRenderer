#pragma once

#include "vulkan/vulkan.h"
#include "RenderPass.h"
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
        void createRasterizer(VkPipelineRasterizationStateCreateInfo inInfo);
        void createMultisamplingInfo(VkSampleCountFlagBits rasterizationSamples);
        void createColorBlending();
        void addColorBlendingAttachment(VkPipelineColorBlendAttachmentState state);
        template<typename T>
        void definePushConstant(VkShaderStageFlags stageFlags);
        void addPipelineSetLayout(VkDescriptorSetLayout descriptorSetLayout);
        void createShaders(const std::shared_ptr<class Shader>& shader);
        void setDepthStencil();
        void setDepthStencil(VkPipelineDepthStencilStateCreateInfo info);
        void confirmCreation(const std::shared_ptr<omp::RenderPass>& renderPass);

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
        std::vector<VkPipelineColorBlendAttachmentState> m_ColorBlendAttachments{};
        VkPipelineColorBlendStateCreateInfo m_ColorBlending{};
        VkPushConstantRange m_ConstantRange{};
        VkPipelineDepthStencilStateCreateInfo m_DepthStencil{};
        std::shared_ptr<omp::Shader> m_Shader;
        VkPipelineCache m_PipelineCache{};

        VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{};
        std::vector<VkDescriptorSetLayout> m_SetLayoutsHandles;

        VkPipelineLayout m_PipelineLayout;
        VkPipeline m_GraphicsPipeline;


        void tryDestroyVulkanObjects();
    };
}

template<typename T>
void omp::GraphicsPipeline::definePushConstant(VkShaderStageFlags stageFlags)
{
    // Push constant for model
    VkPushConstantRange constant_range{};
    constant_range.size = sizeof(T);
    constant_range.offset = 0;
    constant_range.stageFlags = stageFlags;// TODO: better with ranges to save space in shader
    m_ConstantRange = constant_range;

    m_PipelineLayoutInfo.pushConstantRangeCount = 1;
    m_PipelineLayoutInfo.pPushConstantRanges = &m_ConstantRange;
}
