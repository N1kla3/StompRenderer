#pragma once
#include "vulkan/vulkan.h"
#include <memory>


namespace omp{
class GraphicsPipeline
{
public:
    GraphicsPipeline() = default;
    ~GraphicsPipeline();

    void StartCreation();
    void StartDefaultCreation();
    void CreateVertexInfo();
    void CreateInputAssembly();
    void CreateViewport();
    void CreateRasterizer();
    void CreateMultisamplingInfo();
    void CreateColorBlending();
    void CreatePipelineLayout();
    void CreateShaders(const std::shared_ptr<class Shader>& shader);
    void ConfirmCreation();

private:
    bool m_IsCreated = false;

    // PIPELINE //
    // ======== //
    VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
    VkViewport m_Viewport{};
    VkRect2D m_Scissor{};
    VkPipelineViewportStateCreateInfo m_ViewportState{};
    VkPipelineRasterizationStateCreateInfo m_Rasterizer{};
    VkPipelineMultisampleStateCreateInfo m_Multisampling{};
    VkPipelineColorBlendStateCreateInfo m_ColorBlending{};
    VkPushConstantRange m_ConstantRange{};
    VkPipelineDepthStencilStateCreateInfo m_DepthStencil{};

    VkPipelineLayoutCreateInfo m_PipelineLayoutInfo{};

    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_GraphicsPipeline;
};
}
