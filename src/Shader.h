#pragma once
#include <string>
#include <memory>
#include <vector>
#include "VulkanContext.h"

namespace omp{
class Shader
{
public:
    Shader(const std::shared_ptr<VulkanContext>& context, const std::string& vertexPath, const std::string& fragmentPath);


    std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages() const { return m_ShaderStages; }

private:
    std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

    std::shared_ptr<VulkanContext> m_Context;

    static std::vector<char> readFile(const std::string& filename);
};
} // omp
