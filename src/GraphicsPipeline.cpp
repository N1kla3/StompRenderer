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
    TryDestroyVulkanObjects();
}

void omp::GraphicsPipeline::StartCreation()
{
    TryDestroyVulkanObjects();
    m_IsCreated = false;

}

void omp::GraphicsPipeline::StartDefaultCreation()
{
    TryDestroyVulkanObjects();
    m_IsCreated = false;

    CreateVertexInfo();
    CreateInputAssembly();
    CreateViewport(VkExtent2D());
    CreateRasterizer();
    CreateColorBlending();
}

void omp::GraphicsPipeline::CreateVertexInfo()
{
    static auto binding_description = omp::Vertex::getBindingDescription();
    static auto attribute_descriptions = omp::Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &binding_description;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attribute_descriptions.data();

    m_VertexInputInfo = vertexInputInfo;
}

void omp::GraphicsPipeline::CreateInputAssembly()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    m_InputAssembly = inputAssembly;
}

void omp::GraphicsPipeline::CreateViewport(VkExtent2D ScissorExtent)
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
    scissor.extent = ScissorExtent;//m_SwapChainExtent;
    m_Scissor = scissor;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 0;
    viewportState.pViewports = 0;//&viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &m_Scissor;

    m_ViewportState = viewportState;
}

void omp::GraphicsPipeline::CreateRasterizer()
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

void omp::GraphicsPipeline::CreateMultisamplingInfo(VkSampleCountFlagBits rasterizationSamples)
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

void omp::GraphicsPipeline::CreateColorBlending()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    m_ColorBlendAttachment = colorBlendAttachment;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &m_ColorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    m_ColorBlending = colorBlending;
}

void omp::GraphicsPipeline::CreatePipelineLayout(VkDescriptorSetLayout& descriptorSetLayout)
{

    // Push constant for model
    VkPushConstantRange constant_range{};
    constant_range.size = sizeof(omp::ModelPushConstant);
    constant_range.offset = 0;
    constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;// TODO: better with ranges to save space in shader
    m_ConstantRange = constant_range;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &m_ConstantRange;

    m_PipelineLayoutInfo = pipelineLayoutInfo;
}

void omp::GraphicsPipeline::CreateShaders(const std::shared_ptr<struct Shader> &shader)
{
    m_Shader = std::move(shader);
}

void omp::GraphicsPipeline::ConfirmCreation(VkRenderPass renderPass)
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

    VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = m_Shader->GetStagesCount();
    pipelineInfo.pStages = m_Shader->GetShaderStages().data();

    pipelineInfo.pVertexInputState = &m_VertexInputInfo;
    pipelineInfo.pInputAssemblyState = &m_InputAssembly;
    pipelineInfo.pViewportState = &m_ViewportState;
    pipelineInfo.pRasterizationState = &m_Rasterizer;
    pipelineInfo.pMultisampleState = &m_Multisampling;
    pipelineInfo.pDepthStencilState = &depth_stencil;
    pipelineInfo.pColorBlendState = &m_ColorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = m_PipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline)
        != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    m_IsCreated = true;
}

void omp::GraphicsPipeline::TryDestroyVulkanObjects()
{
    if (m_IsCreated)
    {
        vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);
        vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, nullptr);
    }
}
