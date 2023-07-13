#pragma once
#include "LightObject.h"
#include "Rendering/UniformBuffer.h"

namespace omp
{
    class LightSystem
    {
    public:
        static constexpr int LIGHT_COUNT = 1;
        LightSystem(const std::shared_ptr<omp::VulkanContext>& inVulkanContext, uint32_t khrNum);

    private:
        LightObject<GlobalLight> m_GlobalLight;
        std::unique_ptr<omp::UniformBuffer> m_GlobalBuffer;

        std::array<LightObject<PointLight>, LIGHT_COUNT> m_PointLights;
        std::unique_ptr<omp::UniformBuffer> m_PointBuffer;

        std::array<LightObject<SpotLight>, LIGHT_COUNT> m_SpotLights;
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

        LightObject<GlobalLight>& getGlobalLight() { return m_GlobalLight; }
        std::array<LightObject<PointLight>, LIGHT_COUNT>& getPointLight() { return m_PointLights; }
        std::array<LightObject<SpotLight>, LIGHT_COUNT>& getSpotLight() { return m_SpotLights; }

        // TODO: temp
        void setModelForEach(const std::shared_ptr<omp::ModelInstance>& inModel);

        void recreate();
        void update();
        void mapMemory(uint32_t khrImage);
    };
}
