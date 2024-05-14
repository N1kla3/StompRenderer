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
        , m_VertexPath(vertexPath)
        , m_FragmentPath(fragmentPath)
{
    load();
}

std::vector<char> omp::Shader::readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        ERROR(LogRendering, "Failed to open file: {}", filename);
        throw std::runtime_error("Failed to open file");
    }

    std::ifstream::pos_type file_size = file.tellg();
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

void omp::Shader::setVertexPath(const std::string& inPath)
{
    m_VertexPath = inPath;
}

void omp::Shader::setFragmentPath(const std::string& inPath)
{
    m_FragmentPath = inPath;
}

void omp::Shader::setVulkanContext(const std::shared_ptr<VulkanContext>& inContext)
{
    m_Context = inContext;
}

bool omp::Shader::load()
{
    if (!m_Context)
    {
        return false;
    }

    auto vert_shader_code = readFile(m_VertexPath);
    auto frag_shader_code = readFile(m_FragmentPath);

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

    return true;
}

void omp::Shader::serialize(JsonParser<>& parser)
{
    parser.writeValue("vertex_path", m_VertexPath);
    parser.writeValue("fragment_path", m_FragmentPath);
}

void omp::Shader::deserialize(JsonParser<>& parser)
{
    m_VertexPath = parser.readValue<std::string>("vertex_path").value();
    m_VertexPath = parser.readValue<std::string>("fragment_path").value();
}

