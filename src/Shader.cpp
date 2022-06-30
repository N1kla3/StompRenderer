#include <fstream>
#include "Shader.h"
#include "Logs.h"

omp::Shader::Shader(
        const std::shared_ptr<VulkanContext> &context,
        const std::string &vertexPath,
        const std::string &fragmentPath
        )
        : m_Context(context)
{
    auto vertShaderCode = readFile(vertexPath);
    auto fragShaderCode = readFile(fragmentPath);

    VkShaderModule vertShaderModule = m_Context->createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = m_Context->createShaderModule(fragShaderCode);

    // Vertex shader
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    // Fragment shader
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    // Stages
    VkPipelineShaderStageCreateInfo shaderStage[] = {vertShaderStageInfo, fragShaderStageInfo};
}

std::vector<char> omp::Shader::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}
