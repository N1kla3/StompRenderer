#pragma once

#include <string>
#include <memory>
#include <vector>
#include <array>
#include "IO/SerializableObject.h"
#include "VulkanContext.h"

namespace omp
{
    class Shader : public SerializableObject
    {
    public:
        Shader() = default;
        Shader(
            const std::shared_ptr<VulkanContext>& context,
            const std::string& vertexPath,
            const std::string& fragmentPath);
        virtual ~Shader();

        std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages() { return m_ShaderStages; }

        constexpr static uint32_t getStagesCount() { return STAGE_COUNT; }

        void setVertexPath(const std::string& inPath);
        void setFragmentPath(const std::string& inPath);
        void setVulkanContext(const std::shared_ptr<VulkanContext>& inContext);
        bool load();

        virtual void serialize(JsonParser<>& parser) override;
        virtual void deserialize(JsonParser<>& parser) override;

    private:

        static constexpr uint32_t STAGE_COUNT = 2;
        std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

        std::shared_ptr<VulkanContext> m_Context = nullptr;

        std::array<VkShaderModule, STAGE_COUNT> m_ShaderModules;

        std::string m_VertexPath;
        std::string m_FragmentPath;

        static std::vector<char> readFile(const std::string& filename);
    };
} // omp
