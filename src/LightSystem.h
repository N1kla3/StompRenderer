#pragma once
#include "LightObject.h"
#include "Rendering/UniformBuffer.h"

namespace omp
{
    class LightSystem
    {
    public:
        LightSystem(const std::shared_ptr<omp::VulkanContext>& inVulkanContext, uint32_t khrNum);

    private:
        std::unique_ptr<omp::UniformBuffer> m_GlobalBuffer;
        std::unique_ptr<omp::UniformBuffer> m_PointBuffer;
        std::unique_ptr<omp::UniformBuffer> m_SpotBuffer;

        std::shared_ptr<omp::VulkanContext> m_VulkanContext;
        omp::Scene* m_CurrentScene = nullptr;
        uint32_t m_KHRnum = 0;

        size_t m_GlobalLightNum = 0;
        size_t m_PointLightNum = 0;
        size_t m_SpotLightNum = 0;

        // METHODS //
        // ======= //
    public:
        size_t getGlobalLightSize() const { return m_GlobalLightNum; }
        size_t getPointLightSize() const { return m_PointLightNum; }
        size_t getSpotLightSize() const { return m_SpotLightNum; }

        size_t getGlobalLightBufferSize() const { return sizeof(GlobalLight); }
        size_t getPointLightBufferSize() const { return std::max(sizeof(PointLight), sizeof(PointLight) * m_PointLightNum); }
        size_t getSpotLightBufferSize() const { return std::max(sizeof(SpotLight), sizeof(SpotLight) * m_SpotLightNum); }

        VkBuffer getGlobalLightBuffer(uint32_t khr);
        VkBuffer getPointLightBuffer(uint32_t khr);
        VkBuffer getSpotLightBuffer(uint32_t khr);

        void tryRecreateBuffers();
        void update();
        void mapMemory(uint32_t khrImage);
        void onSceneChanged(omp::Scene* scene);
    };
}
