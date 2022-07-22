#pragma once
#include "vulkan/vulkan.h"
#include <memory>


namespace omp{
class GraphicsPipeline
{
public:
    GraphicsPipeline(VkDevice inLogicalDevice);
    ~GraphicsPipeline();

    void StartCreation();
    void StartDefaultCreation();
    void CreateVertexInfo();
    void CreateInputAssembly();
    void CreateViewport();
    void CreateRasterizer();
    void CreateMultisamplingInfo(VkSampleCountFlagBits rasterizationSamples);
    void CreateColorBlending();
    void CreatePipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
    void CreateShaders(const std::shared_ptr<class Shader>& shader);
    void ConfirmCreation(VkRenderPass renderPass);

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


    void TryDestroyVulkanObjects();
};
}
