#pragma once
#include <string>
#include <memory>
#include <vector>
#include <array>
#include "VulkanContext.h"

namespace omp{
class Shader
{
public:
    Shader(const std::shared_ptr<VulkanContext>& context, const std::string& vertexPath, const std::string& fragmentPath);
    virtual ~Shader();

    std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; }

    constexpr static uint32_t GetStagesCount(){ return STAGE_COUNT; }

private:
    static constexpr uint32_t STAGE_COUNT = 2;
    std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

    std::shared_ptr<VulkanContext> m_Context;

    std::array<VkShaderModule, STAGE_COUNT> m_ShaderModules;

    static std::vector<char> readFile(const std::string& filename);
};
} // omp
