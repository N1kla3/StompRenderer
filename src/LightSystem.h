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
        std::shared_ptr<LightObject<GlobalLight>> m_GlobalLight{nullptr};
        std::unique_ptr<omp::UniformBuffer> m_GlobalBuffer;

        std::vector<std::shared_ptr<LightObject<PointLight>>> m_PointLights;
        std::unique_ptr<omp::UniformBuffer> m_PointBuffer;

        std::vector<std::shared_ptr<LightObject<SpotLight>>> m_SpotLights;
        std::unique_ptr<omp::UniformBuffer> m_SpotBuffer;

        std::shared_ptr<omp::VulkanContext> m_VulkanContext;
        uint32_t m_KHRnum = 0;

        // METHODS //
        // ======= //
    public:
        size_t getPointLightSize() const { return m_PointLights.size(); }
        size_t getSpotLightSize() const { return m_SpotLights.size(); }

        size_t getGlobalLightBufferSize() const { return sizeof(GlobalLight); }
        size_t getPointLightBufferSize() const { return m_PointLights.size() * sizeof(PointLight); }
        size_t getSpotLightBufferSize() const { return m_SpotLights.size() * sizeof(SpotLight); }

        VkBuffer getGlobalLightBuffer(uint32_t khr);
        VkBuffer getPointLightBuffer(uint32_t khr);
        VkBuffer getSpotLightBuffer(uint32_t khr);

        std::shared_ptr<LightObject<GlobalLight>>& getGlobalLight() { return m_GlobalLight; }
        std::vector<std::shared_ptr<LightObject<PointLight>>>& getPointLight() { return m_PointLights; }
        std::vector<std::shared_ptr<LightObject<SpotLight>>>& getSpotLight() { return m_SpotLights; }

        std::shared_ptr<omp::LightObject<omp::GlobalLight>> enableGlobalLight(const std::shared_ptr<omp::ModelInstance>& inModel);
        void addPointLight(const std::shared_ptr<LightObject<PointLight>>& inLight);
        void addSpotLight(const std::shared_ptr<LightObject<SpotLight>>& inLight);

        void recreate();
        void update();
        void mapMemory(uint32_t khrImage);
    };
}
