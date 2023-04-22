#include <stdexcept>
#include "GraphicsPipeline.h"
#include "Model.h"
#include "Shader.h"

omp::GraphicsPipeline::GraphicsPipeline(VkDevice inLogicalDevice)
        : m_LogicalDevice(inLogicalDevice)
{

}

omp::GraphicsPipeline::~GraphicsPipeline()
{
    tryDestroyVulkanObjects();
}

void omp::GraphicsPipeline::startCreation()
{
    tryDestroyVulkanObjects();
    m_IsCreated = false;

}

void omp::GraphicsPipeline::startDefaultCreation()
{
    tryDestroyVulkanObjects();
    m_IsCreated = false;

    createVertexInfo();
    createInputAssembly();
    createViewport(VkExtent2D());
    createRasterizer();
    createColorBlending();
}

void omp::GraphicsPipeline::createVertexInfo()
{
    static auto binding_description = omp::Vertex::GetBindingDescription();
    static auto attribute_descriptions = omp::Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    m_VertexInputInfo = vertex_input_info;
}

void omp::GraphicsPipeline::createInputAssembly()
{
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    m_InputAssembly = input_assembly;
}

void omp::GraphicsPipeline::createViewport(VkExtent2D scissorExtent)
{
    // Viewport and scissors
    // NOT USED, using dynamic viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 0;
    viewport.height = 0;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    m_Viewport = viewport;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = scissorExtent;//m_SwapChainExtent;
    m_Scissor = scissor;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 0;
    viewport_state.pViewports = 0;//&viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &m_Scissor;

    m_ViewportState = viewport_state;
}

void omp::GraphicsPipeline::createRasterizer()
{
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    m_Rasterizer = rasterizer;
}

void omp::GraphicsPipeline::createMultisamplingInfo(VkSampleCountFlagBits rasterizationSamples)
{
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = rasterizationSamples;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    m_Multisampling = multisampling;
}

void omp::GraphicsPipeline::createColorBlending()
{
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    m_ColorBlendAttachment = color_blend_attachment;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &m_ColorBlendAttachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    m_ColorBlending = color_blending;
}

void omp::GraphicsPipeline::addPipelineSetLayout(VkDescriptorSetLayout descriptorSetLayout)
{
    m_SetLayoutsHandles.push_back(descriptorSetLayout);

    m_PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    m_PipelineLayoutInfo.setLayoutCount = m_SetLayoutsHandles.size();
    m_PipelineLayoutInfo.pSetLayouts = m_SetLayoutsHandles.data();
}

void omp::GraphicsPipeline::createShaders(const std::shared_ptr<struct Shader>& shader)
{
    m_Shader = std::move(shader);
}

void omp::GraphicsPipeline::confirmCreation(const std::shared_ptr<omp::RenderPass>& renderPass)
{
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {};
    depth_stencil.back = {};

    if (vkCreatePipelineLayout(m_LogicalDevice, &m_PipelineLayoutInfo, nullptr, &m_PipelineLayout)
        != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    VkDynamicState dynamic_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = m_Shader->getStagesCount();
    pipeline_info.pStages = m_Shader->getShaderStages().data();

    pipeline_info.pVertexInputState = &m_VertexInputInfo;
    pipeline_info.pInputAssemblyState = &m_InputAssembly;
    pipeline_info.pViewportState = &m_ViewportState;
    pipeline_info.pRasterizationState = &m_Rasterizer;
    pipeline_info.pMultisampleState = &m_Multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &m_ColorBlending;
    pipeline_info.pDynamicState = &dynamic_state;

    pipeline_info.layout = m_PipelineLayout;
    pipeline_info.renderPass = renderPass->getRenderPass();
    pipeline_info.subpass = 0;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_GraphicsPipeline)
        != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    m_IsCreated = true;
}

void omp::GraphicsPipeline::tryDestroyVulkanObjects()
{
    if (m_IsCreated)
    {
        vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);
        vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, nullptr);
    }
}
