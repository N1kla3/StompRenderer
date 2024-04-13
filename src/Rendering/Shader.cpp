#include <fstream>
#include <optional>
#include "Shader.h"
#include "Logs.h"

omp::Shader::Shader(
        const std::shared_ptr<VulkanContext>& context,
        const std::string& vertexPath,
        const std::string& fragmentPath
)
        : m_Context(context)
{
    auto vert_shader_code = readFile(vertexPath);
    auto frag_shader_code = readFile(fragmentPath);

    VkShaderModule vert_shader_module = m_Context->createShaderModule(vert_shader_code);
    VkShaderModule frag_shader_module = m_Context->createShaderModule(frag_shader_code);
    m_ShaderModules[0] = vert_shader_module;
    m_ShaderModules[1] = frag_shader_module;

    // Vertex shader
    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";

    // Fragment shader
    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    // Stages
    m_ShaderStages = {vert_shader_stage_info, frag_shader_stage_info};
}

std::vector<char> omp::Shader::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        ERROR(LogRendering, "Failed to open file: {}", filename);
        throw std::runtime_error("Failed to open file");
    }

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
}

omp::Shader::~Shader()
{
    for (auto module: m_ShaderModules)
    {
        m_Context->destroyShaderModule(module);
    }
}

void omp::Shader::serialize(JsonParser<>& parser)
{
}

void omp::Shader::deserialize(JsonParser<>& parser)
{
}

