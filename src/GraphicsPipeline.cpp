#include "GraphicsPipeline.h"

omp::GraphicsPipeline::~GraphicsPipeline()
{

}

void omp::GraphicsPipeline::StartCreation()
{
    m_IsCreated = false;

}

void omp::GraphicsPipeline::StartDefaultCreation()
{

}

void omp::GraphicsPipeline::CreateVertexInfo()
{

}

void omp::GraphicsPipeline::CreateInputAssembly()
{

}

void omp::GraphicsPipeline::CreateViewport()
{

}

void omp::GraphicsPipeline::CreateRasterizer()
{

}

void omp::GraphicsPipeline::CreateMultisamplingInfo()
{

}

void omp::GraphicsPipeline::CreateColorBlending()
{

}

void omp::GraphicsPipeline::CreatePipelineLayout()
{

}

void omp::GraphicsPipeline::CreateShaders(const std::shared_ptr<struct Shader> &shader)
{

}

void omp::GraphicsPipeline::ConfirmCreation()
{

    m_IsCreated = true;
}
